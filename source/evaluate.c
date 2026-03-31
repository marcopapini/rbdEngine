/*
 * Component: evaluate.c
 * Evaluate the Reliability curve of a RBD
 *
 * rbdEngine - Evaluate the reliability of a given Reliability Block Diagram
 * through the RBD Description Language file. Navigate the RBD DAG with the
 * Depth First Search (DFS) algorithm and solve the statistical dependencies
 * among different blocks by applying the Conditional Decomposition Method (CDM).
 * Copyright (C) 2025 by Marco Papini <papini.m@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "evaluate.h"

#include "dagdata.h"
#include "rbd.h"
#include "rbddata.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define MAX_PIVOTS_PER_SHANNON_DOMAIN       30


static int computeTopologicalOrder(struct rbd *rbd, struct dag *dag, struct node *node, unsigned char isCdm);
static void collectPivotsRecursive(struct node *node, struct dag *dag);
static void buildShannonDomain(struct dag *dag, struct node *convergence);
static int solveShannonMultiPivot(struct rbd *rbd, struct dag *dag, struct node *convergence);
static int computeBlockReliability(struct rbd *rbd, struct block *block);
static void forceNode(struct node *node, double value);
static void unforceNode(struct node *node);
static void resetAnalyzedFlag(struct node *node);
static void resetAnalyzedFlagRecursive(struct node *node);
static void updateNodesStatus(struct node *node);
static void updateNodesStatusRecursive(struct node *node);
static struct node *findConvergence(struct dag *dag);
static struct node *findConvergenceOfPivot(struct dag *dag, struct node *pivot);
static int isAncestorOf(struct dag *dag, struct node *n1, struct node *n2);
static int isAncestorOfRecursive(struct dag *dag, struct node *n1, struct node *n2);
static int cnttz(unsigned int val);


/**
 * evaluateRbd
 *
 * Evaluate the provided RBD exploiting the supporting RBD DAG
 *
 * Description:
 *  This function evaluates the Reliability curve of the provided
 *  RBD by exploiting the supporting RBD DAG by reconstructing the
 *  topological order needed to evaluate RBD blocks while respecting
 *  the block dependencies.
 *
 * Parameters:
 *      rbd: RBD Data Structure to be evaluated
 *      dag: RBD DAG used to evaluate the RBD
 *
 * Return (int):
 *  0 if the RBD Reliability curve has been evaluated successfully, < 0 otherwise
 */
int evaluateRbd(struct rbd *rbd, struct dag *dag) {
    struct node *convergence;
    unsigned int idx;

    /* Cycle until the entire system has been evaluated, i.e., the root status is equal to COMPUTED */
    while(1) {
        /* Reset to 0 the number of pivot nodes */
        dag->numPivots = 0;

        /**
         * Compute, starting from the root, the topological order (no CDM evaluation).
         * For each node that can be directly computed (no statistical dependence influences
         * the computation), this function also computes the reliability curve associated
         * with the node. Furthermore, this function creates the set of pivot nodes to be
         * used for the subsequent (if any) CDM evaluation.
         */
        if (computeTopologicalOrder(rbd, dag, dag->root, 0) < 0) {
            return -1;
        }

        /* If the root is COMPUTED, then the RBD has been fully evaluated */
        if (dag->root->status == DAG_COMPUTED) {
            break;
        }

        /* Find the current convergence node of the RBD DAG */
        convergence = findConvergence(dag);
        /* Cycle until all convergence nodes have been processed */
        while (convergence != NULL) {
            /**
             * Select the pivot nodes that are inside the current Shannon Domain, i.e.,
             * the pivots that are descendants of the current convergence node
             */
            buildShannonDomain(dag, convergence);
            if (dag->numPivotIdx > MAX_PIVOTS_PER_SHANNON_DOMAIN) {
                fprintf(stderr, "Shannon Domain too complex (%u pivot nodes), max allowed is %u.\n",
                        dag->numPivotIdx, MAX_PIVOTS_PER_SHANNON_DOMAIN);
                return -1;
            }

            /**
             * Evaluate the reliability of the convergence node with CDM
             * evaluation exploiting Shannon decomposition
             */
            if (solveShannonMultiPivot(rbd, dag, convergence) < 0) {
                return -1;
            }

            /* Update the status of all nodes descendants of the convergence node */
            updateNodesStatus(convergence);

            /* Find the next convergence node of the RBD DAG */
            convergence = findConvergence(dag);
        }

        /* Check that the status of pivot nodes is COMPUTED, otherwise raise an error */
        for (idx = 0; idx < dag->numPivots; ++idx) {
            if (dag->pivots[idx]->status != DAG_COMPUTED) {
                return -1;
            }
        }
    }

    return 0;
}


/**
 * computeTopologicalOrder
 *
 * Recursively compute the topological order used to process RBD DAG
 *
 * Description:
 *  This recursive function computes the topological order that is used
 *  to correctly process all RBD blocks, respecting their dependencies.
 *  The topological order is obtained with a variation of Post-order DFS.
 *  In particular, each node is initially tested for it being already
 *  computed, in which case no further action is needed. Otherwise, before
 *  computing the Reliability curve associated with the corresponding block,
 *  all children of the current node are recursively analyzed. If at least
 *  one child is a pivot node for CDM or it's evaluation has been deferred
 *  to the application of CDM, then the current node cannot be analyzed but
 *  it is deferred.
 *
 * Parameters:
 *      rbd: RBD Data Structure used to compute the topological order
 *      node: RBD DAG node for which the topological order is computed
 *      isCdm: 0 if this call is not used to apply the CDM, 1 otherwise
 *
 * Return (int):
 *  0 if the topological order has been computed successfully, < 0 otherwise
 */
static int computeTopologicalOrder(struct rbd *rbd, struct dag *dag, struct node *node, unsigned char isCdm) {
    unsigned char idx;

    /* If the node status is COMPUTED or COMPUTED_PIVOT, stop recursion without further action */
    if ((node->status == DAG_COMPUTED) || (node->status == DAG_COMPUTED_PIVOT)) {
        return 0;
    }

    /* Is the function invoked outside of the application of CDM ? */
    if (isCdm == 0) {
        /* Is the node associated to a component? */
        if (node->input == INPUT_COMPONENT) {
            /* Is the node a pivot node? */
            if (node->refCount > 1) {
                /* Set the node status to COMPUTED_PIVOT and add the node to the set of pivots */
                node->status = DAG_COMPUTED_PIVOT;
                dag->pivots[dag->numPivots++] = node;
            }
            else {
                /* Set the node status to COMPUTED */
                node->status = DAG_COMPUTED;
            }
            return 0;
        }

        /* Early assumption: the current node status is COMPUTED */
        node->status = DAG_COMPUTED;
        /* For each child... */
        for (idx = 0; idx < node->numChildren; ++idx) {
            /* Recursively compute the topological order */
            if (computeTopologicalOrder(rbd, dag, node->children[idx], 0) < 0) {
                return -1;
            }
            /* Set the status to DEFERRED_CDM if the child status is not COMPUTED */
            if (node->children[idx]->status != DAG_COMPUTED) {
                node->status = DAG_DEFERRED_CDM;
            }
        }

        /* Is the current block a pivot node and its status is still COMPUTED? */
        if ((node->refCount > 1) && (node->status == DAG_COMPUTED)) {
            /* Ste the node status to COMPUTED_PIVOT and add the node to the set of pivots */
            node->status = DAG_COMPUTED_PIVOT;
            dag->pivots[dag->numPivots++] = node;
        }

        /* If the node status is COMPUTED or COMPUTED_PIVOT, compute it */
        if (node->status == DAG_COMPUTED || node->status == DAG_COMPUTED_PIVOT) {
            computeBlockReliability(rbd, node->block);
        }
    }
    else {
        /* If the current node has been already analyzed, then stop the recursion */
        if (node->block->bIsAnalyzed != 0) {
            return 0;
        }

        /* For each child... */
        for (idx = 0; idx < node->numChildren; ++idx) {
            /* Recursively compute the topological order */
            if (computeTopologicalOrder(rbd, dag, node->children[idx], 1) < 0) {
                return -1;
            }
        }

        /* Compute the reliability of the current node */
        if (computeBlockReliability(rbd, node->block) < 0) {
            return -1;
        }
    }

    return 0;
}

/**
 * collectPivotsRecursive
 *
 * Recursively add new pivot nodes
 *
 * Description:
 *  This recursive function checks if the current node is a new pivot node
 *  and, if true, it adds it to the list of known pivot nodes.
 *
 * Parameters:
 *      node: current node inside the RBD DAG
 *      dag: RBD DAG used to recursively add new pivots
 */
static void collectPivotsRecursive(struct node *node, struct dag *dag) {
    unsigned int idx;
    int exists;

    /* Is the current node a pivot node? */
    if (node->refCount > 1) {
        /* Check if the current node is already included into the list of pivot nodes */
        exists = 0;
        for (idx = 0; idx < dag->numPivots; ++idx) {
            if (dag->pivots[idx] == node) {
                exists = 1;
                break;
            }
        }
        /* The pivot node is not contained in the list of pivot nodes, add it */
        if (exists == 0) {
            dag->pivots[dag->numPivots++] = node;
        }
    }

    /* Is the current node referring to a block with status equal to DEFERRED_CDM? */
    if ((node->input == INPUT_BLOCK) && (node->status == DAG_DEFERRED_CDM)) {
        /* Recursively check and add all its children */
        for (idx = 0; idx < node->numChildren; idx++) {
            collectPivotsRecursive(node->children[idx], dag);
        }
    }
}

/**
 * buildShannonDomain
 *
 * Define the domain for the current Shannon decomposition
 *
 * Description:
 *  This function defines the domain, i.e., the set of pivot nodes, over
 *  which the current Shannon decomposition operates. A pivot node belongs
 *  to the Shannon domain if: 1) it has not been fully evaluated; and 2) it
 *  is a descendant of the current convergence node.
 *
 * Parameters:
 *      dag: RBD DAG used to build the Shannon Domain
 *      convergence: current convergence node inside the RBD DAG
 */
static void buildShannonDomain(struct dag *dag, struct node *convergence) {
    unsigned int idx;

    /* Reset to 0 the number of pivot indexes */
    dag->numPivotIdx = 0;

    /* For each pivot node... */
    for (idx = 0; idx < dag->numPivots; ++idx) {
        /* Is the status of the current pivot node equal to COMPUTED_PIVOT? */
        if (dag->pivots[idx]->status == DAG_COMPUTED_PIVOT) {
            /* Is the convergence node an ancestor of the current pivot node? */
            if (isAncestorOf(dag, convergence, dag->pivots[idx])) {
                /* Insert the index of the current pivot node into the list of pivot indexes */
                dag->pivotsIdx[dag->numPivotIdx++] = idx;
            }
        }
    }
}

/**
 * solveShannonMultiPivot
 *
 * Perform Shannon Decomposition over a set of pivots
 *
 * Description:
 *  This function evaluates the reliability of a convergence node by performing
 *  the Shannon Decomposition over a set of pivot nodes.
 *
 * Parameters:
 *      rbd: RBD Data Structure used to solve the Shannon Domain
 *      dag: RBD DAG used to solve the Shannon Domain
 *      convergence: the convergence node to be analyzed
 *
 * Return (int):
 *  0 if the Shannon Domain has been evaluated successfully, < 0 otherwise
 */
static int solveShannonMultiPivot(struct rbd *rbd, struct dag *dag, struct node *convergence) {
    unsigned int numScenarios;
    unsigned int numTimes;
    unsigned int sIdx;
    unsigned int scenarioId;
    int changedBit;
    unsigned int pIdx;
    unsigned int tIdx;
    double *inRel;
    double *unreliability;
    double *matrixSeries;
    double *tmpRel;
    double *outRel;
    struct node *pivot;

    /* Retrieve the number of time instants */
    numTimes = rbd->time.numTimes;

    /* Compute the number of scenarios to be evaluated with the current Shannon decomposition */
    numScenarios = 1 << dag->numPivotIdx;

    /* Allocate memory for Shannon Decomposition */
    unreliability = (double *)malloc(numTimes * dag->numPivotIdx * sizeof(double));
    matrixSeries = (double *)malloc(numTimes * (dag->numPivotIdx + 1) * sizeof(double));
    tmpRel = (double *)malloc(numTimes * sizeof(double));
    outRel = (double *)calloc(numTimes, sizeof(double));
    if ((unreliability == NULL) || (matrixSeries == NULL) || (tmpRel == NULL) || (outRel == NULL)) {
        fprintf(stderr, "Unable to allocate memory for Shannon Decomposition\n");
        if (unreliability == NULL) {
            free(unreliability);
        }
        if (matrixSeries == NULL) {
            free(matrixSeries);
        }
        if (tmpRel == NULL) {
            free(tmpRel);
        }
        if (outRel == NULL) {
            free(outRel);
        }
        return -1;
    }

    /* For each pivot node... */
    for (pIdx = 0; pIdx < dag->numPivotIdx; ++pIdx) {
        /* Pre-compute its unreliability */
        pivot = dag->pivots[dag->pivotsIdx[pIdx]];
        inRel = (pivot->input == INPUT_BLOCK) ? pivot->block->reliability : pivot->component->reliability;
        for (tIdx = 0; tIdx < numTimes; ++tIdx) {
            unreliability[(pIdx * numTimes) + tIdx] = 1.0 - inRel[tIdx];
        }
        /* Copy its unreliability into the proper row of Series matrix */
        memcpy(&matrixSeries[pIdx * numTimes], &unreliability[pIdx * numTimes], (numTimes * sizeof(double)));
        /* Force the pivot node to failed */
        forceNode(pivot, 0.0);
    }

    /* Initialize the current batch size to 0 */
    for (sIdx = 0; sIdx < numScenarios; ++sIdx) {
        /* Detect the changed bit using the Count Trailing Zero */
        changedBit = cnttz(sIdx);

        /* Is the changed bit a valid one? */
        if (changedBit >= 0) {
            /* Compute the scenario ID using Gray method */
            scenarioId = sIdx ^ (sIdx >> 1);
            /* Retrieve the pivot node associated with the changed bit */
            pivot = dag->pivots[dag->pivotsIdx[changedBit]];
            if ((scenarioId >> changedBit) & 1) {
                /* The new status of the pivot node is working */
                /* Copy the reliability of the pivot node into the Series matrix */
                inRel = (pivot->input == INPUT_BLOCK) ? pivot->block->reliability : pivot->component->reliability;
                memcpy(&matrixSeries[changedBit * numTimes], inRel, (numTimes * sizeof(double)));
                /* Force the pivot node to working */
                forceNode(pivot, 1.0);
            } else {
                /* The new status of the pivot node is failed */
                /* Copy the unreliability of the pivot node into the Series matrix */
                memcpy(&matrixSeries[changedBit * numTimes], &unreliability[changedBit * numTimes],
                        (numTimes * sizeof(double)));
                /* Force the pivot node to failed */
                forceNode(pivot, 0.0);
            }
        }

        /* Reset all dirty nodes starting from the convergence node */
        resetAnalyzedFlag(convergence);
        /* Compute the reliability of the convergence node for the current scenario */
        computeTopologicalOrder(rbd, dag, convergence, 1);

        /* Copy the reliability of the convergence node for the current scenario into the Series matrix */
        memcpy(&matrixSeries[dag->numPivotIdx * numTimes], convergence->block->reliability,
                (numTimes * sizeof(double)));

        /* Compute the reliability of the current scenario using a Generic Series block */
        if (rbdSeriesGeneric(matrixSeries, tmpRel, (dag->numPivotIdx + 1), numTimes) < 0) {
            fprintf(stderr, "Unable to compute reliability for Series Shannon Decomposition\n");
            free(unreliability);
            free(matrixSeries);
            free(tmpRel);
            free(outRel);
            return -1;
        }

        /* Update the total reliability with the contribution of the current scenario */
        for (tIdx = 0; tIdx < numTimes; ++tIdx) {
            outRel[tIdx] += tmpRel[tIdx];
        }
    }

    /* For each pivot node... */
    for (pIdx = 0; pIdx < dag->numPivotIdx; pIdx++) {
        /* Unforce the pivot node */
        unforceNode(dag->pivots[dag->pivotsIdx[pIdx]]);
    }

    /* Copy the reliability curve computed through Shannon Decomposition to the reliability of the convergence node */
    memcpy(convergence->block->reliability, outRel, numTimes * sizeof(double));

    /* Free the memory used by matrixes/vectors */
    free(unreliability);
    free(matrixSeries);
    free(tmpRel);
    free(outRel);

    return 0;
}

/**
 * computeBlockReliability
 *
 * Compute the Reliability curve of a RBD block
 *
 * Description:
 *  This function computes the Reliability curve of the provided RBD block
 *
 * Parameters:
 *      rbd: RBD Data Structure used to compute the reliability curve
 *      block: RBD block for which the reliability curve is computed
 *
 * Return (int):
 *  0 if the Reliability curve has been computed successfully, < 0 otherwise
 */
static int computeBlockReliability(struct rbd *rbd, struct block *block) {
    int librbdRes;
    double *rel;
    unsigned int tIdx;
    unsigned char cIdx;

    if (block->reliability == NULL) {
        /* Allocate memory for output reliability array */
        block->reliability = (double *)malloc(sizeof(double) * rbd->time.numTimes);
        if (block->reliability == NULL) {
            fprintf(stderr, "Unable to allocate memory for Reliability curve of block %s\n", block->outputName);
            return -1;
        }
    }

    /* Is the block a generic RBD block? */
    if (block->bIsIdentical == 0) {
        /* Allocate memory for the input reliability matrix */
        rel = (double *)malloc(sizeof(double) * rbd->time.numTimes * block->numInputs);
        if (rel == NULL) {
            fprintf(stderr, "Unable to allocate memory for Reliability curve (matrix) of block %s\n", block->outputName);
            return -1;
        }

        /* Set values in reliability matrix */
        for (cIdx = 0; cIdx < block->numInputs; ++cIdx) {
            if (block->inputs[cIdx].bIsUnreliability == 0) {
                if (block->inputs[cIdx].type == INPUT_COMPONENT) {
                    if (rbd->components[block->inputs[cIdx].idx].bIsForced == 0) {
                        memcpy(&rel[cIdx * rbd->time.numTimes],
                               rbd->components[block->inputs[cIdx].idx].reliability,
                               sizeof(double) * rbd->time.numTimes);
                    }
                    else {
                        for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                            rel[(cIdx * rbd->time.numTimes) + tIdx] =
                                    rbd->components[block->inputs[cIdx].idx].forcedValue;
                        }
                    }
                }
                else {
                    if (rbd->blocks[block->inputs[cIdx].idx].bIsForced == 0) {
                        memcpy(&rel[cIdx * rbd->time.numTimes],
                               rbd->blocks[block->inputs[cIdx].idx].reliability,
                               sizeof(double) * rbd->time.numTimes);
                    }
                    else {
                        for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                            rel[(cIdx * rbd->time.numTimes) + tIdx] =
                                    rbd->blocks[block->inputs[cIdx].idx].forcedValue;
                        }
                    }
                }
            }
            else {
                if (block->inputs[cIdx].type == INPUT_COMPONENT) {
                    if (rbd->components[block->inputs[cIdx].idx].bIsForced == 0) {
                        for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                            rel[(cIdx * rbd->time.numTimes) + tIdx] =
                                    1.0 - rbd->components[block->inputs[cIdx].idx].reliability[tIdx];
                        }
                    }
                    else {
                        for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                            rel[(cIdx * rbd->time.numTimes) + tIdx] =
                                    1.0 - rbd->components[block->inputs[cIdx].idx].forcedValue;
                        }
                    }
                }
                else {
                    if (rbd->blocks[block->inputs[cIdx].idx].bIsForced == 0) {
                        for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                            rel[(cIdx * rbd->time.numTimes) + tIdx] =
                                    1.0 - rbd->blocks[block->inputs[cIdx].idx].reliability[tIdx];
                        }
                    }
                    else {
                        for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                            rel[(cIdx * rbd->time.numTimes) + tIdx] =
                                    1.0 - rbd->blocks[block->inputs[cIdx].idx].forcedValue;
                        }
                    }
                }
            }
        }

        /* Evaluate RBD Basic Block with generic components */
        switch (block->type) {
            case BLK_BRIDGE:
                librbdRes = rbdBridgeGeneric(rel,
                                             block->reliability,
                                             block->numInputs,
                                             rbd->time.numTimes);
                break;

            case BLK_KOON:
                librbdRes = rbdKooNGeneric(rel,
                                           block->reliability,
                                           block->numInputs,
                                           block->minInputs,
                                           rbd->time.numTimes);
                break;

            case BLK_PARALLEL:
                librbdRes = rbdParallelGeneric(rel,
                                               block->reliability,
                                               block->numInputs,
                                               rbd->time.numTimes);
                break;

            case BLK_SERIES:
            default:
                librbdRes = rbdSeriesGeneric(rel,
                                             block->reliability,
                                             block->numInputs,
                                             rbd->time.numTimes);
                break;
        }
    }
    else {
        /* Allocate memory for reliability array */
        rel = (double *)malloc(sizeof(double) * rbd->time.numTimes);
        if (rel == NULL) {
            fprintf(stderr, "Unable to allocate memory for Reliability curve (array) of block %s\n", block->outputName);
            return -1;
        }

        /* Set values in reliability array */
        if (block->inputs[0].bIsUnreliability == 0) {
            if (block->inputs[0].type == INPUT_COMPONENT) {
                if (rbd->components[block->inputs[0].idx].bIsForced == 0) {
                    memcpy(rel,
                           rbd->components[block->inputs[0].idx].reliability,
                           sizeof(double) * rbd->time.numTimes);
                }
                else {
                    for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                        rel[tIdx] = rbd->components[block->inputs[0].idx].forcedValue;
                    }
                }
            }
            else {
                if (rbd->blocks[block->inputs[0].idx].bIsForced == 0) {
                    memcpy(rel,
                           rbd->blocks[block->inputs[0].idx].reliability,
                           sizeof(double) * rbd->time.numTimes);
                }
                else {
                    for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                        rel[tIdx] = rbd->blocks[block->inputs[0].idx].forcedValue;
                    }
                }
            }
        }
        else {
            if (block->inputs[0].type == INPUT_COMPONENT) {
                if (rbd->components[block->inputs[0].idx].bIsForced == 0) {
                    for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                        rel[tIdx] = 1.0 - rbd->components[block->inputs[0].idx].reliability[tIdx];
                    }
                }
                else {
                    for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                        rel[tIdx] = 1.0 - rbd->components[block->inputs[0].idx].forcedValue;
                    }
                }
            }
            else {
                if (rbd->blocks[block->inputs[0].idx].bIsForced == 0) {
                    for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                        rel[tIdx] = 1.0 - rbd->blocks[block->inputs[0].idx].reliability[tIdx];
                    }
                }
                else {
                    for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                        rel[tIdx] = 1.0 - rbd->blocks[block->inputs[0].idx].forcedValue;
                    }
                }
            }
        }

        /* Evaluate RBD Basic Block with identical components */
        switch (block->type) {
            case BLK_BRIDGE:
                librbdRes = rbdBridgeIdentical(rel,
                                               block->reliability,
                                               block->numInputs,
                                               rbd->time.numTimes);
                break;

            case BLK_KOON:
                librbdRes = rbdKooNIdentical(rel,
                                             block->reliability,
                                             block->numInputs,
                                             block->minInputs,
                                             rbd->time.numTimes);
                break;

            case BLK_PARALLEL:
                librbdRes = rbdParallelIdentical(rel,
                                                 block->reliability,
                                                 block->numInputs,
                                                 rbd->time.numTimes);
                break;

            case BLK_SERIES:
            default:
                librbdRes = rbdSeriesIdentical(rel,
                                               block->reliability,
                                               block->numInputs,
                                               rbd->time.numTimes);
                break;
        }
    }

    /* Free memory of input reliability */
    free(rel);

    /* Check for librbd errors */
    if (librbdRes < 0) {
        fprintf(stderr, "Unable to compute reliability of block %s\n", block->outputName);
        return -1;
    }

    /* Current block has been successfully analyzed */
    block->bIsAnalyzed = 1;

    return 0;
}

/**
 * forceNode
 *
 * Force the status of a node
 *
 * Description:
 *  This function forces the status of a node to a given value.
 *
 * Parameters:
 *      node: RBD DAG node to be forced
 *      value: value to be forced over the provided DAG node
 */
static void forceNode(struct node *node, double value)
{
    /* Is the node referencing a block? */
    if (node->input == INPUT_BLOCK) {
        /* Set the forced flag and the forced value of the referenced block */
        node->block->bIsForced = 1;
        node->block->forcedValue = value;
    }
    else {
        /* Set the forced flag and the forced value of the referenced component */
        node->component->bIsForced = 1;
        node->component->forcedValue = value;
    }
}

/**
 * unforceNode
 *
 * Unforce the status of a node
 *
 * Description:
 *  This function unforces the status of a node.
 *
 * Parameters:
 *      node: RBD DAG node to be unforced
 */
static void unforceNode(struct node *node)
{
    /* Is the node referencing a block? */
    if (node->input == INPUT_BLOCK) {
        /* Reset the forced flag of the referenced block */
        node->block->bIsForced = 0;
    }
    else {
        /* Reset the forced flag of the referenced component */
        node->component->bIsForced = 0;
    }
}

/**
 * resetAnalyzedFlag
 *
 * Reset the analyzed flag of the given sub-DAG
 *
 * Description:
 *  This function resets the analyzed flag of the requested sub-DAG.
 *
 * Parameters:
 *      node: RBD DAG node to be reset
 */
static void resetAnalyzedFlag(struct node *node) {
    /* Is the node referring a block, is its status different from COMPUTED and is it analyzed? */
    if ((node->input == INPUT_BLOCK) && (node->status != DAG_COMPUTED) && (node->block->bIsAnalyzed != 0)) {
        /* Recursively reset the sub-DAG */
        resetAnalyzedFlagRecursive(node);
    }
}

/**
 * resetAnalyzedFlagRecursive
 *
 * Recursively reset the analyzed flag
 *
 * Description:
 *  This recursive function resets the analyzed flag of a given node and
 *  of its children.
 *
 * Parameters:
 *      node: RBD DAG node to be reset
 */
static void resetAnalyzedFlagRecursive(struct node *node) {
    unsigned char idx;

    /* Reset the analyzed flag*/
    node->block->bIsAnalyzed = 0;
    /* For each child... */
    for (idx = 0; idx < node->numChildren; ++idx) {
        /* Is the child referring a block, is its status different from COMPUTED and is it analyzed? */
        if ((node->children[idx]->input == INPUT_BLOCK) &&
            (node->children[idx]->status != DAG_COMPUTED) &&
            (node->children[idx]->block->bIsAnalyzed != 0)) {
            /* Recursively reset the child */
            resetAnalyzedFlagRecursive(node->children[idx]);
        }
    }
}

/**
 * updateNodesStatus
 *
 * Update the status of a sub-DAG
 *
 * Description:
 *  This function updates to fully computed the status of the given sub-DAG.
 *
 * Parameters:
 *      node: RBD DAG node to be updated
 */
static void updateNodesStatus(struct node *node) {
    /* Is the current node referencing a block? */
    if (node->input == INPUT_BLOCK) {
        /* Is the node status equal to DEFERRED_CDM? */
        if (node->status == DAG_DEFERRED_CDM) {
            /* Recursively update the status of the sub-DAG */
            updateNodesStatusRecursive(node);
        }
        /* Is the node status equal to COMPUTED_PIVOT? */
        else if (node->status == DAG_COMPUTED_PIVOT) {
            /* Set the node status to COMPUTED */
            node->status = DAG_COMPUTED;
        }
    }
}

/**
 * updateNodesStatusRecursive
 *
 * Recursively update the status of a node
 *
 * Description:
 *  This recursive function updates to fully computed the status of a
 *  given node and of its children.
 *
 * Parameters:
 *      node: RBD DAG node to be updated
 */
static void updateNodesStatusRecursive(struct node *node) {
    unsigned char idx;

    /* Set the current node status to fully computed */
    node->status = DAG_COMPUTED;
    /* Recursively update the node status for each child */
    for (idx = 0; idx < node->numChildren; ++idx) {
        /* Is the current child referencing a block? */
        if (node->input == INPUT_BLOCK) {
            /* Is the child status equal to DEFERRED_CDM? */
            if (node->children[idx]->status == DAG_DEFERRED_CDM) {
                /* Recursively update the status of the child */
                updateNodesStatusRecursive(node->children[idx]);
                /**
                 * It is impossible to compute a statistically independent
                 * reliability curve for a descendant of a convergence node
                 * when its status is equal to DEFERRED_CDM.
                 * Clear, if needed, the output filename field.
                 */
                cleanUpXmlField(&node->children[idx]->block->outputFilename);
            }
            /* Is the child status equal to COMPUTED_PIVOT? */
            else if (node->children[idx]->status == DAG_COMPUTED_PIVOT) {
                /* Set the child status to COMPUTED */
                node->children[idx]->status = DAG_COMPUTED;
            }
        }
    }
}

/**
 * findConvergence
 *
 * Retrieve the convergence node given the RBD DAG
 *
 * Description:
 *  This function searches for the convergence node inside the RBD DAG
 *  starting from the first pivot node to be analyzed.
 *
 * Parameters:
 *      dag: RBD DAG used to find the convergence node
 *
 * Return (struct node *):
 *  The convergence node
 */
static struct node *findConvergence(struct dag *dag) {
    struct node *currConv;
    struct node *tmpConv;
    unsigned int idx;
    unsigned char changed;

    /* Search for the first pivot node to be analyzed */
    currConv = NULL;
    for (idx = 0; idx < dag->numPivots; ++idx) {
        if (dag->pivots[idx]->status == DAG_COMPUTED_PIVOT) {
            /* Retrieve the first candidate for the convergence node */
            currConv = findConvergenceOfPivot(dag, dag->pivots[idx]);
            break;
        }
    }
    /* No pivot/convergence node has been found */
    if (currConv == NULL) {
        return NULL;
    }

    /* Cycle to expand the domain of the convergence node */
    do {
        /* Assumption: convergence node has not changed */
        changed = 0;

        /* Add to the list of pivots the internal ones reachable from the current convergence node */
        collectPivotsRecursive(currConv, dag);
        /* For each pivot node... */
        for (idx = 0; idx < dag->numPivots; ++idx) {
            /* Skip pivot nodes with status different from COMPUTED_PIVOT */
            if (dag->pivots[idx]->status == DAG_COMPUTED_PIVOT) {
                /* Is the current convergence node the ancestor of the current pivot? */
                if (isAncestorOf(dag, currConv, dag->pivots[idx])) {
                    /* Retrieve the temporary convergence node starting from the current pivot */
                    tmpConv = findConvergenceOfPivot(dag, dag->pivots[idx]);

                    /* Is the temporary convergence node different from the current one? */
                    if (tmpConv != currConv) {
                        /* Is the temporary convergence node an ancestor of the current one? */
                        if (isAncestorOf(dag, tmpConv, currConv) != 0) {
                            /* Update the convergence node and restart the analysis of all pivots */
                            currConv = tmpConv;
                            changed = 1;
                            break;
                        }
                    }
                }
            }
        }
    } while (changed);

    return currConv;
}

/**
 * findConvergenceOfPivot
 *
 * Retrieve the convergence node given the RBD DAG and a pivot node
 *
 * Description:
 *  This function searches for the convergence node inside the RBD DAG
 *  given the pivot node.
 *
 * Parameters:
 *      dag: RBD DAG used to find the convergence node
 *      pivot: pivot node for which the convergence node has to be found
 *
 * Return (struct node *):
 *  The convergence node
 */
static struct node *findConvergenceOfPivot(struct dag *dag, struct node *pivot) {
    struct node *curr;
    struct node *next;
    unsigned char idx;
    unsigned int paths;

    curr = dag->root;
    /* Cycle until the current node is different from the pivot */
    while (curr != pivot) {
        /* Compute the number of paths leading to the pivot and set the next node to follow */
        next = NULL;
        paths = 0;
        for (idx = 0; idx < curr->numChildren; ++idx) {
            if (isAncestorOf(dag, curr->children[idx], pivot)) {
                ++paths;
                next = curr->children[idx];
            }
        }
        /* Is the number of paths to pivot exactly equal to 1? */
        if (paths == 1) {
            /* Continue towards the pivot */
            curr = next;
        }
        else {
            /* Convergence found, return it */
            break;
        }
    }

    return curr;
}

/**
 * isAncestorOf
 *
 * Check if the first node is an ancestor of the second node
 *
 * Description:
 *  This function checks if the first node is an ancestor of the
 *  second node, i.e., if a path connecting n1 to n2 exists.
 *
 * Parameters:
 *      dag: RBD DAG
 *      n1: first node
 *      n2: second node
 *
 * Return (int):
 *  1 if n1 is an ancestor of n2, 0 otherwise
 */
static int isAncestorOf(struct dag *dag, struct node *n1, struct node *n2) {
    unsigned char info;
    unsigned char isAncestor;

    /* Retrieve the ancestor information and, if valid, return it */
    info = getAncestor(dag, n1->nodeId, n2->nodeId);
    if (info != ANCESTOR_UNKNOWN) {
        return (info & IS_ANCESTOR_MASK);
    }

    /* Recursively check if n1 is an ancestor of n2 */
    isAncestor = isAncestorOfRecursive(dag, n1, n2);

    return isAncestor;
}

/**
 * isAncestorOfRecursive
 *
 * Recursive ancestor of retrieval
 *
 * Description:
 *  This recursive function checks if the first node is an ancestor of the
 *  second node, i.e., if a path connecting n1 to n2 exists.
 *
 * Parameters:
 *      dag: RBD DAG
 *      n1: first node
 *      n2: second node
 *
 * Return (int):
 *  1 if n1 is an ancestor of n2, 0 otherwise
 */
static int isAncestorOfRecursive(struct dag *dag, struct node *n1, struct node *n2) {
    unsigned char idx;
    unsigned char info;
    unsigned char isAncestor;

    /* Assumption: n1 is not an ancestor of n2 */
    isAncestor = 0;
    /* For each child of n1... */
    for (idx = 0; idx < n1->numChildren; ++idx) {
        /* Retrieve the ancestor information and, if valid, return it */
        info = getAncestor(dag, n1->children[idx]->nodeId, n2->nodeId);
        if (info == ANCESTOR_YES) {
            /* n1 is an ancestor of n2, stop recursion */
            isAncestor = 1;
            break;
        }
        /* Check if the child is an ancestor of n2 */
        else if (info == ANCESTOR_UNKNOWN) {
            if (isAncestorOfRecursive(dag, n1->children[idx], n2)) {
                /* n1 is an ancestor of n2, stop recursion */
                isAncestor = 1;
                break;
            }
        }
    }

    /* Is n1 an ancestor of n2? */
    if (isAncestor != 0) {
        /* Set that n1 is an ancestor of n2 and that n2 is not an ancestor of n1 (antisymmetry) */
        setAncestor(dag, n1->nodeId, n2->nodeId, ANCESTOR_YES);
        setAncestor(dag, n2->nodeId, n1->nodeId, ANCESTOR_NO);
    } else {
        /* Set that n1 is not an ancestor of n2 */
        setAncestor(dag, n1->nodeId, n2->nodeId, ANCESTOR_NO);
    }

    return isAncestor;
}

/**
 * cnttz
 *
 * Count Trailing Zeroes (CTZ)
 *
 * Description:
 *  This function counts the trailing zeroes of the given value.
 *  If the given value is 0, then the CTZ operation is undefined
 *  and this function returns -1.
 *
 * Parameters:
 *      val: value used for CTZ operation
 *
 * Return (int):
 *  The number of trailing zeroes, -1 if the given value is 0
 */
static int cnttz(unsigned int val) {
    int bit;

    /* If the value is 0, then the count trailing zeroes is undefined, return -1 */
    if (val == 0) {
        return -1;
    }

    /* Test each bit of value, from lsb, until the first one set to 1 is found */
    bit = 31;
    /* Isolate the lowest bit (closest to lsb) set to 1 */
    val &= -(signed int)val;
    /* Count trailing zeroes through binary search */
    bit -= (!!(val & 0x0000FFFF)) << 4;
    bit -= (!!(val & 0x00FF00FF)) << 3;
    bit -= (!!(val & 0x0F0F0F0F)) << 2;
    bit -= (!!(val & 0x33333333)) << 1;
    bit -= !!(val & 0x55555555);

    return bit;
}

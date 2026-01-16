/*
 *  Component: evaluate.c
 *  Evaluate the Reliability curve of a RBD
 *
 *  rbdEngine - Evaluate the reliability of a given Reliability Block Diagram
 *  through the RBD Description Language file.
 *  Copyright (C) 2025 by Marco Papini <papini.m@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "evaluate.h"

#include "rbd.h"
#include "rbddata.h"

#include <stdlib.h>
#include <string.h>


static int computeTopologicalOrder(struct rbd *rbd, struct node *node);
static int computeBlockReliability(struct rbd *rbd, struct block *block);


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
 *      dag: RBD DAG root node used to evaluate the RBD
 *
 * Return (int):
 *  0 if the RBD Reliability curve has been evaluated successfully, < 0 otherwise
 */
int evaluateRbd(struct rbd *rbd, struct node *dag) {
    return computeTopologicalOrder(rbd, dag);
}


/**
 * computeTopologicalOrder
 *
 * Recursively compute the topological order used to process RBD DAG
 *
 * Description:
 *  This recursive function computes the topological order that is used
 *  to correctly process all RBD blocks, respecting their dependencies.
 *  The topological order is obtained with a variation of Post-order Depth
 *  First Search (DFS) algorithm adapted for DAGs. In particular, each node
 *  is initially tested for it being already analyzed, in which case no
 *  further action is needed. Otherwise, before computing the Reliability
 *  curve associated with the corresponding block, all children of the
 *  current node are recursively analyzed.
 *
 * Parameters:
 *      rbd: RBD Data Structure used to compute the topological order
 *      node: RBD DAG node for which the topological order is computed
 *
 * Return (int):
 *  0 if the topological order has been computed successfully, < 0 otherwise
 */
static int computeTopologicalOrder(struct rbd *rbd, struct node *node) {
    unsigned int idx;

    /* Process current node if and only if its associated block is not analyzed */
    if (node->block->bIsAnalyzed == 0) {
        /* For each child... */
        for (idx = 0; idx < node->numChildren; ++idx) {
            /* Recursively compute the topological order */
            if (computeTopologicalOrder(rbd, node->children[idx]) < 0) {
                return -1;
            }
        }

        /* All children have been analyzed, compute the Reliability curve of current block */
        if (computeBlockReliability(rbd, node->block) < 0) {
            return -1;
        }
    }

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

    /* Allocate memory for output reliability array */
    block->reliability = (double *)malloc(sizeof(double) * rbd->time.numTimes);
    if (block->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for Reliability curve of block %s\n", block->outputName);
        return -1;
    }

    if (block->bIsIdentical == 0) {
        rel = (double *)malloc(sizeof(double) * rbd->time.numTimes * block->numInputs);
        if (rel == NULL) {
            fprintf(stderr, "Unable to allocate memory for Reliability curve (matrix) of block %s\n", block->outputName);
            return -1;
        }

        /* Set values in reliability matrix */
        for (cIdx = 0; cIdx < block->numInputs; ++cIdx) {
            if (block->inputs[cIdx].bIsUnreliability == 0) {
                if (block->inputs[cIdx].type == INPUT_COMPONENT) {
                    memcpy(&rel[cIdx * rbd->time.numTimes],
                           rbd->components[block->inputs[cIdx].idx].reliability,
                           sizeof(double) * rbd->time.numTimes);
                }
                else {
                    memcpy(&rel[cIdx * rbd->time.numTimes],
                           rbd->blocks[block->inputs[cIdx].idx].reliability,
                           sizeof(double) * rbd->time.numTimes);
                }
            }
            else {
                if (block->inputs[cIdx].type == INPUT_COMPONENT) {
                    for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                        rel[(cIdx * rbd->time.numTimes) + tIdx] =
                                1.0 - rbd->components[block->inputs[cIdx].idx].reliability[tIdx];
                    }
                }
                else {
                    for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                        rel[(cIdx * rbd->time.numTimes) + tIdx] =
                                1.0 - rbd->blocks[block->inputs[cIdx].idx].reliability[tIdx];
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
                memcpy(rel,
                       rbd->components[block->inputs[0].idx].reliability,
                       sizeof(double) * rbd->time.numTimes);
            }
            else {
                memcpy(rel,
                       rbd->blocks[block->inputs[0].idx].reliability,
                       sizeof(double) * rbd->time.numTimes);
            }
        }
        else {
            if (block->inputs[0].type == INPUT_COMPONENT) {
                for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                    rel[tIdx] = 1.0 - rbd->components[block->inputs[0].idx].reliability[tIdx];
                }
            }
            else {
                for (tIdx = 0; tIdx < rbd->time.numTimes; ++tIdx) {
                    rel[tIdx] = 1.0 - rbd->blocks[block->inputs[0].idx].reliability[tIdx];
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

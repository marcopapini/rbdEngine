/*
 *  Component: rbd2dag.c
 *  Convert the provided RBD Data Structure into a RBD Directed Acyclic Graph (DAG)
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


#include "rbd2dag.h"

#include "dagdata.h"
#include "rbddata.h"

#include <stdlib.h>


static struct node *buildDag(const struct rbd *const rbd, struct dag *dag, const enum INPUT input, const unsigned int idx, struct node **const parent);
static struct node *searchNodeByElement(struct node *const root, const enum INPUT input, void *element);
static int checkAcyclic(struct dag *dag);
static int checkAcyclicRecursive(struct node *node);


/**
 * rbd2dag
 *
 * Convert the RBD Data Structure into the RBD DAG
 *
 * Description:
 *  This function converts the provided RBD Data Structure into the RBD DAG.
 *  Furthermore, this function checks that the created RBD DAG is valid,
 *  i.e., it contains the expected number of nodes and it is actually acyclic.
 *
 * Parameters:
 *      rbd: RBD Data Structure used to create the RBD DAG
 *      dag: RBD DAG created by this function
 *
 * Return (int):
 *  0 if the RBD DAG has been created successfully, < 0 otherwise
 */
int rbd2dag(const struct rbd *const rbd, struct dag *dag) {
    struct node *res;
    unsigned int nodeIdx;

    /* Allocate memory for current RBD DAG node */
    dag->expectedNodes = rbd->numBlocks + rbd->numComponents;
    dag->root = NULL;
    dag->pivots = NULL;
    dag->pivotsIdx = NULL;
    dag->ancestorMatrix = NULL;
    dag->root = (struct node *)calloc(dag->expectedNodes, sizeof(struct node));
    dag->pivots = (struct node **)calloc(dag->expectedNodes, sizeof(struct node *));
    dag->pivotsIdx = (unsigned int *)calloc(dag->expectedNodes, sizeof(unsigned int));
    dag->ancestorMatrix = (unsigned char *)calloc(((dag->expectedNodes + 3) / 4) * dag->expectedNodes, sizeof(unsigned char));
    if ((dag->root == NULL) || (dag->pivots == NULL) || (dag->pivotsIdx == NULL) || (dag->ancestorMatrix == NULL)) {
        fprintf(stderr, "Unable to allocate memory for RBD Directed Acyclic Graph nodes\n");
        return -1;
    }

    /* Set number of nodes in RBD DAG to 0 */
    dag->numNodes = 0;

    /* Build RBD DAG */
    res = buildDag(rbd, dag, INPUT_BLOCK, rbd->systemBlockIdx, NULL);
    if (res == NULL) {
        fprintf(stderr, "Unable to create RBD Directed Acyclic Graph\n");
        return -1;
    }

    if (dag->expectedNodes != dag->numNodes) {
        fprintf(stderr, "Invalid number of nodes in RBD DAG - Created %d - Expected %d\n", dag->numNodes, dag->expectedNodes);
        return -1;
    }

    /* Check that the RBD DAG is actually acyclic */
    if (checkAcyclic(dag) < 0) {
        return -1;
    }

    /* The root node is an ancestor of all other nodes */
    for (nodeIdx = 0; nodeIdx < dag->numNodes; ++nodeIdx) {
        setAncestor(dag, dag->root->nodeId, nodeIdx, ANCESTOR_YES);
    }

    return 0;
}


/**
 * buildDag
 *
 * Recursively build the RBD DAG
 *
 * Description:
 *  This recursive function builds the RBD DAG.
 *
 * Parameters:
 *      rbd: RBD Data Structure used to build the RBD DAG
 *      dag: RBD DAG to be built
 *      input: type of node to be inserted inside the RBD DAG
 *      idx: index of current element (referred to RBD Data Structure) to be
 *              inserted into the RBD DAG. The current element is a block if
 *              input = INPUT_BLOCK, it is a component otherwise
 *      parent: pointer to the current child of parent's node, NULL if no
 *              parent exists
 *
 * Return (struct node *):
 *  Pointer to the node inserted into the RBD DAG
 */
static struct node *buildDag(const struct rbd *const rbd, struct dag *dag, const enum INPUT input, const unsigned int idx, struct node **const parent) {
    struct node *node;
    struct node *child;
    unsigned int nodeIdx;
    unsigned char numChildren;
    unsigned char childIdx;

    /* Retrieve current node */
    node = &dag->root[dag->numNodes];
    /* Set the node ID and increment number of nodes */
    node->nodeId = dag->numNodes++;

    /* A node is always the ancestor of itself */
    setAncestor(dag, node->nodeId, node->nodeId, ANCESTOR_YES);

    /* Attach current node to the RBD DAG */
    if (parent != NULL) {
        *parent = node;
        node->refCount = 1;
    }
    node->input = input;
    if (input == INPUT_BLOCK) {
        node->component = NULL;
        node->block = &rbd->blocks[idx];
        numChildren = (node->block->bIsIdentical == 0) ? node->block->numInputs : 1;
        node->numChildren = numChildren;
        if (node->numChildren > 0) {
            node->children = (struct node **)calloc(node->numChildren, sizeof(struct node *));
            if (node->children == NULL) {
                fprintf(stderr, "Unable to allocate memory for RBD DAG children of node\n");
                return NULL;
            }
        }

        /* For each child of the current block... */
        for (childIdx = 0; childIdx < numChildren; ++childIdx) {
            /* If and only if the current input is another block */
            if (node->block->inputs[childIdx].type == INPUT_BLOCK) {
                /* Recursively search for the current input block into the RBD DAG */
                child = searchNodeByElement(dag->root, INPUT_BLOCK, &rbd->blocks[node->block->inputs[childIdx].idx]);
                /* Child node is not already included into the RBD DAG, add it */
                if (child == NULL) {
                    /* Recursively build the RBD DAG */
                    child = buildDag(rbd, dag, INPUT_BLOCK, node->block->inputs[childIdx].idx, &node->children[childIdx]);
                    if (child == NULL) {
                        return NULL;
                    }
                }
                else {
                    ++child->refCount;
                    node->children[childIdx] = child;
                }
            }
            else {
                /* Recursively search for the current input component into the RBD DAG */
                child = searchNodeByElement(dag->root, INPUT_COMPONENT, &rbd->components[node->block->inputs[childIdx].idx]);
                /* Child node is not already included into the RBD DAG, add it */
                if (child == NULL) {
                    /* Recursively build the RBD DAG */
                    child = buildDag(rbd, dag, INPUT_COMPONENT, node->block->inputs[childIdx].idx, &node->children[childIdx]);
                    if (child == NULL) {
                        return NULL;
                    }
                }
                else {
                    ++child->refCount;
                    node->children[childIdx] = child;
                }
            }
        }
    }
    else {
        node->block = NULL;
        node->component = &rbd->components[idx];
        node->numChildren = 0;
        /* The node associated with the component is not an ancestor of all other nodes */
        for (nodeIdx = 0; nodeIdx < dag->expectedNodes; ++nodeIdx) {
            if (nodeIdx != node->nodeId) {
                setAncestor(dag, node->nodeId, nodeIdx, ANCESTOR_NO);
            }
        }
    }

    return node;
}

/**
 * searchNodeByElement
 *
 * Recursively navigate the RBD DAG to search for the requested element
 *
 * Description:
 * This recursive function visits all nodes of the RBD DAG
 *  using Depth-First Search (DFS) and, if the current node
 *  references the requested element (block or component),
 *  it returns the current node.
 *
 * Parameters:
 *      node: current node used to visit the DAG using DFS algorithm
 *      input: type of node to be searched for inside the RBD DAG
 *      element: element (block or component) to be searched for
 *
 * Return (struct node *):
 *  Pointer to the RBD DAG node that references the requested block, NULL if not found
 */
static struct node *searchNodeByElement(struct node *const node, const enum INPUT input, void *element) {
    unsigned char childIdx;
    struct node *res;

    /* Is the element to be searched for a block? */
    if (input == INPUT_BLOCK) {
        /* Is the current node referencing a block? */
        if (node->input == INPUT_BLOCK) {
            /* The current RBD DAG root references the requested block */
            if (node->block == (struct block *)element) {
                return node;
            }
        }
    }
    else {
        /* Is the current node referencing a component? */
        if (node->input == INPUT_COMPONENT) {
            /* The current RBD DAG root references the requested component */
            if (node->component == (struct component *)element) {
                return node;
            }
        }
    }

    /* For each child, recursively search for the requested element */
    for (childIdx = 0; childIdx < node->numChildren; ++childIdx) {
        if (node->children[childIdx] != NULL) {
            res = searchNodeByElement(node->children[childIdx], input, element);
            if (res != NULL) {
                return res;
            }
        }
    }

    return NULL;
}

/**
 * checkAcyclic
 *
 * Navigate the RBD DAG to check if it is actually acyclic
 *
 * Description:
 *  This function navigates the whole RBD DAG to verify that the acyclic
 *  property is valid.
 *
 * Parameters:
 *      dag: RBD DAG to be checked for acyclic property
 *
 * Return (int):
 *  0 if no cycle has been detected, < 0 otherwise
 */
static int checkAcyclic(struct dag *dag) {
    /* Recursively check if the root of the RBD DAG is acyclic */
    return checkAcyclicRecursive(dag->root);
}

/**
 * checkAcyclicRecursive
 *
 * Recursively navigate the RBD DAG to check if it is actually acyclic
 *
 * Description:
 *  This recursive function visits all nodes of the RBD DAG
 *  using Depth-First Search (DFS) and, if the current node is
 *  under visit, it then detects that the RBD DAG is invalid
 *  since it actually is a directed cyclic graph.
 *
 * Parameters:
 *      node: current node used to visit the DAG using DFS algorithm
 *
 * Return (int):
 *  0 if no cycle has been detected, < 0 otherwise
 */
static int checkAcyclicRecursive(struct node *node) {
    unsigned char childIdx;

    /* Current node is under visit */
    node->status = DAG_VISITING;

    /* For each child, recursively check the absence of cycles */
    for (childIdx = 0; childIdx < node->numChildren; ++childIdx) {
        /* Check if child is under visiting, i.e., the RBD DAG is not acyclic */
        if (node->children[childIdx]->status == DAG_VISITING) {
            fprintf(stderr, "Invalid RBD Dependency DAG - Cycle detected at block %s\n",
                    node->children[childIdx]->block->outputName);
            return -1;
        }
        /* Is the child not fully visited? */
        if (node->children[childIdx]->status != DAG_VISITED) {
            /* Recursively check the child */
            if (checkAcyclicRecursive(node->children[childIdx]) < 0) {
                return -1;
            }
        }
    }

    /* Current node has been fully visited */
    node->status = DAG_VISITED;

    return 0;
}

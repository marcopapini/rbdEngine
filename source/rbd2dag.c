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

#include "rbddata.h"

#include <stdlib.h>


static unsigned int numNodes;


static struct node *buildDag(const struct rbd *const rbd, const unsigned int blockIdx, struct node *root, struct node **const parent);
static unsigned int computeNumChildred(const struct block *const block);
static struct node *searchNodeByBlock(struct node *const node, const struct block *const block);
static int checkAcyclic(struct node *node);


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
 *      rbd: RBD Data Structure used to create the RBD Parsing Tree
 *      root: set with the root of the RBD DAG
 *
 * Return (int):
 *  0 if the RBD DAG has been created successfully, < 0 otherwise
 */
int rbd2dag(const struct rbd *const rbd, struct node **root) {
    struct node *res;

    /* Allocate memory for current RBD DAG node */
    *root = (struct node *)calloc(rbd->numBlocks, sizeof(struct node));
    if (*root == NULL) {
        fprintf(stderr, "Unable to allocate memory for RBD Directed Acyclic Graph nodes\n");
        return -1;
    }

    /* Set number of nodes in RBD DAG to 0 */
    numNodes = 0;

    /* Build RBD DAG */
    res = buildDag(rbd, rbd->systemBlockIdx, *root, NULL);
    if (res == NULL) {
        fprintf(stderr, "Unable to create RBD Directed Acyclic Graph\n");
        return -1;
    }

    if (rbd->numBlocks != numNodes) {
        fprintf(stderr, "Invalid number of nodes in RBD DAG - Created %d - Expected %d\n", numNodes, rbd->numBlocks);
        return -1;
    }

    /* Check that the RBD DAG is actually acyclic */
    if (checkAcyclic(*root) < 0) {
        return -1;
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
 *      blockIdx: index of current block (referred to RBD Data Structure) to be
 *              inserted into the RBD DAG
 *      root: pointer to the root of the RBD DAG
 *      parent: pointer to the current child of parent's node, NULL if no
 *              parent exists
 *
 * Return (struct node *):
 *  Pointer to the node inserted into the RBD DAG
 */
static struct node *buildDag(const struct rbd *const rbd, const unsigned int blockIdx, struct node *root, struct node **const parent) {
    struct node *node;
    struct node *child;
    unsigned char childIdx;
    unsigned char inputIdx;

    /* Retrieve current node */
    node = &root[numNodes];
    /* Increment number of nodes */
    ++numNodes;

    /* Attach current node to the RBD DAG */
    if (parent != NULL) {
        *parent = node;
    }
    node->block = &rbd->blocks[blockIdx];
    node->numChildren = computeNumChildred(&rbd->blocks[blockIdx]);
    if (node->numChildren > 0) {
        node->children = (struct node **)calloc(node->numChildren, sizeof(struct node *));
        if (node->children == NULL) {
            fprintf(stderr, "Unable to allocate memory for RBD DAG children of node\n");
            return NULL;
        }
    }

    /* For each input of the current block... */
    for (inputIdx = 0, childIdx = 0; inputIdx < rbd->blocks[blockIdx].numInputs; ++inputIdx) {
        /* If and only if the current input is another block */
        if (rbd->blocks[blockIdx].inputs[inputIdx].type == INPUT_BLOCK) {
            /* Recursively search for the current input block into the RBD DAG */
            child = searchNodeByBlock(root, &rbd->blocks[rbd->blocks[blockIdx].inputs[inputIdx].idx]);
            /* Child node is not already included into the RBD DAG, add it */
            if (child == NULL) {
                /* Recursively build the RBD DAG */
                child = buildDag(rbd, rbd->blocks[blockIdx].inputs[inputIdx].idx, root, &node->children[childIdx]);
                if (child == NULL) {
                    return NULL;
                }
                ++childIdx;
            }
            else {
                node->children[childIdx] = child;
                ++childIdx;
            }
        }
    }

    return node;
}

/**
 * computeNumChildred
 *
 * Compute the number of RBD DAG children of the requested block
 *
 * Description:
 *  This function computes the number of children of the RBD DAG node
 *  associated with the provided block
 *
 * Parameters:
 *      block: block for which the number of children is computed
 *
 * Return (unsigned int):
 *  The number of children of the RBD DAG node
 */
static unsigned int computeNumChildred(const struct block *const block) {
    unsigned int numChildren;
    unsigned int idx;

    for (idx = 0, numChildren = 0; idx < block->numInputs; ++idx) {
        if (block->inputs[idx].type == INPUT_BLOCK) {
            ++numChildren;
        }
    }

    return numChildren;
}

/**
 * searchNodeByBlock
 *
 * Recursively navigate the RBD DAG to search for the requested block
 *
 * Description:
 *  This recursive function visits all nodes of the RBD DAG
 *  using Depth-First Search (DFS) and, if the current node
 *  references the requested block, it returns the current node.
 *
 * Parameters:
 *      node: current node used to visit the DAG using DFS algorithm
 *      block: block to be searched for
 *
 * Return (struct node *):
 *  Pointer to the RBD DAG node that references the requested block, NULL if not found
 */
static struct node *searchNodeByBlock(struct node *const node, const struct block *const block) {
    unsigned char childIdx;
    struct node *res;

    /* The current RBD DAG node references the requested block */
    if (node->block == block) {
        return node;
    }

    /* For each child, recursively search for the requested block */
    for (childIdx = 0; childIdx < node->numChildren; ++childIdx) {
        if (node->children[childIdx] != NULL) {
            res = searchNodeByBlock(node->children[childIdx], block);
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
static int checkAcyclic(struct node *node) {
    unsigned char childIdx;

    /* Check if node is under visiting, i.e., the RBD DAG is not acyclic */
    if (node->state == DAG_VISITING) {
        fprintf(stderr, "Invalid RBD Dependency DAG - Cycle detected at block %s\n", node->block->outputName);
        return -1;
    }
    /* Check if node is fully visited, no need to visit again */
    if (node->state == DAG_VISITED) {
        return 0;
    }

    /* Current node is under visit */
    node->state = DAG_VISITING;

    /* For each child, recursively check the absence of cycles */
    for (childIdx = 0; childIdx < node->numChildren; ++childIdx) {
        if (checkAcyclic(node->children[childIdx]) < 0) {
            return -1;
        }
    }

    /* Current node has been fully visited */
    node->state = DAG_VISITED;

    return 0;
}

/*
 *  Component: dagdata.c
 *  Management of DAG Data Structures
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


#include "dagdata.h"

#include <stdlib.h>


static void cleanDagNode(struct node *const node);


/**
 * cleanUpDag
 *
 * Clean up the RBD DAG
 *
 * Description:
 *  This function cleans up the provided RBD DAG by freeing
 *  the allocated memory
 *
 * Parameters:
 *      dag: pointer to the RBD DAG
 */
void cleanUpDag(struct dag *dag) {
    /* Clean-up memory allocated by RBD DAG nodes */
    if (dag->root != NULL) {
        cleanDagNode(dag->root);
        free(dag->root);
        dag->root = NULL;
    }
    /* Clean-up memory allocated by RBD DAG pivot nodes */
    if (dag->pivots != NULL) {
        free(dag->pivots);
        dag->pivots = NULL;
    }
    /* Clean-up memory allocated by RBD DAG pivot indexes */
    if (dag->pivotsIdx != NULL) {
        free(dag->pivotsIdx);
        dag->pivotsIdx = NULL;
    }
    /* Clean-up memory allocated by RBD DAG ancestors matrix */
    if (dag->ancestorMatrix != NULL) {
        free(dag->ancestorMatrix);
        dag->ancestorMatrix = NULL;
    }
}


/**
 * cleanDagNode
 *
 * Recursively clean the RBD DAG node
 *
 * Description:
 *  This recursive function cleans up all the children of the
 *  provided RBD DAG node, finally it cleans the node by freeing
 *  the allocated memory
 *
 * Parameters:
 *      node: pointer to the RBD DAG node
 */
static void cleanDagNode(struct node *const node) {
    unsigned char idx;

    /* Clean-up the current RBD DAG node */
    if (node->children != NULL) {
        /* Clean-up all children */
        for (idx = 0; idx < node->numChildren; ++idx) {
            if (node->children[idx] != NULL) {
                cleanDagNode(node->children[idx]);
            }
        }
        free(node->children);
        node->children = NULL;
    }
}

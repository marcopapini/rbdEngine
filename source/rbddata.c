/*
 *  Component: rbd_data.c
 *  Management of Data Structures equivalent to RBD Description Language
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


#include "rbddata.h"

#include <stdio.h>
#include <stdlib.h>
#include <libxml/globals.h>


static void cleanDagNode(struct node *const node);


/**
 * cleanUpRbd
 *
 * Clean up the RBD Data Structure
 *
 * Description:
 *  This function cleans up the provided RBD Data Structure by freeing
 *  the allocated memory
 *
 * Parameters:
 *      rbd: pointer to the RBD Data Structure
 */
void cleanUpRbd(struct rbd *const rbd) {
    unsigned char cIdx;
    unsigned long bIdx;

    /* Check if components array has been allocated */
    if (rbd->components != NULL) {
        /* For each component... */
        for (cIdx = 0; cIdx < rbd->numComponents; ++cIdx) {
            /* Free resources allocated during XML document parsing */
            cleanUpXmlField(&rbd->components[cIdx].name);
            if (rbd->components[cIdx].type == DST_CUSTOM) {
                cleanUpXmlField(&rbd->components[cIdx].params.c.filename);
            }
            else if (rbd->components[cIdx].type == DST_EXPONENTIAL) {
                cleanUpXmlField(&rbd->components[cIdx].params.e.outputFilename);
            }
            else if (rbd->components[cIdx].type == DST_LOGNORMAL) {
                cleanUpXmlField(&rbd->components[cIdx].params.l.outputFilename);
            }
            else if (rbd->components[cIdx].type == DST_NORMAL) {
                cleanUpXmlField(&rbd->components[cIdx].params.n.outputFilename);
            }
            else if (rbd->components[cIdx].type == DST_WEIBULL) {
                cleanUpXmlField(&rbd->components[cIdx].params.w.outputFilename);
            }
            else if (rbd->components[cIdx].type == DST_GAMMA) {
                cleanUpXmlField(&rbd->components[cIdx].params.g.outputFilename);
            }
            else {
                cleanUpXmlField(&rbd->components[cIdx].params.bs.outputFilename);
            }
            /* Free Reliability curve of component */
            if (rbd->components[cIdx].reliability != NULL) {
                free(rbd->components[cIdx].reliability);
                rbd->components[cIdx].reliability = NULL;
            }
        }
        /* Free components array */
        free(rbd->components);
        rbd->components = NULL;
    }

    /* Free resources allocated during XML document parsing */
    cleanUpXmlField(&rbd->systemBlock);

    /* Check if blocks array has been allocated */
    if (rbd->blocks != NULL) {
        /* For each block... */
        for (bIdx = 0; bIdx < rbd->numBlocks; ++bIdx) {
            /* Check if inputs array has been allocated */
            if (rbd->blocks[bIdx].inputs != NULL) {
                if (rbd->blocks[bIdx].bIsIdentical == 0) {
                    /* For each input... */
                    for (cIdx = 0; cIdx < rbd->blocks[bIdx].numInputs; ++cIdx) {
                        /* Free resources allocated during XML document parsing */
                        cleanUpXmlField(&rbd->blocks[bIdx].inputs[cIdx].name);
                    }
                }
                else {
                    /* Free resources allocated during XML document parsing */
                    cleanUpXmlField(&rbd->blocks[bIdx].inputs[0].name);
                }
                /* Free inputs array */
                free(rbd->blocks[bIdx].inputs);
                rbd->blocks[bIdx].inputs = NULL;
            }
            /* Free resources allocated during XML document parsing */
            cleanUpXmlField(&rbd->blocks[bIdx].outputName);
            cleanUpXmlField(&rbd->blocks[bIdx].outputFilename);
            /* Free Reliability curve of block */
            if (rbd->blocks[bIdx].reliability != NULL) {
                free(rbd->blocks[bIdx].reliability);
                rbd->blocks[bIdx].reliability = NULL;
            }
        }
        /* Free blocks array */
        free(rbd->blocks);
        rbd->blocks = NULL;
    }
}

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
 * cleanUpXmlField
 *
 * Clean up a XML field
 *
 * Description:
 *  This function cleans up the field created during XML
 *  document parsing by freeing the allocated memory.
 *
 * Parameters:
 *      field: field to be cleaned up
 */
void cleanUpXmlField(char **field) {
    /* Free resources allocated during XML document parsing */
    if (*field != NULL) {
        xmlFree(*field);
        *field = NULL;
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

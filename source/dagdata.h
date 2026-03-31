/*
 *  Component: rbd_data.h
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

#ifndef DAGDATA_H
#define DAGDATA_H


#include "common.h"
#include "rbddata.h"

#include <stdio.h>


#define ANCESTOR_INFO_PER_BYTE  0x4     /* Number of ancestor information per byte */

#define ANCESTOR_MASK           0x3     /* Bitmask to extract ancestor information */
#define ANCESTOR_BYTE_RSHIFT    0x2     /* Right shift to extract byte from row */
#define ANCESTOR_BYTE_LSHIFT    0x1     /* Left shift to extract ancestor from byte */
#define ANCESTOR_BYTE_MASK      0x3     /* Bitmask to extract ancestor from byte */

#define ANCESTOR_UNKNOWN        0x0     /* Ancestor information not available */
#define ANCESTOR_NO             0x2     /* Node is not an ancestor */
#define ANCESTOR_YES            0x3     /* Node is an ancestor */
#define IS_ANCESTOR_MASK        0x1     /* Bitmask to extract if node is effectively an ancestor */


/**
 * Enumeration of possible states during traversal visiting of DAG
 */
enum DAG {
    DAG_UNVISITED = 0,      /* DAG node is unvisited */
    DAG_VISITING,           /* DAG node is currently under visit */
    DAG_VISITED,            /* DAG node is visited */
    DAG_COMPUTED,           /* DAG node has been fully computed */
    DAG_COMPUTED_PIVOT,     /* DAG node has been computed, but it is a pivot node for CDM */
    DAG_DEFERRED_CDM        /* DAG node can be evaluated only through CDM */
};

/**
 * Data structure representing a Node of the RBD Directed Acyclic Graph (DAG)
 */
struct node {
    unsigned int nodeId;
    enum INPUT input;
    struct block *block;
    struct component *component;
    unsigned int refCount;
    struct node **children;
    unsigned char numChildren;
    enum DAG status;
};

/**
 * Data structure representing the RBD Directed Acyclic Graph (DAG)
 */
struct dag {
    struct node *root;
    unsigned int numNodes;
    unsigned int expectedNodes;
    unsigned char *ancestorMatrix;
    struct node **pivots;
    unsigned int numPivots;
    unsigned int *pivotsIdx;
    unsigned int numPivotIdx;
};

/**
 * getAncestor
 *
 * Get the ancestor information
 *
 * Description:
 *  This function gets the ancestor information of the provided two node identifiers
 *
 * Parameters:
 *      dag: RBD DAG used for the get operation
 *      firstId: identifier of the first node (possible ancestor)
 *      secondId: identifier of the second node (possible descendant)
 *
 * Return (unsigned char):
 *  Ancestor information as follows: ANCESTOR_UNKOWN if no information is available,
 *  ANCESTOR_NO if the first node is not an ancestor of second one, ANCESTOR_YES otherwise
 */
static inline unsigned char getAncestor(struct dag *dag, unsigned int firstId, unsigned int secondId) {
    unsigned char byte;

    /* Retrieve the byte containing the ancestor information */
    byte = dag->ancestorMatrix[(firstId * ceilDivision(dag->expectedNodes, ANCESTOR_INFO_PER_BYTE)) +
                                  (secondId >> ANCESTOR_BYTE_RSHIFT)];
    /* Extract the ancestor information from the byte */
    return (byte >> ((secondId & ANCESTOR_BYTE_MASK) << ANCESTOR_BYTE_LSHIFT)) & ANCESTOR_MASK;
}

/**
 * setAncestor
 *
 * Set the ancestor information
 *
 * Description:
 *  This function sets the ancestor information of the provided two node identifiers
 *
 * Parameters:
 *      dag: RBD DAG used for the set operation
 *      firstId: identifier of the first node (possible ancestor)
 *      secondId: identifier of the second node (possible descendant)
 *      info: ancestor information to be set
 */
static inline void setAncestor(struct dag *dag, unsigned int firstId, unsigned int secondId, unsigned char info) {
    unsigned int bitPos;
    unsigned int byteIdx;
    unsigned char currentByte;

    /* Compute the byte index of the element to update */
    byteIdx = (firstId * ceilDivision(dag->expectedNodes, ANCESTOR_INFO_PER_BYTE)) + (secondId >> ANCESTOR_BYTE_RSHIFT);
    /* Compute the bitmask position of the element to update */
    bitPos = (secondId & ANCESTOR_BYTE_MASK) << ANCESTOR_BYTE_LSHIFT;

    /* Retrieve the element to update */
    currentByte = dag->ancestorMatrix[byteIdx];

    /* Update the information of the ancestor */
    dag->ancestorMatrix[byteIdx] = currentByte ^ ((currentByte ^ (info << bitPos)) & (ANCESTOR_MASK << bitPos));
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
void cleanUpDag(struct dag *dag);


#endif /* DAGDATA_H */

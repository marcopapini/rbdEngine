/*
 *  Component: rbd_data.h
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

#ifndef RBDDATA_H
#define RBDDATA_H


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
 * Enumeration of possible distribution types
 */
enum DST {
    DST_CUSTOM = 0,         /* Custom distribution, retrieved from file */
    DST_EXPONENTIAL,        /* Exponential distribution */
    DST_LOGNORMAL,          /* Log-normal distribution */
    DST_NORMAL,             /* Normal distribution */
    DST_WEIBULL,            /* Weibull distribution */
    DST_GAMMA,              /* Gamma distribution */
    DST_BIRNBAUM_SAUNDERS   /* Birnbaum-Saunders distribution */
};

/**
 * Enumeration of possible RBD basic blocks
 */
enum BLK {
    BLK_BRIDGE = 0,         /* Bridge block */
    BLK_KOON,               /* KooN block */
    BLK_PARALLEL,           /* Parallel block */
    BLK_SERIES              /* Series block */
};

/**
 * Enumeration of possible inputs of a given RBD block
 */
enum INPUT {
    INPUT_COMPONENT = 0,    /* Input is a RBD Component */
    INPUT_BLOCK             /* Input is a RBD Block */
};

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
 * Data structure representing the RBD Description Language <rbd/time> element
 */
struct time {
    double start;
    double end;
    double step;
    unsigned int numTimes;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/custom> element
 */
struct custom {
    char *filename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/exponential> element
 */
struct exponential {
    double lambda;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/lognormal> element
 */
struct lognormal {
    double mu;
    double sigma;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/normal> element
 */
struct normal {
    double mu;
    double sigma;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/weibull> element
 */
struct weibull {
    double lambda;
    double k;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/gamma> element
 */
struct gamma {
    double alpha;
    double theta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/birnbaum_saunders> element
 */
struct birnbaum_saunder {
    double alpha;
    double beta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component> element
 */
struct component {
    char *name;
    enum DST type;
    union params {
        struct custom c;
        struct exponential e;
        struct lognormal l;
        struct normal n;
        struct weibull w;
        struct gamma g;
        struct birnbaum_saunder bs;
    } params;
    double *reliability;
    unsigned char bIsForced;
    double forcedValue;
};

/**
 * Data structure representing the RBD Description Language <rbd/blocks/block/input> element
 */
struct input {
    char *name;
    unsigned char bIsUnreliability;
    enum INPUT type;
    unsigned int idx;
};

/**
 * Data structure representing the RBD Description Language <rbd/blocks/block> element
 */
struct block {
    enum BLK type;
    unsigned char bIsIdentical;
    unsigned char numInputs;
    unsigned char minInputs;
    struct input *inputs;
    char *outputName;
    char *outputFilename;
    unsigned char bIsAnalyzed;
    unsigned char bIsForced;
    double forcedValue;
    double *reliability;
};

/**
 * Data structure representing the RBD Description Language <rbd> element
 */
struct rbd {
    struct time time;
    unsigned char numComponents;
    unsigned int numBlocks;
    char *systemBlock;
    unsigned int systemBlockIdx;
    struct component *components;
    struct block *blocks;
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
 * floorDivision
 *
 * Compute floor value of division
 *
 * Description:
 *  Computes the floor value of the requested division
 *
 * Parameters:
 *      dividend: dividend of division
 *      divisor: divisor of division
 *
 * Return (int):
 *  Floor value of division
 */
static inline int floorDivision(int dividend, int divisor) {
    return (dividend / divisor);
}

/**
 * ceilDivision
 *
 * Compute ceil value of division
 *
 * Description:
 *  Computes the ceil value of the requested division
 *
 * Parameters:
 *      dividend: dividend of division
 *      divisor: divisor of division
 *
 * Return (int):
 *  Ceil value of division
 */
static inline int ceilDivision(int dividend, int divisor) {
    return floorDivision(dividend + divisor - 1, divisor);
}

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
void cleanUpRbd(struct rbd *const rbd);

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
void cleanUpXmlField(char **field);


#endif /* RBDDATA_H */

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


/**
 * Enumeration of possible distribution types
 */
enum DST {
    DST_CUSTOM = 0,         /* Custom distribution, retrieved from file */
    DST_EXPONENTIAL,        /* Exponential distribution */
    DST_LOGNORMAL,          /* Log-normal distribution */
    DST_NORMAL,             /* Normal distribution */
    DST_WEIBULL             /* Weibull distribution */
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
    DAG_COMPUTED            /* Reliability curve of DAG node has been computed */
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
    } params;
    double *reliability;
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
    struct block *block;
    struct node **children;
    unsigned char numChildren;
    enum DAG state;
};


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
void cleanUpDag(struct node *const dag);


#endif /* RBDDATA_H */

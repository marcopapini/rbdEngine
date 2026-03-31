/*
 *  Component: rbddata.h
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
    DST_CUSTOM = 0,         /* Custom Reliability distribution, retrieved from file */
    DST_EXPONENTIAL,        /* Exponential Reliability distribution */
    DST_LOGNORMAL,          /* Log-normal Reliability distribution */
    DST_NORMAL,             /* Normal Reliability distribution */
    DST_WEIBULL,            /* Weibull Reliability distribution */
    DST_GAMMA,              /* Gamma Reliability distribution */
    DST_BIRNBAUM_SAUNDERS,  /* Birnbaum-Saunders Reliability distribution */
    DST_GOEL_OKUMOTO,       /* Goel-Okumoto NHPP distribution */
    DST_YAMADA_S_SHAPED,    /* Yamada S-Shaped NHPP distribution */
    DST_MUSA_OKUMOTO,       /* Musa-Okumoto NHPP distribution */
    DST_OHBA_S_SHAPED,      /* Ohba S-Shaped NHPP distribution */
    DST_GOEL_GENERALIZED,   /* Goel Generalized NHPP distribution */
    DST_KAPUR_GARG_3S,      /* Kapur-Garg 3-Stage NHPP distribution */
    DST_PHAM_ZHANG          /* Pham-Zhang NHPP distribution */
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
 * Data structure representing the RBD Description Language <rbd/components/component/birnbaum-saunders> element
 */
struct birnbaum_saunders {
    double alpha;
    double beta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/goel-okumoto> element
 */
struct goel_okumoto {
    double a;
    double b;
    double test;
    double offset;
    double eta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/yamada_s-shaped> element
 */
struct yamada_s_shaped {
    double a;
    double b;
    double test;
    double offset;
    double eta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/musa-okumoto> element
 */
struct musa_okumoto {
    double lambda;
    double theta;
    double test;
    double offset;
    double eta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/ohba_s-shaped> element
 */
struct ohba_s_shaped {
    double a;
    double b;
    double phi;
    double test;
    double offset;
    double eta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/goel_generalized> element
 */
struct goel_generalized {
    double a;
    double b;
    double c;
    double test;
    double offset;
    double eta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/kapur-garg_3> element
 */
struct kapur_garg_3 {
    double a;
    double b;
    double test;
    double offset;
    double eta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component/pham-zhang> element
 */
struct pham_zhang {
    double a;
    double b;
    double alpha;
    double beta;
    double test;
    double offset;
    double eta;
    char *outputFilename;
};

/**
 * Data structure representing the RBD Description Language <rbd/components/component> element
 */
struct component {
    char *name;
    enum DST type;
    union params {
        struct custom rel_c;
        struct exponential rel_e;
        struct lognormal rel_l;
        struct normal rel_n;
        struct weibull rel_w;
        struct gamma rel_g;
        struct birnbaum_saunders rel_bs;
        struct goel_okumoto nhpp_go;
        struct yamada_s_shaped nhpp_yss;
        struct musa_okumoto nhpp_mo;
        struct ohba_s_shaped nhpp_oss;
        struct goel_generalized nhpp_gg;
        struct kapur_garg_3 nhpp_kg3;
        struct pham_zhang nhpp_pz;
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

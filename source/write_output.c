/*
 *  Component: write_output.c
 *  Write the Reliability curve to an output file
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


#include "write_output.h"

#include "rbddata.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>


#define TMP_STRING_SIZE         1000


static char tmpString[TMP_STRING_SIZE];


static int writeComponentReliabilityCurve(struct time *time, struct component *component, char *outputFname);
static int writeBlockReliabilityCurve(struct time *time, struct block *block);


/**
 * writeOutputFiles
 *
 * Write the Reliability curves
 *
 * Description:
 *  This function navigates the RBD Data Structure and, for each RBD block
 *  for which the output reliability curve has be written to file, it
 *  writes it to the requested file
 *
 * Parameters:
 *      rbd: RBD Data Structure
 *
 * Return (int):
 *  0 if the Reliability curves have been written successfully, < 0 otherwise
 */
int writeOutputFiles(struct rbd *rbd) {
    unsigned int idx;

    for (idx = 0; idx < rbd->numComponents; ++idx) {
        switch (rbd->components[idx].type) {
            case DST_EXPONENTIAL:
                if (rbd->components[idx].params.e.outputFilename != NULL) {
                    if (writeComponentReliabilityCurve(&rbd->time, &rbd->components[idx],
                                rbd->components[idx].params.e.outputFilename) < 0) {
                        return -1;
                    }
                }
                break;

            case DST_LOGNORMAL:
                if (rbd->components[idx].params.l.outputFilename != NULL) {
                    if (writeComponentReliabilityCurve(&rbd->time, &rbd->components[idx],
                                rbd->components[idx].params.l.outputFilename) < 0) {
                        return -1;
                    }
                }
                break;

            case DST_NORMAL:
                if (rbd->components[idx].params.n.outputFilename != NULL) {
                    if (writeComponentReliabilityCurve(&rbd->time, &rbd->components[idx],
                                rbd->components[idx].params.n.outputFilename) < 0) {
                        return -1;
                    }
                }
                break;

            case DST_WEIBULL:
                if (rbd->components[idx].params.w.outputFilename != NULL) {
                    if (writeComponentReliabilityCurve(&rbd->time, &rbd->components[idx],
                                rbd->components[idx].params.w.outputFilename) < 0) {
                        return -1;
                    }
                }
                break;

            case DST_CUSTOM:
            default:
                break;
        }
    }
    for (idx = 0; idx < rbd->numBlocks; ++idx) {
        if (rbd->blocks[idx].outputFilename != NULL) {
            if (writeBlockReliabilityCurve(&rbd->time, &rbd->blocks[idx]) < 0) {
                return -1;
            }
        }
    }

    return 0;
}


/**
 * writeComponentReliabilityCurve
 *
 * Write the Reliability curve of the provided component
 *
 * Description:
 *  This function writes the Reliability curve associated with the
 *  provided component to the proper output file
 *
 * Parameters:
 *      time: RBD Time Data Structure
 *      component: RBD component
 *      outputFname: filename of output file
 *
 * Return (int):
 *  0 if the Reliability curve has been written successfully, < 0 otherwise
 */
static int writeComponentReliabilityCurve(struct time *time, struct component *component, char *outputFname) {
    unsigned int idx;
    FILE *pOutput;

    /* Open file for writing output reliability curve */
    pOutput = fopen(outputFname, "w");
    if (pOutput == NULL) {
        fprintf(stderr, "Unable to open file output file %s for component %s\n", outputFname, component->name);
        return -1;
    }

    /* Write output file header. Reliability Curve header starts as follows:
     *
     *  From: <FROM_TIME>
     *  To: <TO_TIME>
     *  Step: <STEP>
     *  Values: "
     *
     * Where <FROM_TIME> and <TO_TIME> are respectively the start and end time of the
     * reliability curve and <STEP> is the time discretization.
     */
    snprintf(tmpString, TMP_STRING_SIZE, "From: %.4f\n", time->start);
    if (fputs(tmpString, pOutput) == EOF) {
        fprintf(stderr, "Unable to write reliability curve header to file %s\n", outputFname);
        fclose(pOutput);
        return -1;
    }
    snprintf(tmpString, TMP_STRING_SIZE, "To: %.4f\n", time->end);
    if (fputs(tmpString, pOutput) == EOF) {
        fprintf(stderr, "Unable to write reliability curve header to file %s\n", outputFname);
        fclose(pOutput);
        return -1;
    }
    snprintf(tmpString, TMP_STRING_SIZE, "Step: %.4f\n", time->step);
    if (fputs(tmpString, pOutput) == EOF) {
        fprintf(stderr, "Unable to write reliability curve header to file %s\n", outputFname);
        fclose(pOutput);
        return -1;
    }
    if (fputs("Values: \n", pOutput) == EOF) {
        fprintf(stderr, "Unable to write reliability curve header to file %s\n", outputFname);
        fclose(pOutput);
        return -1;
    }

    /* For each point over reliability curve... */
    for (idx = 0; idx < time->numTimes; ++idx) {
        /* Write point over reliability curve to file */
        snprintf(tmpString, TMP_STRING_SIZE, "%.10e\n", component->reliability[idx]);
        if (fputs(tmpString, pOutput) == EOF) {
            fprintf(stderr, "Unable to write reliability curve to file %s\n", outputFname);
            fclose(pOutput);
            return -1;
        }
    }

    /* Close output file */
    fclose(pOutput);

    return 0;
}

/**
 * writeBlockReliabilityCurve
 *
 * Write the Reliability curve of the provided block
 *
 * Description:
 *  This function writes the Reliability curve associated with the
 *  provided block to the proper output file
 *
 * Parameters:
 *      time: RBD Time Data Structure
 *      block: RBD block
 *
 * Return (int):
 *  0 if the Reliability curve has been written successfully, < 0 otherwise
 */
static int writeBlockReliabilityCurve(struct time *time, struct block *block) {
    unsigned int idx;
    FILE *pOutput;

    /* Open file for writing output reliability curve */
    pOutput = fopen(block->outputFilename, "w");
    if (pOutput == NULL) {
        fprintf(stderr, "Unable to open file output file %s for block %s\n", block->outputFilename, block->outputName);
        return -1;
    }

    /* Write output file header. Reliability Curve header starts as follows:
     *
     *  From: <FROM_TIME>
     *  To: <TO_TIME>
     *  Step: <STEP>
     *  Values: "
     *
     * Where <FROM_TIME> and <TO_TIME> are respectively the start and end time of the
     * reliability curve and <STEP> is the time discretization.
     */
    snprintf(tmpString, TMP_STRING_SIZE, "From: %.4f\n", time->start);
    if (fputs(tmpString, pOutput) == EOF) {
        fprintf(stderr, "Unable to write reliability curve header to file %s\n", block->outputFilename);
        fclose(pOutput);
        return -1;
    }
    snprintf(tmpString, TMP_STRING_SIZE, "To: %.4f\n", time->end);
    if (fputs(tmpString, pOutput) == EOF) {
        fprintf(stderr, "Unable to write reliability curve header to file %s\n", block->outputFilename);
        fclose(pOutput);
        return -1;
    }
    snprintf(tmpString, TMP_STRING_SIZE, "Step: %.4f\n", time->step);
    if (fputs(tmpString, pOutput) == EOF) {
        fprintf(stderr, "Unable to write reliability curve header to file %s\n", block->outputFilename);
        fclose(pOutput);
        return -1;
    }
    if (fputs("Values: \n", pOutput) == EOF) {
        fprintf(stderr, "Unable to write reliability curve header to file %s\n", block->outputFilename);
        fclose(pOutput);
        return -1;
    }

    /* For each point over reliability curve... */
    for (idx = 0; idx < time->numTimes; ++idx) {
        /* Write point over reliability curve to file */
        snprintf(tmpString, TMP_STRING_SIZE, "%.10e\n", block->reliability[idx]);
        if (fputs(tmpString, pOutput) == EOF) {
            fprintf(stderr, "Unable to write reliability curve to file %s\n", block->outputFilename);
            fclose(pOutput);
            return -1;
        }
    }

    /* Close output file */
    fclose(pOutput);

    return 0;
}

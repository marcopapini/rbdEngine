/*
 *  Component: main.c
 *  RBD Evaluation Engine
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
#include "rbd2dag.h"
#include "rbddata.h"
#include "validate.h"
#include "write_output.h"
#include "xml2rbd.h"

#include <stdio.h>
#include <libxml/parser.h>


static xmlDocPtr doc;
static struct rbd rbd;
static struct dag dag;


static void cleanUp();


/**
 * main
 *
 * Entry-point of RBD Engine application
 *
 * Description:
 *  This function is the entry-point of the RBD Engine application.
 *  It validates the provided RBDDL file, it processes it to convert
 *  it into an RBD model and it evaluates the reliability of the
 *  overall model, also producing the requested output files
 *
 * Parameters:
 *      argc: Number of arguments passed from CLI
 *      argv: List of arguments passed from CLI
 *
 * Return (int):
 *  0 if the RBD Engine has terminated successfully, < 0 otherwise
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Invalid number of argument\nUsage: %s <XML_DOCUMENT>", argv[0]);
        return -1;
    }

    /* Initialize libxml2 parser */
    xmlInitParser();

    /* Read XML document */
    doc = xmlReadFile(argv[1], NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Unable to read XML document %s\n", argv[1]);
        cleanUp();
        return -1;
    }

    /* Validate XML document against its XML Schema */
    if (validate(doc) < 0) {
        cleanUp();
        return -1;
    }

    /* Convert the XML document into the RBD Data Structure */
    if (xml2rbd(doc, &rbd) < 0) {
        cleanUp();
        return -1;
    }

    /* Convert the RBD Data Structure into the RBD DAG */
    if (rbd2dag(&rbd, &dag) < 0) {
        cleanUp();
        return -1;
    }

    /* Evaluate the Reliability curve of the overall RBD */
    if (evaluateRbd(&rbd, &dag) < 0) {
        cleanUp();
        return -1;
    }

    /* Write the Reliability curves to the proper output files */
    if (writeOutputFiles(&rbd) < 0) {
        cleanUp();
        return -1;
    }

    /* Clean-up resources */
    cleanUp();

    return 0;
}


/**
 * cleanUp
 *
 * Clean-up internal resources used by the RBD Engine
 *
 * Description:
 *  This function cleans up the internal resources used by the RBD Engine,
 *  i.e., the ones used during the processing of the provided XML document,
 *  the creation of the RBD Data Structure and the RBD Parsing Tree
 */
static void cleanUp() {
    /* Check if XML document is open and eventually close it */
    if (doc != NULL) {
        xmlFreeDoc(doc);
    }
    /* Clean-up XML Schema validation data */
    cleanUpValidation();
    /* Clean-up the XML Parser */
    xmlCleanupParser();

    /* Clean-up the RBD Parsing Tree */
    cleanUpDag(&dag);
    /* Clean-up the RBD Data Structure */
    cleanUpRbd(&rbd);
}

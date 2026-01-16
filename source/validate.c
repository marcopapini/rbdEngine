/*
 *  Component: validate.c
 *  Validate the provided RBD Description Language file against the Schema
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


#include "validate.h"

#include "rbddl.h"

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/xmlschemas.h>


static xmlSchemaParserCtxt *schemaParserCtxt;
static xmlSchema *schema;
static xmlSchemaValidCtxt *schemaValidationCtxt;


/**
 * validate
 *
 * Validate the XML document against the RBD Description Language Schema
 *
 * Description:
 *  This function checks if the provided XML document is valid w.r.t. the XML RBD
 *  Description Language Schema file
 *
 * Parameters:
 *      document: pointer to the XML document
 *
 * Return (int):
 *  0 if the XML document is valid, < 0 otherwise
 */
int validate(xmlDoc *const document) {
    int res;

    /* Create the XML Schema parser */
    schemaParserCtxt = xmlSchemaNewMemParserCtxt((char *)rbddl_xsd, rbddl_xsd_len);
    if (schemaParserCtxt == NULL) {
        return -1;
    }

    /* Parse the XML Schema and build an internal XML Schema structure */
    schema = xmlSchemaParse(schemaParserCtxt);
    if (schema == NULL) {
        return -1;
    }

    /* Create an XML Schema validation context */
    schemaValidationCtxt = xmlSchemaNewValidCtxt(schema);
    if (schemaValidationCtxt == NULL) {
        return -1;
    }

    /* Validate the XML document against the XML Schema */
    res = xmlSchemaValidateDoc(schemaValidationCtxt, document);
    if (res != 0) {
        res = -1;
    }

    return res;
}

/**
 * cleanUpValidation
 *
 * Clean-up internal resources used for the XML validation
 *
 * Description:
 *  This function cleans up the internal libxml2 resources used during the
 *  validation of the provided XML document against the RBD Description
 *  Language Schema file
 */
void cleanUpValidation() {
    /* Free resources used for XML Schema validation */
    if (schemaValidationCtxt != NULL) {
        xmlSchemaFreeValidCtxt(schemaValidationCtxt);
    }
    if (schema != NULL) {
        xmlSchemaFree(schema);
    }
    if (schemaParserCtxt != NULL) {
        xmlSchemaFreeParserCtxt(schemaParserCtxt);
    }
}

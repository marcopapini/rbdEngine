/*
 *  Component: validate.h
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

#ifndef VALIDATE_H
#define VALIDATE_H


#include <libxml/tree.h>


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
int validate(xmlDoc *const document);

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
void cleanUpValidation();


#endif /* VALIDATE_H */

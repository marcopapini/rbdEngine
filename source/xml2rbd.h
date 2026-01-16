/*
 *  Component: xml2rbd.h
 *  Convert the provided RBD Description Language file into an RBD Data Structure
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

#ifndef XML2RBD_H
#define XML2RBD_H


#include "rbddata.h"

#include <libxml/tree.h>


/**
 * xml2rbd
 *
 * Convert the XML RBD Description Language document into the RBD Data Structure
 *
 * Description:
 *  This function reads the provided XML RBD Description Language document and it
 *  converts it into the provided RBD Data Structure
 *
 * Parameters:
 *      document: pointer to the XML RBD Description Language document
 *      rbd: pointer to the filled RBD Data Structure
 *
 * Return (int):
 *  0 if the conversion into the RBD Data Structure is successful, < 0 otherwise
 */
int xml2rbd(const xmlDoc *const doc, struct rbd *const rbd);


#endif /* XML2RBD_H */

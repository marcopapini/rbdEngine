/*
 *  Component: write_output.h
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


#ifndef WRITE_OUTPUT_H
#define WRITE_OUTPUT_H


#include "rbddata.h"


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
int writeOutputFiles(struct rbd *rbd);


#endif /* WRITE_OUTPUT_H */

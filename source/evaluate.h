/*
 *  Component: evaluate.h
 *  Evaluate the Reliability curve of a RBD
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


#ifndef EVALUATE_H
#define EVALUATE_H


#include "dagdata.h"
#include "rbddata.h"


/**
 * evaluateRbd
 *
 * Evaluate the provided RBD exploiting the supporting RBD DAG
 *
 * Description:
 *  This function evaluates the Reliability curve of the provided
 *  RBD by exploiting the supporting RBD DAG by reconstructing the
 *  topological order needed to evaluate RBD blocks while respecting
 *  the block dependencies.
 *
 * Parameters:
 *      rbd: RBD Data Structure to be evaluated
 *      dag: RBD DAG root node used to evaluate the RBD
 *
 * Return (int):
 *  0 if the RBD Reliability curve has been evaluated successfully, < 0 otherwise
 */
int evaluateRbd(struct rbd *rbd, struct dag *dag);


#endif /* EVALUATE_H */

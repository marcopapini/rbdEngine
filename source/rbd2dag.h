/*
 *  Component: rbd2dag.h
 *  Convert the provided RBD Data Structure into a RBD Directed Acyclic Graph (DAG)
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

#ifndef RBD2DAG_H
#define RBD2DAG_H


#include "rbddata.h"


/**
 * rbd2dag
 *
 * Convert the RBD Data Structure into the RBD DAG
 *
 * Description:
 *  This function converts the provided RBD Data Structure into the RBD DAG.
 *  Furthermore, this function checks that the created RBD DAG is valid,
 *  i.e., it contains the expected number of nodes and it is actually acyclic.
 *
 * Parameters:
 *      rbd: RBD Data Structure used to create the RBD DAG
 *      root: set with the root of the RBD DAG
 *
 * Return (int):
 *  0 if the RBD DAG has been created successfully, < 0 otherwise
 */
int rbd2dag(const struct rbd *const rbd, struct node **root);


#endif /* RBD2DAG_H */

/*
 *  Component: common.h
 *  Common functionalities of rbdEngine
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

#ifndef COMMON_H
#define COMMON_H


/**
 * floorDivision
 *
 * Compute floor value of division
 *
 * Description:
 *  Computes the floor value of the requested division
 *
 * Parameters:
 *      dividend: dividend of division
 *      divisor: divisor of division
 *
 * Return (int):
 *  Floor value of division
 */
static inline int floorDivision(int dividend, int divisor) {
    return (dividend / divisor);
}

/**
 * ceilDivision
 *
 * Compute ceil value of division
 *
 * Description:
 *  Computes the ceil value of the requested division
 *
 * Parameters:
 *      dividend: dividend of division
 *      divisor: divisor of division
 *
 * Return (int):
 *  Ceil value of division
 */
static inline int ceilDivision(int dividend, int divisor) {
    return floorDivision(dividend + divisor - 1, divisor);
}


#endif /* COMMON_H */

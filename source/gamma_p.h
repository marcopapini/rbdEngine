/*
 *  Component: gamma_p.h
 *  Compute the regularized lower incomplete gamma function P(a, x)
 *  It combines ASA147 with the modified continued fraction Lentz's method.
 *
 *  rbdEngine - Evaluate the reliability of a given Reliability Block Diagram
 *  through the RBD Description Language file.
 *  Copyright (C) 2025 by Marco Papini <papini.m@gmail.com>
 *
 *  CREDITS & ATTRIBUTION:
 *  1. ASA147:
 *     Original FORTRAN77 version by Chi Leung Lau
 *     Reference: Applied Statistics, Volume 29, Number 1, 1980, pp. 113-114
 *     C implementation reference by John Burkardt (distributed under MIT license)
 *     The derived logic remains compatible with its original MIT-style attribution requirements.
 *  2. Continued Fraction Method (Lentz):
 *     Based on the modified Lentz's method for evaluating continued fractions
 *     Reference: I. J. Thompson, A. R. Barnett,
 *     "Modified Lentz's method for computing continued fractions", 1986.
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


#ifndef GAMMA_P_H_
#define GAMMA_P_H_


/**
 * gamma_p
 *
 * Compute the regularized lower incomplete gamma function P(a, x)
 *
 * Description:
 *  This function computes the regularized lower incomplete gamma function P(a, x)
 *  using an hybrid method leveraging both ASA147 and the modified Lentz's method.
 *
 * Parameters:
 *      a: shape parameter
 *      x: upper limit of integration
 *
 * Return (double):
 *  The regularized lower incomplete gamma function P(a, x)
 */
double gamma_p(double a, double x);


#endif /* GAMMA_P_H_ */

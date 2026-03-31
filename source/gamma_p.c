/*
 *  Component: gamma_p.c
 *  Compute the regularized lower incomplete gamma function P(a, x)
 *  It combines ASA147 with the continued fraction method of Lentz.
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


#include <stdio.h>
#include <math.h>
#include <float.h>


/* Safety and precision constants */
#define ITMAX           5000         /* Maximum iterations to prevent infinite loops */
#define FPMIN           1.0e-100     /* Smallest representable value for continued fractions */


static double gamma_p_asa147(double a, double x);
static double gamma_q_lentz(double a, double x);


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
double gamma_p(double a, double x) {
    double res;

    /* Domain validation */
    if (a <= 0.0) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 0.0;
    }
    if (x >= DBL_MAX || a >= DBL_MAX) {
        return 1.0;
    }

    /*
     * Hybrid method:
     * - Use ASA147 if x < a + 1
     * - Use modified Lentz's method if x >= a + 1
     */
    if (x < a + 1.0) {
        res = gamma_p_asa147(a, x);
    }
    else {
        res = 1.0 - gamma_q_lentz(a, x);
    }
    /* Ensure result is within [0, 1] due to potential precision jitter */
    return fmax(fmin(1.0, res), 0.0);
}


/**
 * gamma_p_asa147
 *
 * Compute the regularized lower incomplete gamma function P(a, x) as in ASA147
 *
 * Description:
 *  This function computes the regularized lower incomplete gamma function P(a, x)
 *  as described in ASA147.
 *
 * Parameters:
 *      a: shape parameter
 *      x: upper limit of integration
 *
 * Return (double):
 *  The regularized lower incomplete gamma function P(a, x) computed with ASA147
 */
static double gamma_p_asa147(double a, double x)
{
    double arg;
    double f;
    double c;
    double value;
    double p;
    int n;

    /*
     * Logarithm of the pre-factor f
     * log(f) = log((x^a * e^-x) / Gamma(a+1))
     * Note: log(Gamma(a+1)) = log(Gamma(a)) + log(a)
     */
    arg = a * log(x) - x - (lgamma(a) + log(a));

    /* Underflow check: if the scaling factor is too small, result is 0 */
    if (arg < log(DBL_MIN))
    {
        return 0.0;
    }

    f = exp(arg);
    c = 1.0;
    value = 1.0;
    p = a;

    /* ASA147 Series Expansion */
    for (n = 0; n < ITMAX; ++n) {
        p += 1.0;
        c = c * x / p;
        value += c;

        /* Convergence check using machine epsilon */
        if (c <= DBL_EPSILON * value)
        {
            break;
        }
    }

    return value * f;
}

/**
 * gamma_q_lentz
 *
 * Compute the regularized upper incomplete gamma function Q(a, x) using modified Lentz's method
 *
 * Description:
 *  This function computes the regularized upper incomplete gamma function Q(a, x)
 *  using the modified Lentz's method.
 *
 * Parameters:
 *      a: shape parameter
 *      x: upper limit of integration
 *
 * Return (double):
 *  The regularized upper incomplete gamma function Q(a, x) computed with modified Lentz's method
 */
static double gamma_q_lentz(double a, double x)
{
    double b;
    double c;
    double d;
    double h;
    double an;
    int i;

    b = x + 1.0 - a;
    c = 1.0 / FPMIN;
    d = 1.0 / b;
    h = d;

    for (i = 1; i <= ITMAX; ++i) {
        an = -i * (i - a);
        b += 2.0;
        d = an * d + b;
        if (fabs(d) < FPMIN) {
            d = FPMIN;
        }
        c = b + an / c;
        if (fabs(c) < FPMIN) {
            c = FPMIN;
        }
        d = 1.0 / d;
        double del = d * c;
        h *= del;

        if (fabs(del - 1.0) <= DBL_EPSILON) {
            break;
        }
    }

    return exp(a * log(x) - x - lgamma(a)) * h;
}

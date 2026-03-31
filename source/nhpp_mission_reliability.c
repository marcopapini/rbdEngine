/*
 *  Component: nhpp_mission_reliability.c
 *  Compute the Mission Reliability curve of several NHPP distributions
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


#include "nhpp_mission_reliability.h"

#include "rbddata.h"

#include <math.h>
#include <stdlib.h>


static double goelOkumotoNhpp(const double x, const double test, const double a, const double b, const double eta);
static double yamadaSShapedNhpp(const double x, const double test, const double a, const double b, const double eta);
static double musaOkumotoNhpp(const double x, const double test, const double lambda, const double theta, const double eta);
static double ohbaSShapedNhpp(const double x, const double test, const double a, const double b, const double phi, const double eta);
static double goelGeneralizedNhpp(const double x, const double test, const double a, const double b, const double c, const double eta);
static double kapurGarg3Nhpp(const double x, const double test, const double a, const double b, const double eta);
static double phamZhangNhpp(const double x, const double test, const double a, const double b, const double alpha, const double beta, const double eta);


/**
 * goelOkumotoMissionReliability
 *
 * Compute the Mission Reliability curve using the Goel-Okumoto NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability curve of the Goel-Okumoto NHPP distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the mission reliability curve
 *      component: updated with the computed mission reliability curve
 *
 * Return (int):
 *  0 if the Mission Reliability curve has been computed successfully, < 0 otherwise
 */
int goelOkumotoMissionReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /**
     * Compute mission reliability curve using Goel-Okumoto NHPP distribution
     * Mission reliability starts from time equal to the NHPP distribution offset
     */
    x = component->params.nhpp_go.offset;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = goelOkumotoNhpp(x, component->params.nhpp_go.test,
                                                      component->params.nhpp_go.a,
                                                      component->params.nhpp_go.b,
                                                      component->params.nhpp_go.eta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Goel-Okumoto NHPP distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * yamadaSShapedMissionReliability
 *
 * Compute the Mission Reliability curve using the Yamada S-Shaped NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability curve of the Yamada S-Shaped NHPP distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the mission reliability curve
 *      component: updated with the computed mission reliability curve
 *
 * Return (int):
 *  0 if the Mission Reliability curve has been computed successfully, < 0 otherwise
 */
int yamadaSShapedMissionReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /**
     * Compute mission reliability curve using Yamada S-Shaped NHPP distribution
     * Mission reliability starts from time equal to the NHPP distribution offset
     */
    x = component->params.nhpp_yss.offset;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = yamadaSShapedNhpp(x, component->params.nhpp_yss.test,
                                                        component->params.nhpp_yss.a,
                                                        component->params.nhpp_yss.b,
                                                        component->params.nhpp_yss.eta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Yamada S-Shaped NHPP distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * musaOkumotoMissionReliability
 *
 * Compute the Mission Reliability curve using the Musa-Okumoto NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability curve of the Musa-Okumoto NHPP distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the mission reliability curve
 *      component: updated with the computed mission reliability curve
 *
 * Return (int):
 *  0 if the Mission Reliability curve has been computed successfully, < 0 otherwise
 */
int musaOkumotoMissionReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /**
     * Compute mission reliability curve using Musa-Okumoto NHPP distribution
     * Mission reliability starts from time equal to the NHPP distribution offset
     */
    x = component->params.nhpp_mo.offset;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = musaOkumotoNhpp(x, component->params.nhpp_mo.test,
                                                      component->params.nhpp_mo.lambda,
                                                      component->params.nhpp_mo.theta,
                                                      component->params.nhpp_mo.eta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Musa-Okumoto NHPP distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * ohbaSShapedMissionReliability
 *
 * Compute the Mission Reliability curve using the Ohba S-Shaped NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability curve of the Ohba S-Shaped NHPP distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the mission reliability curve
 *      component: updated with the computed mission reliability curve
 *
 * Return (int):
 *  0 if the Mission Reliability curve has been computed successfully, < 0 otherwise
 */
int ohbaSShapedMissionReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /**
     * Compute mission reliability curve using Ohba S-Shaped NHPP distribution
     * Mission reliability starts from time equal to the NHPP distribution offset
     */
    x = component->params.nhpp_oss.offset;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = ohbaSShapedNhpp(x, component->params.nhpp_oss.test,
                                                      component->params.nhpp_oss.a,
                                                      component->params.nhpp_oss.b,
                                                      component->params.nhpp_oss.phi,
                                                      component->params.nhpp_oss.eta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Ohba S-Shaped NHPP distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * goelGeneralizedMissionReliability
 *
 * Compute the Mission Reliability curve using the Goel Generalized NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability curve of the Goel Generalized NHPP distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the mission reliability curve
 *      component: updated with the computed mission reliability curve
 *
 * Return (int):
 *  0 if the Mission Reliability curve has been computed successfully, < 0 otherwise
 */
int goelGeneralizedMissionReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /**
     * Compute mission reliability curve using Goel Generalized NHPP distribution
     * Mission reliability starts from time equal to the NHPP distribution offset
     */
    x = component->params.nhpp_gg.offset;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = goelGeneralizedNhpp(x, component->params.nhpp_gg.test,
                                                          component->params.nhpp_gg.a,
                                                          component->params.nhpp_gg.b,
                                                          component->params.nhpp_gg.c,
                                                          component->params.nhpp_gg.eta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Goel Generalized NHPP distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * kapurGarg3MissionReliability
 *
 * Compute the Mission Reliability curve using the Kapur-Garg 3-Stage NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability curve of the Kapur-Garg 3-Stage NHPP distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the mission reliability curve
 *      component: updated with the computed mission reliability curve
 *
 * Return (int):
 *  0 if the Mission Reliability curve has been computed successfully, < 0 otherwise
 */
int kapurGarg3MissionReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /**
     * Compute mission reliability curve using Kapur-Garg 3-Stage NHPP distribution
     * Mission reliability starts from time equal to the NHPP distribution offset
     */
    x = component->params.nhpp_kg3.offset;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = kapurGarg3Nhpp(x, component->params.nhpp_kg3.test,
                                                     component->params.nhpp_kg3.a,
                                                     component->params.nhpp_kg3.b,
                                                     component->params.nhpp_kg3.eta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Kapur-Garg 3-Stage NHPP distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * phamZhangMissionReliability
 *
 * Compute the Mission Reliability curve using the Pham-Zhang NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability curve of the Pham-Zhang NHPP distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the mission reliability curve
 *      component: updated with the computed mission reliability curve
 *
 * Return (int):
 *  0 if the Mission Reliability curve has been computed successfully, < 0 otherwise
 */
int phamZhangMissionReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /**
     * Compute mission reliability curve using Pham-Zhang NHPP distribution
     * Mission reliability starts from time equal to the NHPP distribution offset
     */
    x = component->params.nhpp_pz.offset;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = phamZhangNhpp(x, component->params.nhpp_pz.test,
                                                      component->params.nhpp_pz.a,
                                                      component->params.nhpp_pz.b,
                                                      component->params.nhpp_pz.alpha,
                                                      component->params.nhpp_pz.beta,
                                                      component->params.nhpp_pz.eta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Pham-Zhang NHPP distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}


/**
 * goelOkumotoNhpp
 *
 * Compute the Mission Reliability of the Goel-Okumoto NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability of the Goel-Okumoto
 *  NHPP distribution, with parameters a, b and test, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the Mission Reliability is evaluated
 *      test: time for which the software has been tested (>= 0.0)
 *      a: a parameter of the Goel-Okumoto NHPP distribution (> 0.0)
 *      b: b parameter of the Goel-Okumoto NHPP distribution (> 0.0)
 *      eta: eta parameter (accelerated software ageing) (>= 1.0)
 *
 * Return (double):
 *  The evaluated Mission Reliability at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double goelOkumotoNhpp(const double x, const double test, const double a, const double b, const double eta) {
    double mr;

    if ((test < 0.0) || (a <= 0.0) || (b <= 0.0) || (eta < 1.0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 1.0;
    }
    mr = pow(exp(a * exp(-b * test) * (expm1(-b * x))), eta);
    return fmin(1.0, fmax(0.0, mr));
}

/**
 * yamadaSShapedNhpp
 *
 * Compute the Mission Reliability of the Yamada S-Shaped NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability of the Yamada S-Shaped
 *  NHPP distribution, with parameters a, b and test, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the Mission Reliability is evaluated
 *      test: time for which the software has been tested (>= 0.0)
 *      a: a parameter of the Yamada S-Shaped NHPP distribution (> 0.0)
 *      b: b parameter of the Yamada S-Shaped NHPP distribution (> 0.0)
 *      eta: eta parameter (accelerated software ageing) (>= 1.0)
 *
 * Return (double):
 *  The evaluated Mission Reliability at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double yamadaSShapedNhpp(const double x, const double test, const double a, const double b, const double eta) {
    double mr;

    if ((test < 0.0) || (a <= 0.0) || (b <= 0.0) || (eta < 1.0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 1.0;
    }
    mr = pow(exp(-a * exp(-b * test) * ((1.0 + b * test) - (1.0 + b * (test + x)) * exp(-b * x))), eta);
    return fmin(1.0, fmax(0.0, mr));
}

/**
 * musaOkumotoNhpp
 *
 * Compute the Mission Reliability of the Musa-Okumoto NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability of the Musa-Okumoto
 *  NHPP distribution, with parameters lambda, theta and test, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the Mission Reliability is evaluated
 *      test: time for which the software has been tested (>= 0.0)
 *      lambda: theta parameter of the Musa-Okumoto NHPP distribution (> 0.0)
 *      theta: theta parameter of the Musa-Okumoto NHPP distribution (> 0.0)
 *      eta: eta parameter (accelerated software ageing) (>= 1.0)
 *
 * Return (double):
 *  The evaluated Mission Reliability at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double musaOkumotoNhpp(const double x, const double test, const double lambda, const double theta, const double eta) {
    double mr;
    double num;
    double den;

    if ((test < 0.0) || (lambda <= 0.0) || (theta <= 0.0) || (eta < 1.0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 1.0;
    }
    if (theta < 1e-9) {
        return exp(-lambda * x);
    }
    num = 1.0 + theta * lambda * test;
    den = 1.0 + theta * lambda * (test + x);
    mr = pow(pow(num / den, 1.0 / theta), eta);
    return fmin(1.0, fmax(0.0, mr));
}

/**
 * ohbaSShapedNhpp
 *
 * Compute the Mission Reliability of the Ohba S-Shaped NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability of the Ohba S-Shaped
 *  NHPP distribution, with parameters a, b, phi and test, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the Mission Reliability is evaluated
 *      test: time for which the software has been tested (>= 0.0)
 *      a: a parameter of the Ohba S-Shaped NHPP distribution (> 0.0)
 *      b: b parameter of the Ohba S-Shaped NHPP distribution (> 0.0)
 *      phi: phi parameter of the Ohba S-Shaped NHPP distribution (> 0.0)
 *      eta: eta parameter (accelerated software ageing) (>= 1.0)
 *
 * Return (double):
 *  The evaluated Mission Reliability at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double ohbaSShapedNhpp(const double x, const double test, const double a, const double b, const double phi, const double eta) {
    double mr;
    double mbx;
    double exp_mbT;
    double num;
    double den;

    if ((test < 0.0) || (a <= 0.0) || (b <= 0.0) || (phi < 0.0) || (eta < 1.0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 1.0;
    }
    mbx = -b * x;
    exp_mbT = exp(-b * test);
    num = a * expm1(mbx) * (1.0 + phi) * exp_mbT;
    den = (1.0 + phi * exp_mbT) * (1.0 + phi * exp_mbT * exp(mbx));
    mr = pow(exp(num / den), eta);
    return fmin(1.0, fmax(0.0, mr));
}

/**
 * goelGeneralizedNhpp
 *
 * Compute the Mission Reliability of the Goel Generalized NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability of the Goel Generalized
 *  NHPP distribution, with parameters a, b, c and test, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the Mission Reliability is evaluated
 *      test: time for which the software has been tested (>= 0.0)
 *      a: a parameter of the Goel Generalized NHPP distribution (> 0.0)
 *      b: b parameter of the Goel Generalized NHPP distribution (> 0.0)
 *      c: c parameter of the Goel Generalized NHPP distribution (> 0.0)
 *      eta: eta parameter (accelerated software ageing) (>= 1.0)
 *
 * Return (double):
 *  The evaluated Mission Reliability at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double goelGeneralizedNhpp(const double x, const double test, const double a, const double b, const double c, const double eta) {
    double mr;
    double Tc;
    double diff_args;

    if ((test < 0.0) || (a <= 0.0) || (b <= 0.0) || (c <= 0.0) || (eta < 1.0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 1.0;
    }
    Tc = pow(test, c);
    diff_args = -b * (pow(test + x, c) - Tc);
    mr = pow(exp(a * exp(-b * Tc) * expm1(diff_args)), eta);
    return fmin(1.0, fmax(0.0, mr));
}

/**
 * kapurGarg3Nhpp
 *
 * Compute the Mission Reliability of the Kapur-Garg 3-Stage NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability of the Kapur-Garg 3-Stage
 *  NHPP distribution, with parameters a, b, c and test, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the Mission Reliability is evaluated
 *      test: time for which the software has been tested (>= 0.0)
 *      a: a parameter of the Kapur-Garg 3-Stage NHPP distribution (> 0.0)
 *      b: b parameter of the Kapur-Garg 3-Stage NHPP distribution (> 0.0)
 *      eta: eta parameter (accelerated software ageing) (>= 1.0)
 *
 * Return (double):
 *  The evaluated Mission Reliability at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double kapurGarg3Nhpp(const double x, const double test, const double a, const double b, const double eta) {
    double mr;
    double bx;
    double bT;
    double bTx;
    double term_exp;
    double term_poly;

    if ((test < 0.0) || (a <= 0.0) || (b <= 0.0) || (eta < 1.0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 1.0;
    }
    bT = b * test;
    bx = b * x;
    bTx = bT + bx;
    term_exp = (1.0 + bTx * (1.0 + 0.5 * bTx)) * expm1(-b * x);
    term_poly = bx * (1.0 + bT + 0.5 * bx);
    mr = pow(exp(a * exp(-bT) * (term_exp + term_poly)), eta);
    return fmin(1.0, fmax(0.0, mr));
}

/**
 * phamZhangNhpp
 *
 * Compute the Mission Reliability of the Pham-Zhang NHPP distribution
 *
 * Description:
 *  This function computes the Mission Reliability of the Pham-Zhang
 *  NHPP distribution, with parameters a, b, c and test, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the Mission Reliability is evaluated
 *      test: time for which the software has been tested (>= 0.0)
 *      a: a parameter of the Pham-Zhang NHPP distribution (> 0.0)
 *      b: b parameter of the Pham-Zhang NHPP distribution (> 0.0)
 *      alpha: alpha parameter of the Pham-Zhang NHPP distribution (>= 0.0)
 *      beta: beta parameter of the Pham-Zhang NHPP distribution (>= 0.0 and < 1.0)
 *      eta: eta parameter (accelerated software ageing) (>= 1.0)
 *
 * Return (double):
 *  The evaluated Mission Reliability at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double phamZhangNhpp(const double x, const double test, const double a, const double b, const double alpha, const double beta, const double eta) {
    double mr;
    double mbx;
    double exp_mbT;
    double num_rat;
    double den_rat;

    if ((test < 0.0) || (a <= 0.0) || (b <= 0.0) || (alpha < 0.0) || (beta < 0.0 || beta >= 1.0) || (eta < 1.0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 1.0;
    }
    mbx = -b * x;
    exp_mbT = exp(-b * test);
    num_rat = a * exp_mbT * expm1(mbx) * (beta - 1.0);
    den_rat = 1.0 - beta - beta * expm1(-b * test);
    den_rat = den_rat * (den_rat + beta * exp_mbT * (-expm1(mbx)));
    mr = pow(exp(-((num_rat / den_rat) + (alpha * x))), eta);
    return fmin(1.0, fmax(0.0, mr));
}

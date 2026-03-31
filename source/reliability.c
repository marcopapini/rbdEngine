/*
 *  Component: reliability.c
 *  Compute the Reliability curve of several distributions
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


#include "reliability.h"

#include "gamma_p.h"
#include "rbddata.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TMP_STRING_SIZE             1000


static char tmpString[TMP_STRING_SIZE];


static int checkCustomHeader(const FILE *const pFile, const char *const fname, const struct time *const time);
static int extractCustomReliability(const FILE *const pFile, const char *const fname, const struct time *const time, struct component *const component);
static double exponentialCdf(const double x, const double lambda);
static double lognormalCdf(const double x, const double mu, const double sigma);
static double normalCdf(const double x, const double mu, const double sigma);
static double weibullCdf(const double x, const double lambda, const double k);
static double birnbaumSaundersCdf(const double x, const double alpha, const double beta);


/**
 * customReliability
 *
 * Extract the Reliability curve from a Custom distribution file
 *
 * Description:
 *  This function computes the Reliability curve of a Custom distribution.
 *
 * Parameters:
 *      time: time parameters used to extract the reliability curve
 *      component: updated with the extracted reliability curve
 *
 * Return (int):
 *  0 if the Reliability curve has been extracted successfully, < 0 otherwise
 */
int customReliability(const struct time *const time, struct component *const component) {
    FILE *pFile;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /* Open file containing Custom distribution */
    pFile = fopen(component->params.rel_c.filename, "r");
    if (pFile == NULL) {
        fprintf(stderr, "Unable to open file \'%s\' with Custom distribution\n", component->params.rel_c.filename);
        return -1;
    }

    /* Check Custom distribution file header */
    if (checkCustomHeader(pFile, component->params.rel_c.filename, time) < 0) {
        fclose(pFile);
        return -1;
    }

    /* Extract the reliability curve from Custom distribution file */
    if (extractCustomReliability(pFile, component->params.rel_c.filename, time, component) < 0) {
        fclose(pFile);
        return -1;
    }

    /* Close Custom distribution file */
    fclose(pFile);

    return 0;
}

/**
 * exponentialReliability
 *
 * Compute the Reliability curve using the Exponential distribution
 *
 * Description:
 *  This function computes the Reliability curve of the Exponential distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the reliability curve
 *      component: updated with the computed reliability curve
 *
 * Return (int):
 *  0 if the Reliability curve has been computed successfully, < 0 otherwise
 */
int exponentialReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /* Compute reliability curve using Exponential distribution */
    x = time->start;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = 1.0 - exponentialCdf(x, component->params.rel_e.lambda);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Exponential distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * lognormalReliability
 *
 * Compute the Reliability curve using the Log-normal distribution
 *
 * Description:
 *  This function computes the Reliability curve of the Log-normal distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the reliability curve
 *      component: updated with the computed reliability curve
 *
 * Return (int):
 *  0 if the Reliability curve has been computed successfully, < 0 otherwise
 */
int lognormalReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /* Compute reliability curve using Log-normal distribution */
    x = time->start;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = 1.0 - lognormalCdf(x, component->params.rel_l.mu, component->params.rel_l.sigma);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Log-normal distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * normalReliability
 *
 * Compute the Reliability curve using the Normal distribution
 *
 * Description:
 *  This function computes the Reliability curve of the Normal distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the reliability curve
 *      component: updated with the computed reliability curve
 *
 * Return (int):
 *  0 if the Reliability curve has been computed successfully, < 0 otherwise
 */
int normalReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /* Compute reliability curve using Normal distribution */
    x = time->start;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = 1.0 - normalCdf(x, component->params.rel_n.mu, component->params.rel_n.sigma);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Normal distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * weibullReliability
 *
 * Compute the Reliability curve using the Weibull distribution
 *
 * Description:
 *  This function computes the Reliability curve of the Weibull distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the reliability curve
 *      component: updated with the computed reliability curve
 *
 * Return (int):
 *  0 if the Reliability curve has been computed successfully, < 0 otherwise
 */
int weibullReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /* Compute reliability curve using Weibull distribution */
    x = time->start;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = 1.0 - weibullCdf(x, component->params.rel_w.lambda, component->params.rel_w.k);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Weibull distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * gammaReliability
 *
 * Compute the Reliability curve using the Gamma distribution
 *
 * Description:
 *  This function computes the Reliability curve of the Gamma distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the reliability curve
 *      component: updated with the computed reliability curve
 *
 * Return (int):
 *  0 if the Reliability curve has been computed successfully, < 0 otherwise
 */
int gammaReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /* Compute reliability curve using Gamma distribution */
    x = time->start;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = 1.0 - gamma_p(component->params.rel_g.alpha, x / component->params.rel_g.theta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Gamma distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * birnbaumSaundersReliability
 *
 * Compute the Reliability curve using the Birnbaum-Saunders distribution
 *
 * Description:
 *  This function computes the Reliability curve of the Birnbaum-Saunders distribution.
 *
 * Parameters:
 *      time: time parameters used to compute the reliability curve
 *      component: updated with the computed reliability curve
 *
 * Return (int):
 *  0 if the Reliability curve has been computed successfully, < 0 otherwise
 */
int birnbaumSaundersReliability(const struct time *const time, struct component *const component) {
    unsigned int idx;
    double x;

    /* Allocate memory for reliability curve */
    component->reliability = (double *)malloc(time->numTimes * sizeof(double));
    if (component->reliability == NULL) {
        fprintf(stderr, "Unable to allocate memory for RELIABILITY array\n");
        return -1;
    }

    /* Compute reliability curve using Birnbaum-Saunders distribution */
    x = time->start;
    for (idx = 0; idx < time->numTimes; ++idx) {
        component->reliability[idx] = 1.0 - birnbaumSaundersCdf(x, component->params.rel_bs.alpha, component->params.rel_bs.beta);
        if (component->reliability[idx] < 0.0) {
            fprintf(stderr, "Unable to compute reliability with Birnbaum-Saunders distribution\n");
            return -1;
        }
        x += time->step;
    }

    return 0;
}

/**
 * checkCustomHeader
 *
 * Check the header of a Custom distribution file
 *
 * Description:
 *  This function checks for the correctness (well-formedness) of the provided Custom
 *  distribution file header.
 *
 * Parameters:
 *      pFile: Custom distribution file
 *      fname: name of Custom distribution file
 *      time: time parameters used to check the Custom distribution file header
 *
 * Return (int):
 * 0 if the Custom distribution file header is well-formed, -1 otherwise
 */
static int checkCustomHeader(const FILE *const pFile, const char *const fname, const struct time *const time) {
    char *str;
    double start;
    double end;
    double step;

    /* Check validity of input file header. Reliability Curve file header starts as follows:
     *
     * "From: <START>
     * To: <END>
     * Step: <STEP>
     * Values: "
     *
     * Where <START> is the start time of reliability curve, <END> is the end time
     * of reliability curve and <STEP> is the time discretization.
     */
    if (fgets(tmpString, TMP_STRING_SIZE, (FILE *)pFile) == NULL) {
        fprintf(stderr, "Empty input file %s\n", fname);
        return -1;
    }
    if (strstr(tmpString, "From: ") == NULL) {
        fprintf(stderr, "Invalid Custom reliability file %s\n", fname);
        return -1;
    }
    str = strstr(tmpString, "From: ") + strlen("From: ");
    start = strtod(str, NULL);
    if (fgets(tmpString, TMP_STRING_SIZE, (FILE *)pFile) == NULL) {
        fprintf(stderr, "Invalid Custom reliability file %s\n", fname);
        return -1;
    }
    if (strstr(tmpString, "To: ") == NULL) {
        fprintf(stderr, "Invalid Custom reliability file %s\n", fname);
        return -1;
    }
    str = strstr(tmpString, "To: ") + strlen("To: ");
    end = strtod(str, NULL);
    if (fgets(tmpString, TMP_STRING_SIZE, (FILE *)pFile) == NULL) {
        fprintf(stderr, "Invalid Custom reliability file %s\n", fname);
        return -1;
    }
    if (strstr(tmpString, "Step: ") == NULL) {
        fprintf(stderr, "Invalid Custom reliability file %s\n", fname);
        return -1;
    }
    str = strstr(tmpString, "Step: ") + strlen("Step: ");
    step = strtod(str, NULL);
    if (fgets(tmpString, TMP_STRING_SIZE, (FILE *)pFile) == NULL) {
        fprintf(stderr, "Invalid Custom reliability file %s\n", fname);
        return -1;
    }
    if (strstr(tmpString, "Values: ") == NULL) {
        fprintf(stderr, "Invalid Custom reliability file %s\n", fname);
        return -1;
    }

    /* Check if the retrieved parameters <START>, <END> and <STEP> match with
     * the expected ones. */
    if ((end - start) != (time->end - time->start)) {
        fprintf(stderr, "Time span (end - start) in file %s differs from the expected one: %f vs %f\n", fname, (end - start), (time->end - time->start));
        return -1;
    }
    if (step != time->step) {
        fprintf(stderr, "Time step in file %s differs from the expected one: %f vs %f\n", fname, step, time->step);
        return -1;
    }

    return 0;
}

/**
 * extractCustomReliability
 *
 * Extract the Reliability curve from a Custom distribution file
 *
 * Description:
 *  This function extracts the reliability curve from the provided Custom
 *  distribution file and it stores it into the provided component.
 *
 * Parameters:
 *      pFile: Custom distribution file
 *      fname: name of Custom distribution file
 *      time: time parameters used to extract the reliability curve
 *      component: updated with the extracted reliability curve
 *
 * Return (int):
 * 0 if the reliability curve has been extracted successfully, -1 otherwise
 */
static int extractCustomReliability(const FILE *const pFile, const char *const fname, const struct time *const time, struct component *const component) {
    int count;
    double reliability;
    char *str;
    char *endptr;

    count = 0;
    /* Read remaining input file */
    while (1) {
        /* Retrieve one line */
        str = fgets(tmpString, TMP_STRING_SIZE, (FILE *)pFile);
        /* Check if End of File has been reached */
        if (feof((FILE *)pFile)) {
            break;
        }
        /* An error occurred while parsing the file */
        if ((ferror((FILE *)pFile) != 0) || (str == NULL)) {
            fprintf(stderr, "Unable to correctly extract reliability curve from input file %s\n", fname);
            return -1;
        }
        /* Empty line, skip */
        if ((tmpString[0] == '\n') || (tmpString[0] == '\r')) {
            continue;
        }
        /* Check for array out of bound on reliability curve */
        if (count >= time->numTimes) {
            fprintf(stderr, "Reliability curve from input file %s contains extra (unexpected) lines\n", fname);
            return -1;
        }
        /* Check for valid line */
        str = strtok(tmpString, "\n\r");
        reliability = strtod(str, &endptr);
        if ((endptr == str) || (*endptr != '\0')) {
            fprintf(stderr, "Unable to correctly extract reliability curve from input file %s\n", fname);
            return -1;
        }
        if ((reliability < 0.0) || (reliability > 1.0)) {
            fprintf(stderr, "Invalid reliability value for reliability curve in file %s\n", fname);
            return -1;
        }
        /* Store current reliability value into reliability curve */
        component->reliability[count] = reliability;
        /* Line is valid, increment number of valid points over reliability curve */
        ++count;
    }
    /* Check for wrong size of reliability curve */
    if (count != time->numTimes) {
        fprintf(stderr, "Reliability curve from input file %s contains less points than expected\n", fname);
        return -1;
    }

    return 0;
}

/**
 * exponentialCdf
 *
 * Compute the CDF of the Exponential distribution
 *
 * Description:
 *  This function computes the CDF of the Exponential distribution, with parameter lambda,
 *  evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the CDF is evaluated
 *      lambda: lambda parameter of the Exponential distribution (> 0.0)
 *
 * Return (double):
 *  The evaluated CDF at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double exponentialCdf(const double x, const double lambda) {
    double cdf;

    if (lambda <= 0.0) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 0.0;
    }
    cdf = -expm1(-lambda * x);
    return fmax(fmin(1.0, cdf), 0.0);
}

/**
 * lognormalCdf
 *
 * Compute the CDF of the Log-normal distribution
 *
 * Description:
 *  This function computes the CDF of the Log-normal distribution, with parameters mu
 *  and sigma, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the CDF is evaluated
 *      mu: mu parameter of the Log-normal distribution
 *      sigma: sigma parameter of the Log-normal distribution (> 0.0)
 *
 * Return (double):
 *  The evaluated CDF at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double lognormalCdf(const double x, const double mu, const double sigma) {
    double cdf;

    if (sigma <= 0.0) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 0.0;
    }
    cdf = 0.5 * erfc(-((log(x) - mu) / sigma) * M_SQRT1_2);
    return fmax(fmin(1.0, cdf), 0.0);
}

/**
 * normalCdf
 *
 * Compute the CDF of the Normal distribution
 *
 * Description:
 *  This function computes the CDF of the Normal distribution, with parameters mu
 *  and sigma, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the CDF is evaluated
 *      mu: mu parameter of the Normal distribution
 *      sigma: sigma parameter of the Normal distribution (> 0.0)
 *
 * Return (double):
 *  The evaluated CDF at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double normalCdf(const double x, const double mu, const double sigma) {
    double cdf;

    if (sigma <= 0.0) {
        return -1.0;
    }
    cdf = 0.5 * erfc(-((x - mu) / sigma) * M_SQRT1_2);
    return fmax(fmin(1.0, cdf), 0.0);
}

/**
 * weibullCdf
 *
 * Compute the CDF of the Weibull distribution
 *
 * Description:
 *  This function computes the CDF of the Weibull distribution, with parameters lambda
 *  and k, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the CDF is evaluated
 *      lambda: lambda parameter of the Weibull distribution (> 0.0)
 *      k: k parameter of the Weibull distribution (> 0.0)
 *
 * Return (double):
 *  The evaluated CDF at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double weibullCdf(const double x, const double lambda, const double k) {
    double cdf;

    if ((lambda <= 0.0) || (k <= 0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 0.0;
    }
    cdf = -expm1(0.0 - pow(x / lambda, k));
    return fmax(fmin(1.0, cdf), 0.0);
}

/**
 * birnbaumSaundersCdf
 *
 * Compute the CDF of the Birnbaum-Saunders distribution
 *
 * Description:
 *  This function computes the CDF of the Birnbaum-Saunders distribution, with parameters alpha
 *  and beta, evaluated at x.
 *
 * Parameters:
 *      x: value (abscissa) at which the CDF is evaluated
 *      alpha: alpha parameter of the Birnbaum-Saunders distribution (> 0.0)
 *      beta: beta parameter of the Birnbaum-Saunders distribution (> 0.0)
 *
 * Return (double):
 *  The evaluated CDF at x (in [0.0, 1.0]) if successful, -1.0 otherwise
 */
static double birnbaumSaundersCdf(const double x, const double alpha, const double beta) {
    if ((alpha <= 0.0) || (beta <= 0.0)) {
        return -1.0;
    }
    if (x <= 0.0) {
        return 0.0;
    }
    return normalCdf((1.0 / alpha) * (sqrt(x / beta) - sqrt(beta / x)), 0.0, 1.0);
}

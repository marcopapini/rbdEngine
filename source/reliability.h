/*
 *  Component: reliability.h
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

#ifndef RELIABILITY_H
#define RELIABILITY_H


#include "rbddata.h"


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
int customReliability(const struct time *const time, struct component *const component);

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
int exponentialReliability(const struct time *const time, struct component *const component);

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
int lognormalReliability(const struct time *const time, struct component *const component);

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
int normalReliability(const struct time *const time, struct component *const component);

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
int weibullReliability(const struct time *const time, struct component *const component);

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
int gammaReliability(const struct time *const time, struct component *const component);

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
int birnbaumSaundersReliability(const struct time *const time, struct component *const component);


#endif /* RELIABILITY_H */

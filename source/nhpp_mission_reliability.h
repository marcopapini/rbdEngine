/*
 *  Component: nhpp_mission_reliability.h
 *  Compute the Mission Reliability curve of several Non-Homogeneous
 *  Poisson Process (NHPP) distributions
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


#ifndef NHPP_MISSION_RELIABILITY_H
#define NHPP_MISSION_RELIABILITY_H


#include "rbddata.h"


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
int goelOkumotoMissionReliability(const struct time *const time, struct component *const component);

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
int yamadaSShapedMissionReliability(const struct time *const time, struct component *const component);

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
int musaOkumotoMissionReliability(const struct time *const time, struct component *const component);

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
int ohbaSShapedMissionReliability(const struct time *const time, struct component *const component);

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
int goelGeneralizedMissionReliability(const struct time *const time, struct component *const component);

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
int kapurGarg3MissionReliability(const struct time *const time, struct component *const component);

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
int phamZhangMissionReliability(const struct time *const time, struct component *const component);


#endif /* NHPP_MISSION_RELIABILITY_H */

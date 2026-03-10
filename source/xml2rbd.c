/*
 *  Component: xml2rbd.c
 *  Convert the provided RBD Description Language file into an RBD Data Structure
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


#include "xml2rbd.h"

#include "rbddata.h"
#include "reliability.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/globals.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


#define IS_VALID_ATTR(attrName, attrVal)    do { \
                                                if ((attrVal) == NULL) { \
                                                    fprintf(stderr, "Unable to allocate memory for \"%s\" attribute\n", attrName); \
                                                    return -1; \
                                                } \
                                            } while (0);


static int processTimeElement(const xmlNode *const timeNode, struct time *const time);
static unsigned char getNumComponents(const xmlNode *const componentsNode);
static int processComponentElement(const xmlNode *const componentNode, struct component *const component);
static int processComponentsElement(const xmlNode *const componentsNode, struct rbd *const rbd);
static unsigned int getNumBlocks(const xmlNode *const blocksNode);
static unsigned char getNumInputs(const xmlNode *const innerBlockNode);
static int processBlockElement(const xmlNode *const blockNode, struct block *const block);
static int processBlocksElement(const xmlNode *const blocksNode, struct rbd *const rbd);
static int checkNames(const struct rbd * const rbd);
static int computeComponentReliability(struct rbd * const rbd);
static int checkBlocks(struct rbd * const rbd);


/**
 * xml2rbd
 *
 * Convert the XML RBD Description Language document into the RBD Data Structure
 *
 * Description:
 *  This function reads the provided XML RBD Description Language document and it
 *  converts it into the provided RBD Data Structure
 *
 * Parameters:
 *      document: pointer to the XML RBD Description Language document
 *      rbd: pointer to the filled RBD Data Structure
 *
 * Return (int):
 *  0 if the conversion into the RBD Data Structure is successful, < 0 otherwise
 */
int xml2rbd(const xmlDoc *const doc, struct rbd *const rbd) {
    const xmlNode *root;
    const xmlNode *node;

    /* Retrieve XML root element <rbd> */
    root = xmlDocGetRootElement(doc);

    /* Retrieve first child, <time> element, and process it */
    node = root->children;
    while (node->type != XML_ELEMENT_NODE) {
        node = node->next;
    }
    if (processTimeElement(node, &rbd->time) < 0) {
        return -1;
    }

    /* Retrieve second child, <components> element, and process it */
    node = node->next;
    while (node->type != XML_ELEMENT_NODE) {
        node = node->next;
    }
    rbd->numComponents = getNumComponents(node);
    rbd->components = (struct component *)calloc(rbd->numComponents, sizeof(struct component));
    if (rbd->components == NULL) {
        fprintf(stderr, "Unable to allocate memory for COMPONENTS array\n");
        return -1;
    }
    if (processComponentsElement(node, rbd) < 0) {
        return -1;
    }

    /* Retrieve third child, <blocks> element, and process it */
    node = node->next;
    while (node->type != XML_ELEMENT_NODE) {
        node = node->next;
    }
    rbd->numBlocks = getNumBlocks(node);
    rbd->blocks = (struct block *)calloc(rbd->numBlocks, sizeof(struct block));
    if (rbd->blocks == NULL) {
        fprintf(stderr, "Unable to allocate memory for BLOCKS array\n");
        return -1;
    }
    if (processBlocksElement(node, rbd) < 0) {
        return -1;
    }

    /* Check if names of components and blocks are valid */
    if (checkNames(rbd) < 0) {
        return -1;
    }

    /* Check if the block are valid */
    if (checkBlocks(rbd) < 0) {
        return -1;
    }

    /* Compute the reliability curves of all components */
    if (computeComponentReliability(rbd) < 0) {
        return -1;
    }

    return 0;
}


/**
 * processTimeElement
 *
 * Convert the XML <time> element into the RBD Time Data Structure
 *
 * Description:
 *  This function reads the provided XML <time> element and it
 *  converts it into the provided RBD Time Data Structure
 *
 * Parameters:
 *      timeNode: pointer to the XML <time> element
 *      time: pointer to the filled RBD Time Data Structure
 *
 * Return (int):
 *  0 if the conversion into the RBD Time Data Structure is successful, < 0 otherwise
 */
static int processTimeElement(const xmlNode *const timeNode, struct time *const time) {
    xmlChar *attr;

    /* Extract data from attribute start */
    attr = xmlGetProp(timeNode, BAD_CAST "start");
    IS_VALID_ATTR("rbd/time@start", attr);
    time->start = strtod((const char *)attr, NULL);
    xmlFree(attr);
    /* Extract data from attribute end */
    attr = xmlGetProp(timeNode, BAD_CAST "end");
    IS_VALID_ATTR("rbd/time@end", attr);
    time->end = strtod((const char *)attr, NULL);
    xmlFree(attr);
    /* Extract data from attribute step */
    attr = xmlGetProp(timeNode, BAD_CAST "step");
    IS_VALID_ATTR("rbd/time@step", attr);
    time->step = strtod((const char *)attr, NULL);
    xmlFree(attr);

    if (time->start > time->end) {
        fprintf(stderr, "Invalid <time> element, attribute start (%f) is greater than attribute end (%f)\n",
                time->start, time->end);
        return -1;
    }

    time->numTimes = (unsigned int)floor((time->end - time->start) / time->step) + 1;

    return 0;
}

/**
 * getNumComponents
 *
 * Retrieve the number of XML <component> elements into the provided XML <components> element
 *
 * Description:
 *  This function reads the provided XML <components> element and it
 *  returns the number of its children XML <component> elements
 *
 * Parameters:
 *      componentsNode: pointer to the XML <components> element
 *
 * Return (unsigned char):
 *  The number of XML <component> elements children of the provided XML <components> element
 */
static unsigned char getNumComponents(const xmlNode *const componentsNode) {
    unsigned char numComponents;
    const xmlNode *node;

    numComponents = 0;
    /* Count the number of <component> elements */
    node = componentsNode->children;
    while (node) {
        if (node->type == XML_ELEMENT_NODE) {
            numComponents++;
        }
        node = node->next;
    }

    return numComponents;
}

/**
 * processComponentElement
 *
 * Convert the XML <component> element into the RBD Component Data Structure
 *
 * Description:
 *  This function reads the provided XML <component> element and it
 *  converts it into the provided RBD Component Data Structure
 *
 * Parameters:
 *      componentNode: pointer to the XML <component> element
 *      component: pointer to the filled RBD Component Data Structure
 *
 * Return (int):
 *  0 if the conversion into the RBD Component Data Structure is successful, < 0 otherwise
 */
static int processComponentElement(const xmlNode *const componentNode, struct component *const component) {
    const xmlNode *node;
    xmlChar *attr;
    char *endPtr;

    component->name = (char *)xmlGetProp(componentNode, BAD_CAST "name");
    IS_VALID_ATTR("rbd/components/component@name", component->name);
    /* For each one of the <component> children */
    node = componentNode->children;
    while (node) {
        if (node->type == XML_ELEMENT_NODE) {
            if (xmlStrEqual(node->name, BAD_CAST "custom")) {
                /* Process the <custom> element */
                component->type = DST_CUSTOM;
                component->params.c.filename = (char *)xmlGetProp(node, BAD_CAST "filename");
                IS_VALID_ATTR("rbd/components/component/custom@filename", component->params.c.filename);
            }
            else if (xmlStrEqual(node->name, BAD_CAST "exponential")) {
                /* Process the <exponential> element */
                component->type = DST_EXPONENTIAL;
                attr = xmlGetProp(node, BAD_CAST "lambda");
                IS_VALID_ATTR("rbd/components/component/exponential@lambda", attr);
                component->params.e.lambda = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                component->params.e.outputFilename = (char *)xmlGetProp(node, BAD_CAST "outputFilename");
            }
            else if (xmlStrEqual(node->name, BAD_CAST "lognormal")) {
                /* Process the <lognormal> element */
                component->type = DST_LOGNORMAL;
                attr = xmlGetProp(node, BAD_CAST "mu");
                IS_VALID_ATTR("rbd/components/component/lognormal@mu", attr);
                component->params.l.mu = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                attr = xmlGetProp(node, BAD_CAST "sigma");
                IS_VALID_ATTR("rbd/components/component/lognormal@sigma", attr);
                component->params.l.sigma = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                component->params.l.outputFilename = (char *)xmlGetProp(node, BAD_CAST "outputFilename");
            }
            else if (xmlStrEqual(node->name, BAD_CAST "normal")) {
                /* Process the <normal> element */
                component->type = DST_NORMAL;
                attr = xmlGetProp(node, BAD_CAST "mu");
                IS_VALID_ATTR("rbd/components/component/normal@mu", attr);
                component->params.n.mu = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                attr = xmlGetProp(node, BAD_CAST "sigma");
                IS_VALID_ATTR("rbd/components/component/normal@sigma", attr);
                component->params.n.sigma = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                component->params.n.outputFilename = (char *)xmlGetProp(node, BAD_CAST "outputFilename");
            }
            else if (xmlStrEqual(node->name, BAD_CAST "weibull")) {
                /* Process the <weibull> element */
                component->type = DST_WEIBULL;
                attr = xmlGetProp(node, BAD_CAST "lambda");
                IS_VALID_ATTR("rbd/components/component/weibull@lambda", attr);
                component->params.w.lambda = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                attr = xmlGetProp(node, BAD_CAST "k");
                IS_VALID_ATTR("rbd/components/component/weibull@k", attr);
                component->params.w.k = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                component->params.w.outputFilename = (char *)xmlGetProp(node, BAD_CAST "outputFilename");
            }
            else if (xmlStrEqual(node->name, BAD_CAST "gamma")) {
                /* Process the <gamma> element */
                component->type = DST_GAMMA;
                attr = xmlGetProp(node, BAD_CAST "alpha");
                IS_VALID_ATTR("rbd/components/component/gamma@alpha", attr);
                component->params.g.alpha = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                attr = xmlGetProp(node, BAD_CAST "theta");
                IS_VALID_ATTR("rbd/components/component/gamma@theta", attr);
                component->params.g.theta = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                component->params.g.outputFilename = (char *)xmlGetProp(node, BAD_CAST "outputFilename");
            }
            else {
                /* Process the <birnbaum_saunders> element */
                component->type = DST_BIRNBAUM_SAUNDERS;
                attr = xmlGetProp(node, BAD_CAST "alpha");
                IS_VALID_ATTR("rbd/components/component/gamma@alpha", attr);
                component->params.bs.alpha = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                attr = xmlGetProp(node, BAD_CAST "beta");
                IS_VALID_ATTR("rbd/components/component/gamma@beta", attr);
                component->params.bs.beta = strtod((char *)attr, &endPtr);
                xmlFree(attr);
                component->params.bs.outputFilename = (char *)xmlGetProp(node, BAD_CAST "outputFilename");
            }
            break;
        }
        node = node->next;
    }

    return 0;
}

/**
 * processComponentsElement
 *
 * Convert the XML <components> element into a list of RBD Component Data Structures
 *
 * Description:
 *  This function reads the provided XML <components> element and it
 *  converts it into the list of RBD Component Data Structures added to the
 *  provided RBD Data Structure
 *
 * Parameters:
 *      componentsNode: pointer to the XML <components> element
 *      rbd: pointer to RBD Data Structure filled with list of RBD Component Data Structures
 *
 * Return (int):
 *  0 if the conversion into the list of RBD Component Data Structures is successful, < 0 otherwise
 */
static int processComponentsElement(const xmlNode *const componentsNode, struct rbd *const rbd) {
    const xmlNode *node;
    unsigned char componentIdx;

    componentIdx = 0;
    /* For each one of the <component> elements */
    node = componentsNode->children;
    while (node) {
        if (node->type == XML_ELEMENT_NODE) {
            /* Process the <component> element */
            if (processComponentElement(node, &rbd->components[componentIdx]) < 0) {
                return -1;
            }
            ++componentIdx;
        }
        node = node->next;
    }

    return 0;
}


/**
 * getNumBlocks
 *
 * Retrieve the number of XML <block> elements into the provided XML <blocks> element
 *
 * Description:
 *  This function reads the provided XML <blocks> element and it
 *  returns the number of its children XML <block> elements
 *
 * Parameters:
 *      blocksNode: pointer to the XML <blocks> element
 *
 * Return (unsigned int):
 *  The number of XML <block> elements children of the provided XML <blocks> element
 */
static unsigned int getNumBlocks(const xmlNode *const blocksNode) {
    unsigned int numBlocks;
    const xmlNode *node;

    numBlocks = 0;
    /* Count the number of <block> elements */
    node = blocksNode->children;
    while (node) {
        if (node->type == XML_ELEMENT_NODE) {
            numBlocks++;
        }
        node = node->next;
    }

    return numBlocks;
}


/**
 * getNumInputs
 *
 * Retrieve the number of XML <input> elements into the provided XML element (child of <block> element)
 *
 * Description:
 *  This function reads the provided XML element (child of <block> element) and it
 *  returns the number of its children XML <input> elements
 *
 * Parameters:
 *      innerBlockNode: pointer to the XML element (child of <block> element)
 *
 * Return (unsigned char):
 *  The number of XML <input> elements children of the provided XML element (child of <block> element)
 */
static unsigned char getNumInputs(const xmlNode *const innerBlockNode) {
    unsigned char numInputs;
    const xmlNode *node;

    numInputs = 0;
    /* Count the number of <input> elements */
    node = innerBlockNode->children;
    while (node) {
        if (node->type == XML_ELEMENT_NODE && xmlStrEqual(node->name, BAD_CAST "input")) {
            numInputs++;
        }
        node = node->next;
    }

    return numInputs;
}

/**
 * processBlockElement
 *
 * Convert the XML <block> element into a list of RBD Block Data Structures
 *
 * Description:
 *  This function reads the provided XML <block> element and it
 *  converts it into the provided RBD Block Data Structure
 *
 * Parameters:
 *      blockNode: pointer to the XML <block> element
 *      block: pointer to the filled RBD Block Data Structure
 *
 * Return (int):
 *  0 if the conversion into the list of RBD Block Data Structures is successful, < 0 otherwise
 */
static int processBlockElement(const xmlNode *const blockNode, struct block *const block) {
    const xmlNode *node;
    xmlChar *attr;
    char *endPtr;
    unsigned char inputIdx;

    /* The current block is not analyzed */
    block->bIsAnalyzed = 0;

    /* Extract data from optional attribute outputFilename */
    block->outputFilename = (char *)xmlGetProp(blockNode, BAD_CAST "outputFilename");

    /* For each one of the <block> children */
    node = blockNode->children;
    while (node) {
        if (node->type == XML_ELEMENT_NODE) {
            if (xmlStrEqual(node->name, BAD_CAST "bridgeGeneric")) {
                /* Set the proper type of block */
                block->type = BLK_BRIDGE;
                block->bIsIdentical = 0;
            }
            else if (xmlStrEqual(node->name, BAD_CAST "bridgeIdentical")) {
                /* Set the proper type of block */
                block->type = BLK_BRIDGE;
                block->bIsIdentical = 1;
                /* Set number of inputs in Bridge Identical to 5 */
                block->numInputs = 5;
            }
            else if (xmlStrEqual(node->name, BAD_CAST "koonGeneric")) {
                /* Set the proper type of block */
                block->type = BLK_KOON;
                block->bIsIdentical = 0;
                /* Extract data from attribute minInputs */
                attr = xmlGetProp(node, BAD_CAST "minInputs");
                IS_VALID_ATTR("rbd/blocks/block/koonGeneric@minInputs", attr);
                block->minInputs = (unsigned char)strtoul((const char *)attr, &endPtr, 0);
                xmlFree(attr);
            }
            else if (xmlStrEqual(node->name, BAD_CAST "koonIdentical")) {
                /* Set the proper type of block */
                block->type = BLK_KOON;
                block->bIsIdentical = 1;
                /* Extract data from attribute numInputs */
                attr = xmlGetProp(node, BAD_CAST "numInputs");
                IS_VALID_ATTR("rbd/blocks/block/koonIdentical@numInputs", attr);
                block->numInputs = (unsigned char)strtoul((const char *)attr, &endPtr, 0);
                xmlFree(attr);
                /* Extract data from attribute minInputs */
                attr = xmlGetProp(node, BAD_CAST "minInputs");
                IS_VALID_ATTR("rbd/blocks/block/koonIdentical@minInputs", attr);
                block->minInputs = (unsigned char)strtoul((const char *)attr, &endPtr, 0);
                xmlFree(attr);
            }
            else if (xmlStrEqual(node->name, BAD_CAST "parallelGeneric")) {
                /* Set the proper type of block */
                block->type = BLK_PARALLEL;
                block->bIsIdentical = 0;
            }
            else if (xmlStrEqual(node->name, BAD_CAST "parallelIdentical")) {
                /* Set the proper type of block */
                block->type = BLK_PARALLEL;
                block->bIsIdentical = 1;
                /* Extract data from attribute numInputs */
                attr = xmlGetProp(node, BAD_CAST "numInputs");
                IS_VALID_ATTR("rbd/blocks/block/parallelIdentical@numInputs", attr);
                block->numInputs = (unsigned char)strtoul((const char *)attr, &endPtr, 0);
                xmlFree(attr);
            }
            else if (xmlStrEqual(node->name, BAD_CAST "seriesGeneric")) {
                /* Set the proper type of block */
                block->type = BLK_SERIES;
                block->bIsIdentical = 0;
            }
            else {
                /* Set the proper type of block */
                block->type = BLK_SERIES;
                block->bIsIdentical = 1;
                /* Extract data from attribute numInputs */
                attr = xmlGetProp(node, BAD_CAST "numInputs");
                IS_VALID_ATTR("rbd/blocks/block/seriesIdentical@numInputs", attr);
                block->numInputs = (unsigned char)strtoul((const char *)attr, &endPtr, 0);
                xmlFree(attr);
            }
            break;
        }
        node = node->next;
    }

    if (block->bIsIdentical == 0) {
        block->numInputs = getNumInputs(node);
        block->inputs = (struct input *)calloc(block->numInputs, sizeof(struct input));
    }
    else {
        if (block->type == BLK_BRIDGE) {
            if (block->numInputs != 5) {
                fprintf(stderr, "Invalid number of INPUTS for BRIDGE block %s (expected 5, requested %u)\n",
                        block->outputName, block->numInputs);
                return -1;
            }
        }
        block->inputs = (struct input *)calloc(1, sizeof(struct input));
    }
    if (block->inputs == NULL) {
        fprintf(stderr, "Unable to allocate memory for INPUTS array\n");
        return -1;
    }

    /* For each one of the <BLOCK_TYPE> children */
    inputIdx = 0;
    node = node->children;
    while (node) {
        if (node->type == XML_ELEMENT_NODE) {
            if (xmlStrEqual(node->name, BAD_CAST "input")) {
                block->inputs[inputIdx].name = (char *)xmlGetProp(node, BAD_CAST "name");
                IS_VALID_ATTR("rbd/blocks/block/<BLOCK>/input@name", block->inputs[inputIdx].name);
                attr = xmlGetProp(node, BAD_CAST "isUnreliability");
                if (attr != NULL) {
                    if (xmlStrEqual(node->name, BAD_CAST "true") == 0) {
                        block->inputs[inputIdx].bIsUnreliability = 1;
                    }
                    else {
                        block->inputs[inputIdx].bIsUnreliability = 0;
                    }
                    xmlFree(attr);
                }
                else {
                    block->inputs[inputIdx].bIsUnreliability = 0;
                }
                ++inputIdx;
            }
            else {
                block->outputName = (char *)xmlGetProp(node, BAD_CAST "name");
                IS_VALID_ATTR("rbd/blocks/block/<BLOCK>/output@name", block->outputName);
            }
        }
        node = node->next;
    }

    return 0;
}

/**
 * processBlocksElement
 *
 * Convert the XML <blocks> element into a list of RBD Block Data Structures
 *
 * Description:
 *  This function reads the provided XML <blocks> element and it
 *  converts it into the list of RBD Block Data Structures added to the
 *  provided RBD Data Structure
 *
 * Parameters:
 *      blocksNode: pointer to the XML <blocks> element
 *      rbd: pointer to RBD Data Structure filled with list of RBD Block Data Structures
 *
 * Return (int):
 *  0 if the conversion into the list of RBD Block Data Structures is successful, < 0 otherwise
 */
static int processBlocksElement(const xmlNode *const blocksNode, struct rbd *const rbd) {
    const xmlNode *node;
    unsigned int blockIdx;

    /* Extract data from attribute system */
    rbd->systemBlock = (char *)xmlGetProp(blocksNode, BAD_CAST "system");
    IS_VALID_ATTR("rbd/blocks@system", rbd->systemBlock);

    blockIdx = 0;
    /* For each one of the <block> elements */
    node = blocksNode->children;
    while (node) {
        if (node->type == XML_ELEMENT_NODE) {
            /* Process the <block> element */
            if (processBlockElement(node, &rbd->blocks[blockIdx]) < 0) {
                return -1;
            }
            ++blockIdx;
        }
        node = node->next;
    }

    return 0;
}

/**
 * checkNames
 *
 * Check the names of all components and blocks
 *
 * Description:
 *  This function checks the names of all the components and blocks belonging
 *  to the provided RBD Data Structure. Each component and block name shall be
 *  unique, i.e., two components, two blocks, or one component and a block
 *  cannot have the same name
 *
 * Parameters:
 *      rbd: RBD Data Structure to be checked
 *
 * Return (int):
 *  0 if the names of the components and blocks are valid, < 0 otherwise
 */
static int checkNames(const struct rbd * const rbd) {
    unsigned short ii, jj;

    /* For each component... */
    for (ii = 0; ii < rbd->numComponents; ++ii) {
        /* Check that the component name is unique among both other components and blocks */
        for (jj = ii + 1; jj < rbd->numComponents; ++jj) {
            if (strcmp(rbd->components[ii].name, rbd->components[jj].name) == 0) {
                fprintf(stderr, "Same name (%s) used for components %d, %d\n", rbd->components[ii].name, ii + 1, jj + 1);
                return -1;
            }
        }
        for (jj = 0; jj < rbd->numBlocks; ++jj) {
            if (strcmp(rbd->components[ii].name, rbd->blocks[jj].outputName) == 0) {
                fprintf(stderr, "Same name (%s) used for component %d, block %d\n", rbd->components[ii].name, ii + 1, jj + 1);
                return -1;
            }
        }
    }

    /* For each block (excluding last)... */
    for (ii = 0; ii < rbd->numBlocks - 1; ++ii) {
        /* Check that each block output name is unique */
        for (jj = ii + 1; jj < rbd->numBlocks; ++jj) {
            if (strcmp(rbd->blocks[ii].outputName, rbd->blocks[jj].outputName) == 0) {
                fprintf(stderr, "Same name (%s) used for blocks %d, %d\n", rbd->blocks[ii].outputName, ii + 1, jj + 1);
                return -1;
            }
        }
    }

    return 0;
}

/**
 * computeComponentReliability
 *
 * Compute the reliability curve for each component
 *
 * Description:
 *  This function computes the reliability curve for each component
 *
 * Parameters:
 *      rbd: RBD Data Structure updated with computed reliability curves of components
 *
 * Return (int):
 *  0 if the reliability curves have been successfully computed, < 0 otherwise
 */
static int computeComponentReliability(struct rbd * const rbd) {
    unsigned char ii;

    /* Compute the reliability curve of each component */
    for (ii = 0; ii < rbd->numComponents; ++ii) {
        switch(rbd->components[ii].type) {
            case DST_CUSTOM:
                if (customReliability(&rbd->time, &rbd->components[ii]) < 0) {
                    return -1;
                }
                break;
            case DST_EXPONENTIAL:
                if (exponentialReliability(&rbd->time, &rbd->components[ii]) < 0) {
                    return -1;
                }
                break;
            case DST_LOGNORMAL:
                if (lognormalReliability(&rbd->time, &rbd->components[ii]) < 0) {
                    return -1;
                }
                break;
            case DST_NORMAL:
                if (normalReliability(&rbd->time, &rbd->components[ii]) < 0) {
                    return -1;
                }
                break;
            case DST_WEIBULL:
                if (weibullReliability(&rbd->time, &rbd->components[ii]) < 0) {
                    return -1;
                }
                break;
            case DST_GAMMA:
                if (gammaReliability(&rbd->time, &rbd->components[ii]) < 0) {
                    return -1;
                }
                break;
            case DST_BIRNBAUM_SAUNDERS:
            default:
                if (birnbaumSaundersReliability(&rbd->time, &rbd->components[ii]) < 0) {
                    return -1;
                }
                break;
        }
    }

    return 0;
}

/**
 * checkBlockNames
 *
 * Check the blocks of the RBD Data Structure
 *
 * Description:
 *  This function checks the blocks belonging to the provided RBD Data Structure.
 *  - Each block name shall be unique
 *  - Top-level block shall be the output of a block
 *  - All inputs of a block shall be valid, i.e., a valid block or component
 *
 * Parameters:
 *      rbd: RBD Data Structure to be checked
 *
 * Return (int):
 *  0 if the blocks are valid, < 0 otherwise
 */
static int checkBlocks(struct rbd * const rbd) {
    unsigned int bIdx, idx;
    unsigned int iIdx;
    unsigned char bFound;

    /* Check that the top-level block corresponds to the output of a block */
    bFound = 0;
    for (bIdx = 0; bIdx < rbd->numBlocks; ++bIdx) {
        if (strcmp(rbd->blocks[bIdx].outputName, rbd->systemBlock) == 0) {
            rbd->systemBlockIdx = bIdx;
            bFound = 1;
            break;
        }
    }
    if (bFound == 0) {
        fprintf(stderr, "Top-level block (%s) not defined in any block\n", rbd->systemBlock);
        return -1;
    }

    if (rbd->blocks[rbd->systemBlockIdx].outputFilename == NULL) {
        fprintf(stderr, "Top-level block (%s) is missing output file\n", rbd->systemBlock);
        return -1;
    }

    /* For each block... */
    for (bIdx = 0; bIdx < rbd->numBlocks; ++bIdx) {
        if (rbd->blocks[bIdx].bIsIdentical == 0) {
            /* For each input of the current block... */
            for (iIdx = 0; iIdx < rbd->blocks[bIdx].numInputs; ++iIdx) {
                bFound = 0;
                for (idx = 0; idx < rbd->numComponents; ++idx) {
                    if (strcmp(rbd->blocks[bIdx].inputs[iIdx].name, rbd->components[idx].name) == 0) {
                        rbd->blocks[bIdx].inputs[iIdx].type = INPUT_COMPONENT;
                        rbd->blocks[bIdx].inputs[iIdx].idx = idx;
                        bFound = 1;
                        break;
                    }
                }
                if (bFound == 0) {
                    for (idx = 0; idx < rbd->numBlocks; ++idx) {
                        if (strcmp(rbd->blocks[bIdx].inputs[iIdx].name, rbd->blocks[idx].outputName) == 0) {
                            rbd->blocks[bIdx].inputs[iIdx].type = INPUT_BLOCK;
                            rbd->blocks[bIdx].inputs[iIdx].idx = idx;
                            bFound = 1;
                            break;
                        }
                    }
                }
                if (bFound == 0) {
                    fprintf(stderr, "Unknown input %s in block %d\n", rbd->blocks[bIdx].inputs[iIdx].name, bIdx + 1);
                    return -1;
                }
            }
        }
        else {
            bFound = 0;
            for (idx = 0; idx < rbd->numComponents; ++idx) {
                if (strcmp(rbd->blocks[bIdx].inputs[0].name, rbd->components[idx].name) == 0) {
                    rbd->blocks[bIdx].inputs[0].type = INPUT_COMPONENT;
                    rbd->blocks[bIdx].inputs[0].idx = idx;
                    bFound = 1;
                    break;;
                }
            }
            if (bFound == 0) {
                for (idx = 0; idx < rbd->numBlocks; ++idx) {
                    if (strcmp(rbd->blocks[bIdx].inputs[0].name, rbd->blocks[idx].outputName) == 0) {
                        rbd->blocks[bIdx].inputs[0].type = INPUT_BLOCK;
                        rbd->blocks[bIdx].inputs[0].idx = idx;
                        bFound = 1;
                        break;
                    }
                }
            }
            if (bFound == 0) {
                fprintf(stderr, "Unknown input %s in block %d\n", rbd->blocks[bIdx].inputs[0].name, bIdx + 1);
                return -1;
            }
        }

        if (rbd->blocks[bIdx].outputFilename != NULL) {
            for (idx = bIdx + 1; idx < rbd->numBlocks; ++idx) {
                if (rbd->blocks[idx].outputFilename != NULL) {
                    if (strcmp(rbd->blocks[bIdx].outputFilename, rbd->blocks[idx].outputFilename) == 0) {
                        fprintf(stderr, "Blocks %s and %s refer to the same output file %s\n",
                                rbd->blocks[bIdx].outputName, rbd->blocks[idx].outputName,
                                rbd->blocks[bIdx].outputFilename);
                        return -1;
                    }
                }
            }
        }
    }

    return 0;
}

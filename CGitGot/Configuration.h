
#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "cJSON.h"

/*
* Parses the config file and returns the cJSON object hierarchy.
*/
auto GetJSONConfig(int verbose)->cJSON*;

#endif
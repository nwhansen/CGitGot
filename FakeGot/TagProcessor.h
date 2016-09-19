
#ifndef TAGPROCESSOR_H
#define TAGPROCESSOR_H

#include "Repository.h"
#include "cJSON.h"

/**
* Filters and returns all repos that match the given tag list
*/
Repository** FilterByTags(cJSON* repos, int* matchedRepositoryCount, int tagListSize, char** tagList);

Repository** AllToRepos(cJSON* repos, int* matchedRepositoryCount);

#endif
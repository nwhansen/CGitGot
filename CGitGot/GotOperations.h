#ifndef GOTOPERATIONS_H
#define GOTOPERATIONS_H

#include "Operations.h"
#include "Repository.h"

auto RunGot(got::Operations operation, int filteredRepos, Repository* repos) -> int;

#endif
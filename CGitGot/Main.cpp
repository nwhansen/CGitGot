/*
* This program is free software; you can distribute it
* and/or modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or any later version.  Accordingly, this program is
* distributed in the hope that it will be useful, but WITHOUT A
* WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details (http://www.gnu.org/copyleft/gpl.txt).
*
* Copyright Nate Hansen
*/
#include <stdlib.h>
#include <iostream>
#include "cJSON.h"
#include "opt.h"
#include "Repository.h"
#include "TagProcessor.h"
#include "Operations.h"
#include "Configuration.h"
#include "GotOperations.h"

char* baseCommand = nullptr;
int printOperations = 0;
int quiet = 0;
int verbose = 0;
int help = 0;
int tagCount = 0;
char **tags = nullptr;
got::Operations operation;

auto getRepos(cJSON* root, int* filteredRepoCount, int tagCount, char** tags) -> Repository* {
	*filteredRepoCount = 0;

	Repository* repos = nullptr;
	cJSON* repoJSON = cJSON_GetObjectItem(root, "repos");
	if (repoJSON == nullptr) {
		return nullptr;
	}
	if (tagCount > 0) {
		repos = FilterByTags(repoJSON, filteredRepoCount, tagCount, tags);
	}
	else {
		cJSON* config = cJSON_GetObjectItem(root, "configuration");
		if (config != nullptr) {
			cJSON* noTagSelectsAll = cJSON_GetObjectItem(config, "noTagSelectsAll");
			if (noTagSelectsAll != nullptr && noTagSelectsAll->type == cJSON_True) {
				repos = AllToRepos(repoJSON, filteredRepoCount);
			}
		}
	}
	return repos;
}

auto printAllOperations(void*) -> int {
	std::cout << got::STATUS << " -\t" << "Returns the status of all repositories that match the criteria" << std::endl;
	std::cout << got::PULL << "   -\t" << "Pulls on all repositories that match the criteria [can specify branch to pull with --branch argument]" << std::endl;
	return OPT_EXIT;
}

auto convertCommand(void* arg) -> int {
	//Convert void* into a char** but then convert into char*... blah
	char* command = *((char**)arg);
	if (command == nullptr) {
		optPrintUsage();
		return OPT_EXIT;
	}
	if (strcmp(got::STATUS, command) == 0) {
		operation = got::status;
	}
	else if (strcmp(got::PULL, command) == 0) {
		operation = got::pull;
	}
	else {
		std::cout << "Unrecognized Command" << std::endl;
		return OPT_EXIT;
	}
	return OPT_OK;
}

auto printSpecializedHelp(void*) -> int {
	//I didn't like that --help doesn't have a hook so we get around this by forcing the issue.
	// since this will be all open source i will be following the CopyLeft by making the source of OPT available.
	if (help != 1) {
		return OPT_OK;
	}
	if (baseCommand == nullptr) {
		optPrintUsage();
		return OPT_EXIT;
	}
	else {
		std::cout << "Individual command help is coming later" << std::endl;
	}
	return OPT_EXIT;
}

auto loadOpt() -> void {
	optUsage("FakeGot [arguments/operation] [options]");
	optTitle("FakeGot: A shallow clone of GitGot using native code\r\n");
	optDisableMenu();
	optVersion("0.3");

	optregp(&baseCommand, OPT_STRING, "operation", "Operation to execute, for a list of operations pass in --help-commands");
	optdescript(&baseCommand, "The command to be executed among all of the filtered repositories");
	opthook(&baseCommand, convertCommand);

	optrega_array(&tagCount, &tags, OPT_STRING, 't', "tags", "Array of tags to be 'ORed' together");

	optreg(&printOperations, OPT_BOOL, '\0', "Lists all operations currently supported");
	optlongname(&printOperations, "help-commands");
	opthook(&printOperations, printAllOperations);

	optrega(&help, OPT_BOOL, '\0', "help", "Prints the help menu");
	opthook(&help, printSpecializedHelp);

	optrega(&verbose, OPT_BOOL, 'v', "verbose", "If we should be noisy");
	optrega(&quiet, OPT_BOOL, 'q', "quiet", "If we should suppress all output");
}

auto main(int argc, char* argv[]) -> int {
	//Prepare opt
	loadOpt();
	//Print the help if we didn't get an argument
	if (argc == 1) {
		optPrintUsage();
		return 0;
	}
	//Let opt at it. Note that argc is changed by this call.
	opt(&argc, &argv);

	//Here we free opt since we are currently done with it.
	opt_free();

	auto root = GetJSONConfig(verbose);
	if (root == nullptr)  {
		if (verbose) {
			std::cout << "Error in (or missing) configuration data" << std::endl;
		}
		return 1;
	}
	if (verbose && tagCount > 0){
		std::cout << baseCommand << " on tag count " << tagCount << " with tags:";
		for (int i = 0; i < tagCount; i++) {
			std::cout << " " << tags[i];
		}
		std::cout << std::endl;
	}
	int filteredRepoCount;
	auto repos = getRepos(root, &filteredRepoCount, tagCount, tags);
	if (repos == nullptr) {
		std::cout << "No repositories selected" << std::endl;
	}

	if (verbose) {
		for (int i = 0; i < filteredRepoCount; i++) {
			//For now print the matching repos.
			std::cout << repos[i].name << ": " << repos[i].path << std::endl;
		}
	}

	int result = RunGot(operation, filteredRepoCount, repos);

#ifdef DEBUG
	std::cout << "Press enter to exit.";
	std::cin.ignore();
#endif
	//We use the already allocated strings in the repo's so we reduce copying
	cJSON_Delete(root);
	return 0;
}



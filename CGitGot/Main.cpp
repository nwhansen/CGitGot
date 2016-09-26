#include <stdlib.h>
#include <iostream>
#include "cJSON.h"
#include "opt.h"
#include "git2.h"
#include "Repository.h"
#include "TagProcessor.h"
#include "Operations.h"

char* baseCommand = nullptr;
int printOperations = 0;
int quiet = 0;
int verbose = 0;
int help = 0;
int tagCount = 0;
char **tags = nullptr;
got::Operations operation;

auto getConfigPath() -> char* {
	char* documentPath;
	size_t size;
	getenv_s(&size, nullptr, 0, "USERPROFILE");

	if (size == 0) {
		std::cout << "Missing USERPROFILE to find config";
		return nullptr;
	}
	documentPath = (char*)malloc(size);
	if (documentPath == 0) {
		std::cout << "Unable to malloc for user profile";
		return nullptr;
	}
	getenv_s(&size, documentPath, size, "USERPROFILE");
	//This should be right

	//Now to have to create a path to a file i should be caring about..
	//We could do this via fancy streams instead le hacks shall ensue
	if (documentPath[size - 2] != '\\') {
		//We are missing the trailing \\ add them.
		//Remember that we need 4 new characters but sizeof of the file will also return the null 
		// which means we only need 3 more... magic...
		documentPath = (char*)realloc(documentPath, size + sizeof("got.conf"));
		//Write out the double slash
		size = size + sizeof("got.conf");
		strcat_s(documentPath, size, "\\");
	}
	else {
		//We don't want the null terminator there
		documentPath = (char*)realloc(documentPath, size + sizeof("got.conf") - 1);
		size = size + sizeof("got.conf") - 1;
	}
	strcat_s(documentPath, size, "got.conf");
	return documentPath;
}

auto getConfigData(char* documentPath) -> cJSON* {

	FILE *configFile = fopen(documentPath, "rb");
	if (configFile == nullptr) {
		std::cout << "Unable to open configuration file" << std::endl;
		return nullptr;
	}
	//Get the end of the file
	fseek(configFile, 0L, SEEK_END);
	long filesize = ftell(configFile);
	fseek(configFile, 0L, SEEK_SET);

	char* configData = (char*)malloc(filesize + 1);
	//Read all of the data
	size_t size = fread(configData, 1, filesize, configFile);
	//Null terminate it
	configData[size] = '\0';

	fclose(configFile);
	//Parse it now.
	return cJSON_Parse(configData);
}

auto getRepos(cJSON* root, int* filteredRepoCount, int tagCount, char** tags) -> Repository** {
	*filteredRepoCount = 0;

	Repository** repos = nullptr;
	cJSON* repoJSON = cJSON_GetObjectItem(root, "repos");
	if (tagCount > 0) {
		FilterByTags(repoJSON, filteredRepoCount, tagCount, tags);
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
	//Print the descriptions if we didn't get a parameter
	if (argc == 1) {
		optPrintUsage();
		return 0;
	}
	//Let opt at it. Note that argc is changed by this call.
	opt(&argc, &argv);

	//Here we free opt since it uses lots of globals this will lower our overhead by a bit.
	opt_free();

	//Begin the process of loading the configuration file.
	size_t size;
	char* documentPath = getConfigPath();
	if (documentPath == nullptr) {
		return 1;
	}
	if (verbose) {
		std::cout << "Configuration Path: " << documentPath << std::endl;
	}
	auto root = getConfigData(documentPath);
	if (root == nullptr)  {
		return 1;
	}
	else {
		//Free the path to the document for now we don't allow editing yet
		free(documentPath);
	}
	if (verbose){
		std::cout << baseCommand << " on tag count " << tagCount;
		std::cout << "with tags:";
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
	cJSON_Delete(root);

	if (verbose) {
		for (int i = 0; i < filteredRepoCount; i++) {
			//Do operation specified for each repo...? or pass this on.
			//For now print the matching repos.
			std::cout << repos[i]->name << ": " << repos[i]->path;
		}
	}
#ifdef DEBUG
	std::cout << "Press enter to exit.";
	std::cin.ignore();
	char destChar;
	//std::cin.get(destChar);
#endif
	return 0;
}



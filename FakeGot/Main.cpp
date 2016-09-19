#include <stdlib.h>
#include <iostream>
#include "cJSON.h"
#include "opt.h"
#include "Repository.h"
#include "TagProcessor.h"

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
	filteredRepoCount = 0;
	
	Repository** repos = nullptr;
	cJSON* repoJSON = cJSON_GetObjectItem(root, "repos");
	if (tagCount > 0) {
		FilterByTags(repoJSON, filteredRepoCount, tagCount, tags);
	}
	else {
		cJSON* config = cJSON_GetObjectItem(root, "configuration");
		if (config != nullptr) {
			cJSON* noTagSelectsAll = cJSON_GetObjectItem(config, "noTagSelectsAll");
			if (noTagSelectsAll != nullptr || noTagSelectsAll->type == cJSON_True) {
				repos = AllToRepos(repoJSON, filteredRepoCount);
			}
		}
	}
	return repos;
}

auto main(int argc, char* argv[]) -> int {
	//Check the arguments
	char* baseCommand = nullptr;
	bool printCommands;
	int tagCount = 0;
	char **tags = nullptr;
	optUsage("This program is a shallow clone of GitGot, which utilized perl. This is a natively compiled to help improve performance on windows boxes");
	optTitle("FakeGot");
	optVersion("0.3");
	optregp(&baseCommand, OPT_STRING, "operation", "Operation to execute, for a list of operations pass in --help-commands");
	optdescript(&baseCommand, "The command to be executed among all of the filtered repositories");
	optrega_array(&tagCount, &tags, OPT_STRING, 't', "tags", "Array of tags to be 'ORed' together");
	optreg(&printCommands, OPT_BOOL, '\0', "help-commands");
	opt(&argc, &argv);
	
	if (argc == 1) {
		optPrintUsage();
		return 0;
	}

	opt_free();
	//Begin the process of loading the configuration file.
	size_t size;
	char* documentPath = getConfigPath();
	if (documentPath == nullptr) {
		return 1;
	}
	std::cout << "Configuration Path: " << documentPath << std::endl;
	auto root = getConfigData(documentPath);
	if (root == nullptr)  {
		return 1;
	}
	else {
		//Free the path to the document for now we don't allow editing yet
		free(documentPath);
	}

	std::cout << baseCommand << " on tag count " << tagCount;
	std::cout << "with tags:";
	for (int i = 0; i < tagCount; i++) {
		std::cout << " " << tags[i];
	}
	std::cout << std::endl;
	int filteredRepoCount;
	auto repos = getRepos(root, &filteredRepoCount, tagCount, tags);
	if (repos == nullptr) {
		std::cout << "No repositories selected";
	}
	cJSON_Delete(root);
	
	for (int i = 0; i < filteredRepoCount; i++) {
		//Do operation specified for each repo...? or pass this on.
		//For now print the matching repos.
		std::cout << repos[i]->name << ": " << repos[i]->path;
	}

	char tmp[255];
	std::cin >> tmp; 
	return 0;
}



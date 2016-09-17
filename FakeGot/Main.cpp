#include "cJSON.h"
#include "opt.h"
#include <stdlib.h>
#include <iostream>

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
		std::cout << "Unable to open config file" << std::endl;
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

auto main(int argc, char* argv[]) -> int {
	//Check the arguments
	int test = 0;
	optreg(&test, OPT_INT, 't', "Testing");
	opt(&argc, &argv);
	
	opt_free();
	if (test != 0) {
		std::cout << "Hey you passed a param " << test << std::endl;
	}
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
		free(documentPath);
	}
	std::cout << cJSON_Print(root);
	free(root);
	
	char tmp[255];
	std::cin >> tmp; 
	return 0;
}



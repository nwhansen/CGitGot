#ifdef _MSC_VER
//The Windows specific configuration setup.
#include <stdlib.h>
#include <iostream>
#include "Configuration.h"


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

auto GetJSONConfig(int verbose) -> cJSON* {
	//Begin the process of loading the configuration file.
	char* documentPath = getConfigPath();
	if (documentPath == nullptr) {
		return nullptr;
	}
	if (verbose) {
		std::cout << "Configuration Path: " << documentPath << std::endl;
	}
	auto result = getConfigData(documentPath);
	free(documentPath);
	return result;
}



#endif
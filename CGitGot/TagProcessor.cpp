#include <stdlib.h>
#include <iostream>
#include "cJSON.h"
#include "TagProcessor.h"


Repository** FilterByTags(cJSON* root, int* matchedRepositoryCount, int taglistSize, char** tagList) {
	*matchedRepositoryCount = 0;
	return nullptr;
}

Repository* AllToRepos(cJSON* root, int* matchedRepositoryCount) {
	int count =  cJSON_GetArraySize(root);
	if (count == 0) {
		return nullptr;
	}
	Repository* dest = (Repository*)malloc(sizeof(Repository) * count);
	//Start walking the "base"
	root = root->child;
	Repository* current = dest;
	do {
		//current->name = cJSON_GetValueA
		auto name = cJSON_GetObjectItem(root, "name");
		if (name == nullptr) {
			std::cout << "F this";
			free(dest);
			return nullptr;
		}
		current->name = name->valuestring;
		auto path = cJSON_GetObjectItem(root, "path");
		if (name == nullptr) {
			std::cout << "Missing path for " << current->name;
			free(dest);
			return nullptr;
		}
		current->path = path->valuestring;
		current = current + 1;
		root = root->next;
	} while (root != nullptr);
	*matchedRepositoryCount = count;
	return dest;
}
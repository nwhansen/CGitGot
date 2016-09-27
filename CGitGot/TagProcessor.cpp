#include <stdlib.h>
#include <iostream>
#include "cJSON.h"
#include "TagProcessor.h"


auto blarg(cJSON* tags, int taglistSize, char** taglist) -> bool {
	cJSON* tag = tags->child;
	do {
		for (int i = 0; i < taglistSize; i++) {
			if (strcmp(tag->valuestring, taglist[i]) == 0) {
				return true;
			}
		}
		tag = tag->next;
	} while (tag != nullptr);
	return false;
}


Repository* FilterByTags(cJSON* root, int* matchedRepositoryCount, int taglistSize, char** tagList) {
	*matchedRepositoryCount = 0;
	int count = cJSON_GetArraySize(root);
	if (count == 0) {
		return nullptr;
	}
	Repository* dest = (Repository*)malloc(sizeof(Repository) * count);
	//Start walking the "base"
	Repository* current = dest;
	root = root->child;
	do {
		cJSON* tags = cJSON_GetObjectItem(root, "tags");
		if (tags != nullptr && blarg(tags, taglistSize, tagList)) {
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
		}
		root = root->next;
	} while (root != nullptr);

	*matchedRepositoryCount = std::abs(current - dest);
	return dest;
}

Repository* AllToRepos(cJSON* root, int* matchedRepositoryCount) {
	*matchedRepositoryCount = 0;
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
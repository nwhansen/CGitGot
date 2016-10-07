
#include <iostream>
#include "git2.h"
#include "GotOperations.h"
#include "OutputRenderer.h"

static const auto CLEAN = "CLEAN";
static const auto DIRTY = "DIRTY";

auto RunStatus(Repository* repo) -> void {
	git_repository* gitRepo; 
	int ret = git_repository_open_ext(&gitRepo, repo->path, 0, nullptr);
	if (ret) {
		//Need to write something here i guess
		std::cout << "No repo at: " << repo->path << std::endl;
		return;
	}
	if (git_repository_is_bare(gitRepo)) {
		std::cout << "Cannot status a bare repo at: " << repo->path << std::endl;
		git_repository_free(gitRepo);
		return;
	}
	git_status_list *list;
	git_status_options options;
	git_status_init_options(&options, GIT_STATUS_OPTIONS_VERSION);
	//Query the status
	git_status_list_new(&list, gitRepo, &options);
	
	//Find out if there is entries.
	int state = git_status_list_entrycount(list);	
	
	const char* stateString; 
	//For now we don't care if those are new or modified. This will be handled later I hope..
	// Or this could be broken into 2 searches so that we just "test" that there is something. instead of iterating over all of them.
	if (state == 0) {
		stateString = CLEAN;
	}
	else {
		stateString = DIRTY;
	}

	RenderOutput(repo, stateString);

	git_status_list_free(list);
	git_repository_free(gitRepo);

}

auto statusOnRepos(int filteredRepoCount, Repository* repos) -> void{
	for (int i = 0; i < filteredRepoCount; i++) {
		RunStatus(&(repos[i]));
	}
}


auto RunGot(Operations operation, int filteredRepoCount, Repository* repos) -> int {
	git_libgit2_init();
	switch (operation) {
	case status:
		statusOnRepos(filteredRepoCount, repos);
		break;
	default:
		break;
	}

	git_libgit2_shutdown();
	return 0;
}
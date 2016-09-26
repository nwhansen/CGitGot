
#ifndef REPOSITORY_H
#define REPOSITORY_H

struct Repository
{
public:
	char* name;
	char* path;
	char** tags;
};

#endif
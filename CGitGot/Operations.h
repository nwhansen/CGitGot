#ifndef OPERATIONS_H
#define OPERATIONS_H

// These need to be extern so that we don't violate the one definition rule.
extern char* STATUS;
extern char* PULL;

enum Operations
{
	no_op = 0,
	status = 1,
	pull = 2
};


#endif
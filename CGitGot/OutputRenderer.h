#ifndef OUTPUTRENDERER_H
#define OUTPUTRENDERER_H

#include "Repository.h"

/*
 * Renders the repository data and then the message to be displayed
 */
auto RenderOutput(Repository* repo, const char* message) -> void;


#endif
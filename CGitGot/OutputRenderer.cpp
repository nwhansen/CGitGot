
#include "OutputRenderer.h"
#include <iostream>

auto RenderOutput(Repository * repo, const char* message) -> void {

	std::cout << repo->name << '\t' << message << std::endl;
}
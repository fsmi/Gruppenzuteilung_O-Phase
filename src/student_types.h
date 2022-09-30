#pragma once

#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <string>

#include "definitions.h"

std::vector<std::pair<Filter, StudentID>> parseTypesFile(std::ifstream& file);

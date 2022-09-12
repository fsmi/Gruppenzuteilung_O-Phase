#pragma once

#include <iostream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <string>

#include "definitions.h"

using FilterFn = std::function<bool(const StudentData&)>;

std::vector<std::tuple<FilterFn, StudentID, std::string>> parseTypesFile(std::ifstream& file);

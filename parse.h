#pragma once

#include <string>

#include "boost/property_tree/ptree.hpp"

#include "definitions.h"

using PTree = boost::property_tree::ptree;

GroupData parseGroup(const PTree &tree);

StudentData parseStudent(const PTree &tree);

TeamData parseTeam(const PTree &tree);

Rating parseRating(const PTree &tree);

Input parseInput(const PTree &tree);

PTree writeOutputToTree(const State &s);

void writeOutputToFiles(const State &s, std::string path,
    const std::vector<std::pair<std::function<bool(const StudentData&)>, std::string>> &filters = {});

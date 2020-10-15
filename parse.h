#pragma once

#include "boost/property_tree/ptree.hpp"

#include "definitions.h"

using PTree = boost::property_tree::ptree;

GroupData parseGroup(const PTree &tree);

StudentData parseStudent(const PTree &tree);

TeamData parseTeam(const PTree &tree);

Rating parseRating(const PTree &tree);

Input parseInput(const PTree &tree);

PTree writeOutput(const State &s);

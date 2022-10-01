#pragma once

#include <string>

#include "boost/property_tree/ptree.hpp"

#include "definitions.h"
#include "student_types.h"

using PTree = boost::property_tree::ptree;

Input parseInput(const PTree &tree);

PTree writeOutputToTree(const State &s);

void writeOutputToFiles(const State &s, std::string path,
    const std::vector<std::pair<Filter, StudentID>> &filters = {});

std::string courseTypeToString(CourseType type);

std::string degreeTypeToString(DegreeType type);

std::string semesterToString(Semester semester);

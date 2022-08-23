#pragma once

#include "definitions.h"

std::pair<std::vector<int32_t>, bool> calculateAssignment(const State &s, bool top_level);

bool applyAssignment(State &s, const std::vector<int32_t> &assignment,
                     bool teams = true, bool students = true, bool top_level = true);

bool assignTeamsAndStudents(State &s, bool top_level);

void assignWithMinimumNumberPerGroup(
    State &s, StudentID min_capacity);

void assertMinimumNumberPerGroupForSpecificType(State &s,
    const std::vector<std::tuple<std::function<bool(const StudentData&)>, StudentID, std::string>> &filters);

void printCurrentAssignment(const State &s);

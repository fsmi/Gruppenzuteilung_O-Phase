#pragma once

#include "definitions.h"
#include "student_types.h"

void signalHandler(int);

std::pair<std::vector<int32_t>, bool> calculateAssignment(const State &s, std::mt19937_64& gen, bool top_level);

bool applyAssignment(State &s, const std::vector<int32_t> &assignment,
                     bool teams = true, bool students = true, bool top_level = true);

bool assignTeamsAndStudents(State &s, bool top_level);

void assignWithMinimumNumberPerGroup(
    State &s, StudentID min_capacity);

void assertMinimumNumberPerGroupForSpecificType(State &s, const std::vector<std::pair<Filter, StudentID>>& filters);

// given a specific filter, returns the groups sorted in ascending
// order by the number of students that would be removed from the
// group when applying the filter
std::vector<std::pair<GroupID, StudentID>>
groupsByNumFiltered(const State &s, StudentID min_members, const Filter& filter);

void printCurrentAssignment(const State &s);

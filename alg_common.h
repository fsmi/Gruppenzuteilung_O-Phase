#pragma once

#include "definitions.h"

// given a specific filter, returns the groups sorted in ascending
// order by the number of students that would be removed from the
// group when applying the filter
std::vector<std::tuple<GroupID, StudentID, bool>>
groupsByNumFiltered(const State &s, StudentID min_members,
               std::function<bool(const StudentData &)> filter);

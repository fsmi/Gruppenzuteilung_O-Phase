#pragma once

#include "definitions.h"

std::vector<std::pair<GroupID, StudentID>>
groupsByNumFiltered(const State &s, StudentID min_members,
               std::function<bool(const StudentData &)> filter);

#pragma once

#include "definitions.h"

std::vector<int32_t> calculateAssignment(const State &s);

bool applyAssignment(State &s, const std::vector<int32_t> &assignment,
                     bool teams = true, bool students = true);

void assignTeamsAndStudents(State &s,
                            StudentID initial_capacity = INITIAL_GROUP_CAPACITY,
                            bool fair_capacity = true);

void assignWithMinimumNumberPerGroup(
    State &s, StudentID min_capacity,
    StudentID initial_capacity = INITIAL_GROUP_CAPACITY,
    bool fair_capacity = true);

void printCurrentAssignment(const State &s);

#pragma once

#include "definitions.h"

std::vector<int32_t> calculateAssignment(const State &s);

bool applyAssignment(State &s, const std::vector<int32_t> &assignment,
                     bool teams = true, bool students = true);

void assignTeamsAndStudents(State &s);

void assignWithMinimumNumberPerGroup(
    State &s, StudentID min_capacity);

void printCurrentAssignment(const State &s);

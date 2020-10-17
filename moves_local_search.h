#pragma once

#include <assert.h>

#include "definitions.h"

struct MoveStep {
  size_t parent;
  int32_t path_rating;
  ParticipantID participant;
  GroupID target;

  MoveStep(size_t parent, int32_t path_rating, ParticipantID participatn,
           GroupID target);
};

struct SearchLevel {
  size_t parent;
  uint32_t level;
  std::vector<ParticipantID> participants;

  SearchLevel(size_t parent, std::vector<ParticipantID>&& participants);

  SearchLevel next() const;
};

std::vector<GroupID> groupsByNumCourse(const State &s, CourseType course,
                                       StudentID min_members);

void assertMinNumCourse(State &s, CourseType course, StudentID min);

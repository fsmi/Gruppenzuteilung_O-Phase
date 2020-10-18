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

  SearchLevel(size_t parent);

  SearchLevel next() const;
};

class MoveSequence {
  int32_t _rating;
  GroupID _source;
  std::vector<std::pair<ParticipantID, GroupID>> _seq;

public:
  MoveSequence(int32_t rating, GroupID source);

  int32_t rating() const;

  void modifyRating(int32_t val);

  // moves are applied FIFO
  void push(ParticipantID part, GroupID target);

  // assumes the source participant is already removed
  void apply(State &s, bool print_moves) const;
};

std::vector<GroupID>
groupsByNumCourse(const State &s, CourseType course, StudentID min_members);

void assertMinNumCourse(State &s, CourseType course, StudentID min);

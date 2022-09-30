#pragma once

#include <assert.h>
#include <functional>

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

  SearchLevel(size_t parent, uint32_t min_level);

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
groupsByNumber(const State &s, StudentID min_members,
               std::function<bool(const StudentData &)> predicate);

std::vector<StudentID>
numPerGroup(const State &s, std::function<bool(const StudentData &)> predicate);

std::optional<MoveSequence>
moveFromGroup(const State &s, GroupID group,
              std::function<bool(const StudentData &)> predicate,
              int32_t best_expected = 0);

void moveAllFromGroup(State &s, GroupID group, StudentID min,
                      std::function<bool(const StudentData &)> predicate);

void assertMininumNumber(State &s, StudentID min,
                         std::function<bool(const StudentData &)> predicate);

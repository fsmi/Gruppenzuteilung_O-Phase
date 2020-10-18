#include "moves_local_search.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <optional>
#include <variant>

const static int64_t STEPS_PER_VALUE = 1000;

void printMove(const State &s, ParticipantID part, GroupID from, GroupID to) {
  std::string p_name =
      s.isTeam(part) ? s.teamData(part).name : s.studentData(part).name;
  std::cout << "move \"" << p_name << "\" from group \""
            << s.groupData(from).name << "\" to group \""
            << s.groupData(to).name << "\", ";
}

MoveStep::MoveStep(size_t parent, int32_t path_rating,
                   ParticipantID participant, GroupID target)
    : parent(parent), path_rating(path_rating), participant(participant),
      target(target) {}

SearchLevel::SearchLevel(size_t parent)
    : parent(parent), level(NUM_RATINGS - 1) {}

SearchLevel SearchLevel::next() const {
  assert(level > 0);
  SearchLevel result(*this);
  result.level--;
  return result;
}

MoveSequence::MoveSequence(int32_t rating, GroupID source)
    : _rating(rating), _source(source), _seq() {}

int32_t MoveSequence::rating() const { return _rating; }

void MoveSequence::modifyRating(int32_t val) { _rating += val; }

// moves are applied FIFO
void MoveSequence::push(ParticipantID part, GroupID target) {
  _seq.emplace_back(part, target);
}

void MoveSequence::apply(State &s, bool print_moves) const {
  for (size_t i = 0; i < _seq.size(); ++i) {
    GroupID from = (i == _seq.size() - 1) ? _source : _seq[i + 1].second;
    s.unassignParticipant(_seq[i].first, from);
    assert(s.assignParticipant(_seq[i].first, _seq[i].second));
    if (print_moves) {
      printMove(s, _seq[i].first, from, _seq[i].second);
    }
  }
}

std::vector<GroupID> groupsByNumCourse(const State &s, CourseType course,
                                       StudentID min_members) {
  std::vector<std::pair<GroupID, StudentID>> groups;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    const auto &list = s.groupAssignmentList(group);
    StudentID num = 0;
    if (!list.empty() && s.groupIsEnabled(group)) {
      for (const auto &pair : list) {
        StudentData data = s.data().students[pair.first];
        if (data.course_type == course) {
          num++;
        }
      }
      if (num > 0 && num < min_members) {
        groups.emplace_back(group, num);
      }
    }
  }
  std::sort(groups.begin(), groups.end(), [&](const auto &p1, const auto &p2) {
    return p1.second < p2.second;
  });
  std::vector<GroupID> result;
  for (const auto &pair : groups) {
    result.push_back(pair.first);
  }
  return std::move(result);
}

std::vector<StudentID> numPerGroup(const State &s, CourseType course) {
  std::vector<StudentID> num_per_group(s.numGroups(), 0);
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    const auto &list = s.groupAssignmentList(group);
    for (const auto &pair : list) {
      StudentData data = s.data().students[pair.first];
      if (data.course_type == course) {
        num_per_group[group]++;
      }
    }
  }
  return std::move(num_per_group);
}

// perform a BFS for the best possible move
std::optional<MoveSequence> moveFromGroup(const State &s, CourseType course,
                                          GroupID group) {
  std::vector<MoveStep> search_tree;
  search_tree.emplace_back(0, 0, 0, group);
  std::deque<std::variant<MoveStep, SearchLevel>> queue;
  queue.emplace_back(SearchLevel(0));

  int64_t counter = 0;
  size_t max_idx = 0;
  while (!queue.empty() &&
         (max_idx == 0 ||
          (-static_cast<int64_t>(search_tree[max_idx].path_rating) *
               STEPS_PER_VALUE >
           counter))) {
    std::variant<MoveStep, SearchLevel> next = queue.front();
    queue.pop_front();
    if (std::holds_alternative<MoveStep>(next)) {
      MoveStep step = std::get<MoveStep>(next);
      size_t index = search_tree.size();
      search_tree.push_back(step);
      if (s.groupCapacity(step.target) > 0) {
        if (max_idx == 0 ||
            step.path_rating > search_tree[max_idx].path_rating) {
          max_idx = index;
        }
      } else {
        queue.emplace_back(SearchLevel(index));
      }
    } else {
      SearchLevel lvl = std::get<SearchLevel>(next);
      std::vector<bool> group_allowed(s.numGroups(), true);
      size_t parent = lvl.parent;
      MoveStep parent_node = search_tree[parent];
      do {
        MoveStep node = search_tree[parent];
        parent = node.parent;
        group_allowed[node.target] = false;
      } while (search_tree[parent].parent != parent);
      std::vector<ParticipantID> participants;
      for (const auto &pair : s.groupAssignmentList(parent_node.target)) {
        if (!s.isTeam(pair.second) &&
            (s.studentData(pair.second).course_type != course)) {
          const std::vector<Rating> &rating = s.rating(pair.second);
          for (GroupID i = 0; i < s.numGroups(); ++i) {
            if (s.groupIsEnabled(i) && group_allowed[i] &&
                (rating[i].index == lvl.level)) {
              MoveStep step(lvl.parent,
                            parent_node.path_rating +
                                static_cast<int32_t>(rating[i].getValue()) -
                                static_cast<int32_t>(
                                    rating[parent_node.target].getValue()),
                            pair.second, i);
              queue.emplace_back(step);
            }
          }
        }
      }
      if (lvl.level > 0) {
        SearchLevel next_lvl = lvl.next();
        queue.emplace_back(next_lvl);
      }
    }
    counter++;
  }

  int32_t best_rating = search_tree[max_idx].path_rating;
  if (max_idx == 0) {
    return std::optional<MoveSequence>();
  }
  MoveSequence result(best_rating, group);
  for (size_t current = max_idx; current > 0;
       current = search_tree[current].parent) {
    result.push(search_tree[current].participant, search_tree[current].target);
  }
  return std::make_optional(std::move(result));
}

std::optional<std::pair<std::vector<MoveSequence>, int32_t>>
moveAllFromGroup(const State &s, CourseType course, GroupID group,
                 StudentID number) {
  assert(s.groupSize(group) >= number);
  int32_t total_rating = 0;
  std::vector<MoveSequence> result;
  State current_state(s);
  for (StudentID i = 0; i < number; ++i) {
    std::optional<MoveSequence> seq =
        moveFromGroup(current_state, course, group);
    if (!seq.has_value()) {
      return std::optional<std::pair<std::vector<MoveSequence>, int32_t>>();
    }
    total_rating += seq->rating();
    seq->apply(current_state, false);
    result.push_back(std::move(*seq));
  }
  return std::make_optional(std::make_pair(std::move(result), total_rating));
}

void moveCourseFromGroup(State &s, CourseType course, GroupID group,
                         StudentID min, bool print_moves) {
  while (true) {
    const std::vector<StudentID> num_per_group = numPerGroup(s, course);
    if (num_per_group[group] == 0 || num_per_group[group] >= min) {
      break;
    }
    const auto &assignment = s.groupAssignmentList(group);
    ParticipantID part =
        std::find_if(assignment.cbegin(), assignment.cend(),
                     [&](const auto &pair) {
                       return s.data().students[pair.first].course_type ==
                              course;
                     })
            ->second;
    s.unassignParticipant(part, group);

    StudentID num_to_remove;
    if (s.isTeam(part)) {
      num_to_remove = s.teamData(part).size();
    } else {
      num_to_remove = 1;
    }
    const std::vector<Rating> &rating = s.rating(part);
    int32_t max_rating = std::numeric_limits<int32_t>::min();
    GroupID max_id = 0;
    std::vector<MoveSequence> max_seqs;
    for (GroupID g_id = 0; g_id < s.numGroups(); ++g_id) {
      if (s.groupIsEnabled(g_id) && (g_id != group) &&
          ((num_per_group[g_id] >= min - 1) ||
           num_per_group[g_id] > num_per_group[group])) {
        StudentID g_num_remove;
        if (s.groupCapacity(g_id) >= num_to_remove) {
          g_num_remove = 0;
        } else {
          g_num_remove = num_to_remove - s.groupCapacity(g_id);
        }
        auto result = moveAllFromGroup(s, course, g_id, g_num_remove);
        if (result.has_value()) {
          int32_t curr_rating =
              result->second +
              static_cast<int32_t>(num_to_remove) *
                  (static_cast<int32_t>(rating[g_id].getValue()) -
                   static_cast<int32_t>(rating[group].getValue()));
          if (curr_rating > max_rating) {
            max_rating = curr_rating;
            max_id = g_id;
            max_seqs = std::move(result->first);
          }
        }
      }
    }
    assert(max_rating > std::numeric_limits<int32_t>::min());
    if (print_moves) {
      std::cout << "Moves: ";
    }
    for (const MoveSequence &seq : max_seqs) {
      seq.apply(s, print_moves);
    }

    assert(s.assignParticipant(part, max_id));
    if (print_moves) {
      printMove(s, part, group, max_id);
    }
    if (print_moves) {
      std::cout << " (diff=" << max_rating << ")." << std::endl;
    }
  }
}

void assertMinNumCourse(State &s, CourseType course, StudentID min) {
  std::vector<GroupID> groups = groupsByNumCourse(s, course, min);
  while (!groups.empty()) {
    moveCourseFromGroup(s, course, groups[0], min, true);
    groups = groupsByNumCourse(s, course, min);
  }
  std::vector<StudentID> num_per_group = numPerGroup(s, course);
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    std::cout << s.groupData(group).name << ": " << num_per_group[group]
              << std::endl;
  }
}

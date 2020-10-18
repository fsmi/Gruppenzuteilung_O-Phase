#include "moves_local_search.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <variant>

const static int64_t STEPS_PER_VALUE = 1000;

MoveStep::MoveStep(size_t parent, int32_t path_rating,
                   ParticipantID participant, GroupID target)
    : parent(parent), path_rating(path_rating), participant(participant),
      target(target) {}

SearchLevel::SearchLevel(size_t parent,
                         std::vector<ParticipantID> &&participants)
    : parent(parent), level(NUM_RATINGS - 1), participants(participants) {}

SearchLevel SearchLevel::next() const {
  assert(level > 0);
  SearchLevel result(*this);
  result.level--;
  return result;
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

bool moveFromGroup(State &s, CourseType course, GroupID group, StudentID min,
                   const std::vector<StudentID> &num_per_group,
                   const std::vector<bool> &has_team) {
  const auto &assignment = s.groupAssignmentList(group);
  ParticipantID part =
      std::find_if(assignment.cbegin(), assignment.cend(),
                   [&](const auto &pair) {
                     return s.data().students[pair.first].course_type == course;
                   })
          ->second;
  assert(!s.isTeam(part));
  s.unassignParticipant(part, group);

  // perform a BFS for the best possible move
  std::vector<MoveStep> search_tree;
  search_tree.emplace_back(0, 0, part, group);
  std::deque<std::variant<MoveStep, SearchLevel>> queue;
  queue.emplace_back(SearchLevel(0, {part}));

  int64_t counter = 0;
  MoveStep best(0, std::numeric_limits<int32_t>::min(), part, group);
  while (
      !queue.empty() &&
      (-static_cast<int64_t>(best.path_rating) * STEPS_PER_VALUE > counter)) {
    std::variant<MoveStep, SearchLevel> next = queue.front();
    queue.pop_front();
    if (std::holds_alternative<MoveStep>(next)) {
      MoveStep step = std::get<MoveStep>(next);
      if (s.groupCapacity(step.target) > 0) {
        if (step.path_rating > best.path_rating) {
          best = step;
        }
      } else {
        size_t index = search_tree.size();
        search_tree.push_back(step);
        std::vector<ParticipantID> participants;
        for (const auto &pair : s.groupAssignmentList(step.target)) {
          if (!s.isTeam(pair.second) &&
              (s.studentData(pair.second).course_type != course)) {
            participants.push_back(pair.second);
          }
        }
        queue.emplace_back(SearchLevel(index, std::move(participants)));
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
      for (const ParticipantID &part : lvl.participants) {
        const std::vector<Rating> &rating = s.rating(part);
        for (GroupID i = 0; i < s.numGroups(); ++i) {
          bool valid_target =
              s.groupIsEnabled(i) &&
              ((num_per_group[i] >= min - 1) || has_team[i] ||
               num_per_group[i] > num_per_group[parent_node.target]);
          if (valid_target && group_allowed[i] &&
              (rating[i].index == lvl.level)) {
            MoveStep step(
                lvl.parent,
                parent_node.path_rating +
                    static_cast<int32_t>(rating[i].getValue()) -
                    static_cast<int32_t>(rating[parent_node.target].getValue()),
                part, i);
            queue.emplace_back(step);
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

  // apply move
  std::cout << "Moves: ";
  assert(best.path_rating > std::numeric_limits<int32_t>::min());
  MoveStep current = best;
  MoveStep previous = best;
  while (search_tree[current.parent].parent != current.parent) {
    current = search_tree[current.parent];
    std::cout << "move \"" << s.studentData(current.participant).name
              << "\" from group \"" << s.groupData(current.target).name
              << "\" to group \"" << s.groupData(previous.target).name
              << "\", ";
    s.unassignParticipant(previous.participant, current.target);
    s.assignParticipant(previous.participant, previous.target);
    previous = current;
  }
  assert(current.participant == part);
  std::cout << "move \"" << s.studentData(current.participant).name
            << "\" from group \"" << s.groupData(group).name << "\" to group \""
            << s.groupData(current.target).name
            << "\" (diff=" << best.path_rating << ")." << std::endl;
  s.assignParticipant(current.participant, current.target);
  return true;
}

void assertMinNumCourse(State &s, CourseType course, StudentID min) {
  std::vector<GroupID> groups = groupsByNumCourse(s, course, min);
  for (GroupID group : groups) {
    while (true) {
      std::vector<StudentID> num_per_group(s.numGroups(), 0);
      std::vector<bool> has_team(s.numGroups(), false);
      for (GroupID group = 0; group < s.numGroups(); ++group) {
        const auto &list = s.groupAssignmentList(group);
        for (const auto &pair : list) {
          StudentData data = s.data().students[pair.first];
          if (data.course_type == course) {
            num_per_group[group]++;
          }
          if (s.isTeam(pair.second)) {
            has_team[group] = true;
          }
        }
      }
      if (num_per_group[group] == 0 || num_per_group[group] >= min ||
          has_team[group]) {
        break;
      }
      moveFromGroup(s, course, group, min, num_per_group, has_team);
    }
  }
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
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    std::cout << s.groupData(group).name << ": " << num_per_group[group] << std::endl;
  }
}

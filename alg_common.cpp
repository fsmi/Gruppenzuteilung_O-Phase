#include "alg_common.h"

#include <vector>

std::vector<std::tuple<GroupID, StudentID, bool>>
groupsByNumFiltered(const State &s, StudentID min_members,
               std::function<bool(const StudentData &)> filter) {
  std::vector<std::tuple<GroupID, StudentID, bool>> groups;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    const auto &list = s.groupAssignmentList(group);
    StudentID num = 0;
    if (!list.empty() && s.groupIsEnabled(group)) {
      bool contains_only_ignored = true;
      for (const auto &pair : list) {
        StudentData data = s.data().students[pair.first];
        if (filter(data)) {
          num++;
          if (data.type_specific_assignment) {
            contains_only_ignored = false;
          }
        }
      }
      if (num < min_members) {
        groups.emplace_back(group, num, contains_only_ignored);
      }
    }
  }
  std::sort(groups.begin(), groups.end(), [&](const auto &p1, const auto &p2) {
    return std::get<1>(p1) < std::get<1>(p2);
  });
  return groups;
}

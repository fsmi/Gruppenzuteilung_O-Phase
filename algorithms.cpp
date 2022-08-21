#include "algorithms.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/maximum_weighted_matching.hpp>
#include <iostream>
#include <chrono>

#include "alg_common.h"

using EdgeProperty = boost::property<boost::edge_weight_t, uint32_t>;
using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                    boost::no_property, EdgeProperty>;
using GraphTraits = boost::graph_traits<Graph>;


// ##########################################
// ########     Helper Functions     ########
// ##########################################

void printCurrentAssignment(const State &s) {
  std::cout << std::endl;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    const GroupData &gd = s.groupData(group);
    std::cout << gd.name << std::endl;
    for (const auto &pair : s.groupAssignmentList(group)) {
      const std::string &rating = s.data().ratings[pair.first][group].getName();
      std::cout << "  - " << s.data().students[pair.first].name << " ["
                << rating << "]" << std::endl;
    }
    std::cout << std::endl;
  }
}

bool combinationIsValid(const StudentData &student, const GroupData &group) {
  bool course_is_valid = (group.course_type == CourseType::Any) ||
                         (student.course_type == group.course_type);
  // bool degree_is_valid = (group.degree_type == DegreeType::Any) ||
  //                        (student.degree_type == group.degree_type);
  return course_is_valid;
}

bool combinationIsValid(const TeamData &team, const GroupData &group,
                        const std::vector<StudentData> &students) {
  bool valid = true;
  for (StudentID student : team.members) {
    valid &= combinationIsValid(students[student], group);
  }
  return valid;
}

// returns a factor for the weights of a single participant
double getFactor(const State &/*s*/, ParticipantID /*part*/) {
  // if (s.isTeam(part)) {
  //   for (StudentID member : s.teamData(part).members) {
  //     if (s.data().students[member].is_commuter) {
  //       return 2.0;
  //     }
  //   }
  // } else {
  //   if (s.studentData(part).is_commuter) {
  //     return 2.0;
  //   }
  // }
  return 1.0;
}

// ####################################
// ########     Algorithms     ########
// ####################################

std::pair<std::vector<int32_t>, bool> calculateAssignment(const State &s) {
  // initialize vertices
  std::vector<GraphTraits::vertex_descriptor> first_group_vertex;
  std::vector<GroupID> vertex_to_group;
  first_group_vertex.push_back(0);
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    if (s.groupIsEnabled(group)) {
      for (StudentID i = 0; i < s.groupCapacity(group); ++i) {
        vertex_to_group.push_back(group);
      }
    }
    first_group_vertex.push_back(vertex_to_group.size());
  }
  std::vector<ParticipantID> participants;
  const GraphTraits::vertex_descriptor first_participant =
      vertex_to_group.size();
  GraphTraits::vertex_descriptor num_vertices = first_participant;
  for (ParticipantID part = 0; part < s.numParticipants(); ++part) {
    if (!s.isAssigned(part)) {
      num_vertices++;
      participants.push_back(part);
    }
  }
  if (first_participant < num_vertices - first_participant) {
    std::cerr << "Not enough capacity available: " << (num_vertices - first_participant)
              << " participants, but only " << first_participant << " group vertices!" << std::endl;
    std::exit(-1);
  }

  Graph g(num_vertices);
  std::vector<GraphTraits::vertex_descriptor> mates(num_vertices);

  // add edges
  for (ParticipantID i = 0; i < participants.size(); ++i) {
    const ParticipantID &part = participants[i];
    assert(!s.isAssigned(part));
    double factor = getFactor(s, part);
    for (GroupID group = 0; group < s.numGroups(); ++group) {
      bool validTeam = s.isTeam(part) &&
                       combinationIsValid(s.teamData(part), s.groupData(group),
                                          s.data().students);
      bool validStudent =
          !s.isTeam(part) &&
          combinationIsValid(s.studentData(part), s.groupData(group));
      if (!s.isExludedFromGroup(part, group) && (validTeam || validStudent)) {
        uint32_t rating = ceil(factor * s.rating(part).at(group).getValue(s.numGroups()));
        for (GraphTraits::vertex_descriptor vertex = first_group_vertex[group];
             vertex < first_group_vertex[group + 1]; ++vertex) {
          add_edge(vertex, first_participant + i, EdgeProperty(rating), g);
        }
      }
    }
  }

  // calculate the matching
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  maximum_weighted_matching(g, &mates[0]);
  std::cout << "Matching with size " << matching_size(g, &mates[0])
            << " and total weight " << matching_weight_sum(g, &mates[0])
            << " calculated ("
            << std::chrono::duration<double>(std::chrono::system_clock::now() - start).count()
            << " s)." << std::endl;

  // translate the matching to an assignment
  std::vector<int32_t> assignment(s.numParticipants(), -1);
  GraphTraits::vertex_iterator vi, vi_end;
  boost::tie(vi, vi_end) = vertices(g);

  bool success = true;
  ParticipantID part_idx = 0;
  for (vi += first_participant; vi != vi_end; ++vi) {
    ParticipantID part = participants.at(part_idx);
    GraphTraits::vertex_descriptor group_vertex = mates[*vi];
    if (group_vertex != GraphTraits::null_vertex()) {
      GroupID group = vertex_to_group[group_vertex];
      assignment[part] = group;
    } else {
      const std::string &name =
          s.isTeam(part) ? s.teamData(part).id : s.studentData(part).name;
      std::cerr << "WARNING: Participant \"" << name << "\" not assigned!"
                << std::endl;
      success = false;
    }
    ++part_idx;
  }
  return {std::move(assignment), success};
}

bool applyAssignment(State &s, const std::vector<int32_t> &assignment,
                     bool teams, bool students) {
  assert(s.numParticipants() == assignment.size());
  State s_temp(s);
  bool success = true;
  for (ParticipantID part = 0; part < s_temp.numParticipants(); ++part) {
    if (((teams && s_temp.isTeam(part)) ||
         (students && !s_temp.isTeam(part))) &&
        assignment[part] >= 0) {
      bool assign_success = s_temp.assignParticipant(part, assignment[part]);
      if (!assign_success) {
        if (VERBOSE) {
          std::cerr << "WARNING: Capacity of group \""
                    << s_temp.groupData(assignment[part]).name << "\" exceeded."
                    << std::endl;
        }
        success = false;
      }
    }
  }
  if (success) {
    s = s_temp;
  }
  return success;
}

std::vector<GroupID>
preassignLargeTeams(State &s, const std::vector<int32_t> &assignment) {
  // calculate statistics of the team
  std::vector<std::vector<ParticipantID>> teams_per_group(s.numGroups());
  std::vector<StudentID> max_size(s.numGroups(), 0);
  std::vector<StudentID> total_size(s.numGroups(), 0);
  for (ParticipantID part = 0; part < s.numParticipants(); ++part) {
    if (s.isTeam(part) && assignment[part] >= 0) {
      assert(!s.isAssigned(part));
      GroupID group = assignment[part];
      teams_per_group[group].push_back(part);
      StudentID team_size = s.teamData(part).size();
      max_size[group] = std::max(max_size[group], team_size);
      total_size[group] += team_size;
    }
  }

  // We use the previously calculated team assignment:
  // In each group, we fix the assignment of teams of maximum size.
  std::vector<GroupID> modified_groups;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    if (total_size[group] > s.groupCapacity(group)) {
      assert(teams_per_group[group].size() > 0);
      for (const ParticipantID &team : teams_per_group[group]) {
        if (s.teamData(team).size() == max_size[group]) {
          modified_groups.push_back(group);
          bool success = s.assignParticipant(team, group);
          if (success && VERBOSE) {
            std::cout << "> Preassign team \"" << s.teamData(team).id
                      << "\" to group \"" << s.groupData(group).name << "\"."
                      << std::endl;
          }
        }
      }
    }
  }
  return std::move(modified_groups);
}

bool assignTeamsAndStudents(State &s) {
  s.reset();
  StudentID num_students = s.data().students.size();
  StudentID activeCapacity = s.totalActiveGroupCapacity();
  StudentID initial_capacity = 0;
  assert(activeCapacity > num_students);
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    initial_capacity = std::max(initial_capacity, s.groupCapacity(group));
  }
  StudentID total_reduced = 0;

  bool success;
  do {
    StudentID additional_students_in_teams = 0;
    for (ParticipantID part = 0; part < s.numParticipants(); ++part) {
      if (s.isTeam(part) && !s.isAssigned(part)) {
        additional_students_in_teams += s.teamData(part).size() - 1;
      }
    }
    double team_factor =
        static_cast<double>(num_students - additional_students_in_teams) /
        static_cast<double>(num_students);
    double mod_reduced_factor =
        static_cast<double>(activeCapacity + total_reduced) /
        static_cast<double>(activeCapacity);
    double reduction_factor = team_factor * mod_reduced_factor;
    std::cout << "Relative capacity for team assignment set to " << reduction_factor << "."
              << std::endl;

    State s_temp(s);
    for (GroupID group = 0; group < s.numGroups(); ++group) {
      s_temp.setCapacity(group, ceil(reduction_factor * s_temp.groupCapacity(group)));
    }
    auto [assignment, success_first_step] = calculateAssignment(s_temp);
    if (!success_first_step) {
      std::cout << "Team assignment failed. Canceling." << std::endl;
      return false;
    }
    success = applyAssignment(s, assignment, true, false);
    if (!success) {
      std::cerr << "WARNING: Team assignment not successful due to exceeded "
                   "capacity. Assign single teams and retry."
                << std::endl;
      std::vector<GroupID> modified_groups = preassignLargeTeams(s, assignment);
      total_reduced += modified_groups.size();
    }
  } while (!success);

  std::cout << "Team assignment successful." << std::endl;
  auto [assignment, success_final] = calculateAssignment(s);
  success = applyAssignment(s, assignment) && success_final;
  return success;
}

// Top level function that calculates an assignment with a specified minimum capacity for the groups
void assignWithMinimumNumberPerGroup(State &s, StudentID min_capacity) {
  StudentID allowed_min = 1;
  StudentID active_capacity = s.totalActiveGroupCapacity();
  while (true) {
    const bool success = assignTeamsAndStudents(s);
    assert(success);
    StudentID current_min = std::numeric_limits<StudentID>::max();
    for (GroupID group = 0; group < s.numGroups(); ++group) {
      if (s.groupIsEnabled(group)) {
        current_min = std::min(current_min, s.groupSize(group));
      }
    }

    // Some groups are below the minimum capacity, therefore we disable these groups and retry.
    if (current_min < min_capacity) {
      allowed_min = std::max(allowed_min, current_min) + 1;
      std::cout << "Disabling groups with size smaller then " << allowed_min
                << "." << std::endl;
      std::vector<GroupID> groups_to_remove;
      for (GroupID group = 0; group < s.numGroups(); ++group) {
        if (s.groupIsEnabled(group) && s.groupSize(group) < allowed_min) {
          groups_to_remove.push_back(group);
        }
      }
      std::sort(groups_to_remove.begin(), groups_to_remove.end(),
                [&](GroupID g1, GroupID g2) {
                  return s.groupWeight(g1) < s.groupWeight(g2);
                });
      bool removed = false;
      for (GroupID group : groups_to_remove) {
        StudentID capacity = s.groupData(group).capacity;
        if (active_capacity - capacity >=
            ceil(CAPACITY_BUFFER * s.data().students.size())) {
          std::cout << "> Disable group \"" << s.groupData(group).name << "\" ("
                    << s.groupSize(group) << " participants)." << std::endl;
          s.disableGroup(group);
          active_capacity -= capacity;
          removed = true;
        }
      }
      if (!removed) {
        std::cout << "No further group could be removed. Stopping."
                  << std::endl;
        break;
      }
    } else {
      break;
    }
  }
}

// Top level function that add filters to reassign participants,
// so that a minimum number per group can be ensured
void assertMinimumNumberPerGroupForSpecificType(State &s,
    const std::vector<std::tuple<std::function<bool(const StudentData&)>, StudentID, std::string>> &filters) {
  std::cout << "Calculating reassignments to assert minimum number." << std::endl << std::endl;
  bool success = true;
  while (success) {
    std::vector<std::vector<std::pair<GroupID, StudentID>>> group_disable_order;
    size_t total_num_groups = 0;
    for (size_t i = 0; i < filters.size(); ++i) {
      auto [filter, minimum, _] = filters[i];
      std::vector<std::pair<GroupID, StudentID>> order = groupsByNumFiltered(s, minimum, filter);
      std::vector<std::pair<GroupID, StudentID>> rev_order;
      while (!order.empty()) {
        auto [group, num] = order.back();
        order.pop_back();
        // disable group without this type directly
        if (num == 0) {
          s.addFilterToGroup(group, filter);
        } else {
          rev_order.emplace_back(group, num);
        }
      }
      total_num_groups += rev_order.size();
      group_disable_order.push_back(std::move(rev_order));
    }

    if (total_num_groups == 0) {
      std::cout << "Successfully calculated reassignment!" << std::endl;
      break;
    }

    // disable groups for specific participants
    for (size_t _i = 0; _i < std::min(DISABLED_GROUPS_PER_STEP, (total_num_groups + 4) / 5); ++_i) {
      size_t max_index = 0;
      int32_t max_rating = std::numeric_limits<int32_t>::min();
      for (size_t j = 0; j < group_disable_order.size(); ++j) {
        if (!group_disable_order[j].empty()) {
          auto [group, num] = group_disable_order[j].back();
          // 2 times diff to minimum minus current number
          int32_t rating = 2 * (std::get<1>(filters[j]) - num) - num;
          if (rating > max_rating) {
            max_rating = rating;
            max_index = j;
          }
        }
      }
      auto [group, num] = group_disable_order[max_index].back();
      group_disable_order[max_index].pop_back();
      auto [filter, minimum, name] = filters[max_index];
      if (VERBOSE) {
        std::cout << "> Removing students of type \"" << name << "\" from group "
                  << s.groupData(group).name << " (" << num << " students)" << std::endl;
      }
      s.addFilterToGroup(group, filter);
    }

    // try to calculate new assignment
    State s_temp(s);
    success = assignTeamsAndStudents(s_temp);
    if (success) {
      s = s_temp;
    } else {
      std::cout << "WARNING: Could not continue reassignment. Stopping." << std::endl;
    }
  }
}

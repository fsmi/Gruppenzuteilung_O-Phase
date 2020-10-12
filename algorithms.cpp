#include <iostream>
#include <random>

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/maximum_weighted_matching.hpp>

#include "algorithms.h"

using EdgeProperty = boost::property<boost::edge_weight_t, uint32_t>;
using Graph = boost::adjacency_matrix<boost::undirectedS, boost::no_property,
                                      EdgeProperty>;
using GraphTraits = boost::graph_traits<Graph>;

// ####################################
// ########     Algorithms     ########
// ####################################

bool combinationIsValid(const StudentData &student, const GroupData &group) {
  return !(student.course_type != CourseType::Mathe &&
           group.course_type == CourseType::Mathe) &&
         !(student.degree_type != DegreeType::Master &&
           group.degree_type == DegreeType::Master);
}

bool combinationIsValid(const TeamData &team, const GroupData &group,
                        const std::vector<StudentData> &students) {
  // TODO
  return true;
}

double getFactor(const State &s, ParticipantID part) {
  // TODO
  return 1.0;
}

std::vector<int32_t> calculateAssignment(const State &s) {
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
  assert(first_participant >= num_vertices - first_participant);

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
      if (validTeam || validStudent) {
        uint32_t rating = ceil(factor * s.rating(part).at(group).getValue());
        for (GraphTraits::vertex_descriptor vertex = first_group_vertex[group];
             vertex < first_group_vertex[group + 1]; ++vertex) {
          add_edge(vertex, first_participant + i, EdgeProperty(rating), g);
        }
      }
    }
  }

  // calculate the matching
  maximum_weighted_matching(g, &mates[0]);
  std::cout << "Matching with size " << matching_size(g, &mates[0])
            << " and total weight " << matching_weight_sum(g, &mates[0])
            << " calculated." << std::endl;

  // translate the matching to an assignment
  std::vector<int32_t> assignment(s.numParticipants(), -1);
  GraphTraits::vertex_iterator vi, vi_end;
  boost::tie(vi, vi_end) = vertices(g);

  ParticipantID part_idx = 0;
  for (vi += first_participant; vi != vi_end; ++vi) {
    ParticipantID part = participants.at(part_idx);
    GraphTraits::vertex_descriptor group_vertex = mates[*vi];
    if (group_vertex != GraphTraits::null_vertex()) {
      GroupID group = vertex_to_group[group_vertex];
      assignment[part] = group;
    } else {
      const std::string &name =
          s.isTeam(part) ? s.teamData(part).name : s.studentData(part).name;
      std::cerr << "WARNING: Participant \"" << name << "\" not assigned!"
                << std::endl;
    }
    ++part_idx;
  }
  return std::move(assignment);
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
        std::cerr << "WARNING: Capacity of group \""
                  << s_temp.groupData(assignment[part]).name << "\" exceeded."
                  << std::endl;
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
  std::vector<GroupID> modified_groups;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    if (total_size[group] > s.groupCapacity(group)) {
      assert(teams_per_group[group].size() > 0);
      for (const ParticipantID &team : teams_per_group[group]) {
        if (s.teamData(team).size() == max_size[group]) {
          modified_groups.push_back(group);
          bool success = s.assignParticipant(team, group);
          if (success) {
            std::cout << "> Preassign team \"" << s.teamData(team).name
                      << "\" to group \"" << s.groupData(group).name << "\"."
                      << std::endl;
          }
        }
      }
    }
  }
  return std::move(modified_groups);
}

void assignTeamsAndStudents(State &s, StudentID initial_capacity,
                            bool fair_capacity) {
  s.resetWithCapacity(initial_capacity);
  StudentID capacity;
  if (fair_capacity) {
    StudentID num_students = s.data().students.size();
    StudentID additional_students_in_teams = 0;
    for (const TeamData &team : s.data().teams) {
      additional_students_in_teams += team.size() - 1;
    }
    capacity =
        ceil(static_cast<double>(num_students - additional_students_in_teams) /
             static_cast<double>(num_students) * initial_capacity);
    std::cout << "Capacity for team assignment set to " << capacity << "."
              << std::endl;
  } else {
    capacity = initial_capacity;
  }
  StudentID diff = initial_capacity - capacity;
  std::vector<StudentID> reduced_capacities(s.numGroups(), 0);

  bool success;
  do {
    State s_temp(s);
    for (GroupID group = 0; group < s.numGroups(); ++group) {
      s_temp.decreaseCapacity(group, diff + reduced_capacities[group]);
    }
    std::vector<int32_t> assignment = calculateAssignment(s_temp);
    success = applyAssignment(s, assignment, true, false);
    if (!success) {
      std::cerr << "WARNING: Team assignment not successful due to exceeded "
                   "capacity. Assign single teams and retry."
                << std::endl;
      std::vector<GroupID> modified_groups = preassignLargeTeams(s, assignment);
      for (const GroupID &modified : modified_groups) {
        reduced_capacities.at(modified)++;
      }
    }
  } while (!success);

  std::cout << "Team assignment successful." << std::endl;
  std::vector<int32_t> assignment = calculateAssignment(s);
  success = applyAssignment(s, assignment);
  assert(success);
}

void assignWithMinimumNumberPerGroup(State &s, StudentID min_capacity,
                                     StudentID initial_capacity,
                                     bool fair_capacity) {
  assert(min_capacity < initial_capacity);
  StudentID allowed_min = 1;
  while (true) {
    assignTeamsAndStudents(s, initial_capacity, fair_capacity);
    StudentID current_min = std::numeric_limits<StudentID>::max();
    for (GroupID group = 0; group < s.numGroups(); ++group) {
      if (s.groupIsEnabled(group)) {
        current_min = std::min(current_min, s.groupSize(group));
      }
    }

    if (current_min < min_capacity) {
      allowed_min = std::max(allowed_min, current_min);
      if (min_capacity - allowed_min > 7) {
        allowed_min += 3;
      } else if (min_capacity - allowed_min > 4) {
        allowed_min += 2;
      } else {
        allowed_min += 1;
      }
      std::cout << "Disabling groups with size smaller then " << allowed_min
                << "." << std::endl;
      for (GroupID group = 0; group < s.numGroups(); ++group) {
        if (s.groupSize(group) < allowed_min) {
          s.disableGroup(group);
          std::cout << "> Disable group \"" << s.groupData(group).name << "\"."
                    << std::endl;
        }
      }
    } else {
      break;
    }
  }
}

// ##########################################
// ########     Helper Functions     ########
// ##########################################

void printCurrentAssignment(const State &s) {
  std::cout << std::endl;
  for (GroupID group = 0; group < s.numGroups(); ++group) {
    const GroupData &gd = s.groupData(group);
    std::cout << gd.name << " (" << gd.main_group << "):" << std::endl;
    for (const StudentID &student : s.groupAssignmentList(group)) {
      const std::string &rating = s.data().ratings[student][group].getName();
      std::cout << "  - " << s.data().students[student].name << " [" << rating
                << "]" << std::endl;
    }
    std::cout << std::endl;
  }
}

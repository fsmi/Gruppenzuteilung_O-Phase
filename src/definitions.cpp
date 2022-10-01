#include "definitions.h"

#include <algorithm>
#include <assert.h>

#include "config.h"

// ####################################
// ########     Input Data     ########
// ####################################

Rating::Rating(uint32_t index) : index(index) { }

uint32_t Rating::getValue(GroupID num_groups) const {
  ASSERT(index < num_groups);
  return num_groups * num_groups - (index * (index + 1) / 2);
}

std::string Rating::getName() const { return std::to_string(index); }

bool Rating::operator==(const Rating &other) const {
  return index == other.index;
}

bool Rating::operator!=(const Rating &other) const {
  return index != other.index;
}

bool Rating::isValid() const {
  return index != std::numeric_limits<uint32_t>::max();
}

Rating Rating::minRating(GroupID num_groups) {
  return Rating(num_groups - 1);
}

GroupData::GroupData(std::string id, std::string name, StudentID capacity, CourseType ct, DegreeType dt)
    : id(id), name(name), capacity(capacity), course_type(ct), degree_type(dt) {}

StudentData::StudentData(std::string id, std::string name, CourseType ct, DegreeType dt, Semester s, bool ts)
    : id(id), name(name), course_type(ct), degree_type(dt), semester(s), type_specific_assignment(ts) {}

TeamData::TeamData(std::string id, std::vector<StudentID> members)
    : id(id), members(std::move(members)) {
      ASSERT_WITH(members.size() <= Config::get().max_team_size,
                  "team \"" << id << "\" exceeds maximum team size");
    }

size_t TeamData::size() const { return members.size(); }

// #########################################
// ########     Assignment Data     ########
// #########################################

Filter::Filter(std::vector<std::pair<FilterFn, u_int32_t>>&& filters, std::string&& name):
        filters(std::move(filters)), name(std::move(name)) {
  std::sort(filters.begin(), filters.end(), [](const auto& l, const auto& r) {
    return l.second < r.second;
  });
}

bool Filter::apply(const StudentData& data) const {
  for (const auto& [f, _]: filters) {
    if (!f(data)) {
      return false;
    }
  }
  return true;
}

u_int32_t Filter::id() const {
  u_int32_t result = 0;
  for (const auto& [_, id]: filters) {
    result *= 137;
    result += id;
    result += 13;
  }
  return result;
}

bool ratingsEqual(const std::vector<Rating> &r1,
                  const std::vector<Rating> &r2) {
  if (r1.size() != r2.size()) {
    return false;
  }

  for (size_t i = 0; i < r1.size(); ++i) {
    if (r1[i] != r2[i]) {
      return false;
    }
  }
  return true;
}

Participant::Participant(uint32_t index, bool is_team)
    : index(index), is_team(is_team), assignment(-1) {}

State::State(Input &data)
    : _data(data),
      _group_states(data.groups.size()),
      _group_assignments(data.groups.size(),
                         std::vector<std::pair<StudentID, ParticipantID>>()),
      _participants() {
  ASSERT(data.students.size() == data.ratings.size());
  std::vector<bool> is_in_team(data.students.size(), false);

  // sanitize ratings and check mapping to team id
  for (StudentID student = 0; student < data.ratings.size(); ++student) {
    for (Rating& rating: data.ratings[student]) {
      if (Config::get().allow_default_ratings && !rating.isValid()) {
        rating = Rating::minRating(data.groups.size());
      }
      ASSERT_WITH(rating.isValid(), "Invalid rating for student \"" << data.students[student].id << "\"");
    }
    const std::string& student_id = data.students[student].id;
    if ((Config::get().output_per_team || Config::get().input_per_team)
        && data.student_id_to_team_id.find(student_id) == data.student_id_to_team_id.end()) {
      FATAL_ERROR("Input/output per team requested. But student \"" << student_id << "\" is not member of a team.\n"
                  << "You either need to sanitize the input data or use --input-per-team=false/--output-per-team=false");
    }
  }
  // collect teams
  ParticipantID num_removed_teams = 0;
  for (ParticipantID team_id = 0; team_id < data.teams.size(); ++team_id) {
    const TeamData &team = data.teams[team_id];
    ASSERT_WITH(team.members.size() > 0, "team \"" << team.id << "\" has no member");
    if (team.members.size() > 1) {
      std::vector<Rating> team_rating;
      const StudentData* first_student_type_specific = nullptr;
      for (const StudentID &student : team.members) {
        ASSERT(student < is_in_team.size() && !is_in_team[student]);
        ASSERT_WITH(data.ratings[student].empty() || data.ratings[student].size() == data.groups.size(),
                    "student \"" << data.students[student].id << "\" has invalid rating");
        is_in_team[student] = true;
        if (!data.ratings[student].empty()) {
          ASSERT_WITH(team_rating.empty() || ratingsEqual(data.ratings[student], team_rating),
                      "conflicting ratings for team \"" << team.id << "\"");
          team_rating = data.ratings[student];
        }

        if (data.students[student].type_specific_assignment) {
          if (first_student_type_specific == nullptr) {
            first_student_type_specific = &data.students[student];
          } else if (first_student_type_specific->course_type != data.students[student].course_type
                     || first_student_type_specific->degree_type != data.students[student].degree_type
                     || first_student_type_specific->semester != data.students[student].semester) {
            data.students[student].type_specific_assignment = false;
            WARNING("Potential inconsistency in type specific assignment for team " << team.id << ":\n"
                    << "Disabling type specific assignment for member \"" << data.students[student].id <<"\".", false);
          }
        }
      }
      ASSERT_WITH(!team_rating.empty(), "no rating found for team \"" << team.id << "\"");
      for (const StudentID &student : team.members) {
        if (data.ratings[student].empty()) {
          data.ratings[student] = team_rating;
        }
      }
      _participants.emplace_back(team_id, true);
    } else {
      num_removed_teams++;
    }
  }
  if (num_removed_teams > 0) {
    if (Config::get().output_per_team) {
      MAJOR_TRACE("Removed " << num_removed_teams << " teams with size 1.", true);
    } else {
      WARNING("Removed " << num_removed_teams << " teams with size 1.", true);
    }
  }
  // collect students without team
  for (size_t i = 0; i < data.students.size(); ++i) {
    if (!is_in_team[i]) {
      _participants.emplace_back(i, false);
    }
  }
  // group capacities
  for (size_t i = 0; i < data.groups.size(); ++i) {
    ASSERT_WITH(data.groups[i].capacity < Config::get().max_group_size,
                "group \"" << data.groups[i].name << "\" has invalid capacity");
    _group_states[i].capacity = data.groups[i].capacity;
  }
  ASSERT(totalActiveGroupCapacity() >= ceil(Config::get().capacity_buffer * data.students.size()));

  // check total participant count
  StudentID total_count = 0;
  for (ParticipantID p = 0; p < numParticipants(); ++p) {
    if (isTeam(p)) {
      total_count += teamData(p).size();
    } else {
      ++total_count;
    }
  }
  ASSERT_WITH(data.students.size() == total_count,
              "Internal error: Student count (" << data.students.size()
              << ") does not match mapped count (" << total_count << ").");
}

const Input &State::data() const { return _data.get(); }

GroupID State::numGroups() const { return data().groups.size(); }

GroupID State::numActiveGroups() const {
  GroupID num = 0;
  for (GroupState state : _group_states) {
    if (state.enabled) {
      num++;
    }
  }
  return num;
}

StudentID State::totalActiveGroupCapacity() const {
  StudentID cap = 0;
  for (GroupID group = 0; group < numGroups(); ++group) {
    if (groupIsEnabled(group)) {
      cap += groupCapacity(group);
    }
  }
  return cap;
}

const GroupData &State::groupData(GroupID id) const {
  ASSERT(id < data().groups.size());
  return data().groups[id];
}

StudentID State::groupCapacity(GroupID id) const {
  ASSERT(id < data().groups.size());
  return _group_states[id].capacity;
}

bool State::groupIsEnabled(GroupID id) const {
  ASSERT(id < data().groups.size());
  return _group_states[id].enabled;
}

const std::vector<std::pair<StudentID, ParticipantID>> &
State::groupAssignmentList(GroupID id) const {
  ASSERT(id < data().groups.size());
  return _group_assignments[id];
}

StudentID State::groupSize(GroupID id) const {
  ASSERT(id < data().groups.size());
  return _group_assignments[id].size();
}

uint32_t State::groupWeight(GroupID id) const {
  // TODO: assert correctness?!
  ASSERT(id < data().groups.size());
  return _group_states[id].weight;
}

ParticipantID State::numParticipants() const { return _participants.size(); }

bool State::isTeam(ParticipantID id) const {
  ASSERT(id < _participants.size());
  return _participants[id].is_team;
}

bool State::isAssigned(ParticipantID id) const {
  ASSERT(id < _participants.size());
  return _participants[id].assignment >= 0;
}

GroupID State::getAssignment(ParticipantID id) const {
  ASSERT(isAssigned(id) && _participants[id].assignment >= 0);
  return _participants[id].assignment;
}

const StudentData &State::studentData(ParticipantID id) const {
  ASSERT(!isTeam(id));
  StudentID student_id = _participants[id].index;
  ASSERT(student_id < data().students.size());
  return data().students[student_id];
}

const TeamData &State::teamData(ParticipantID id) const {
  ASSERT(isTeam(id));
  uint32_t team_id = _participants[id].index;
  ASSERT(team_id < data().teams.size());
  return data().teams[team_id];
}

GroupID State::assignment(ParticipantID id) const {
  ASSERT(isAssigned(id));
  return static_cast<GroupID>(_participants[id].assignment);
}

const std::vector<Rating> &State::rating(ParticipantID id) const {
  StudentID student_id;
  if (isTeam(id)) {
    student_id = teamData(id).members[0];
  } else {
    student_id = _participants[id].index;
  }
  return data().ratings[student_id];
}

void State::disableGroup(GroupID id) {
  ASSERT(id < data().groups.size());
  _group_states[id].enabled = false;
}

void State::addFilterToGroup(GroupID id, Filter filter) {
  ASSERT(id < data().groups.size());
  if (!groupContainsFilter(id, filter)) {
    _group_states[id].participant_filters.push_back(filter);
  }
}

bool State::groupContainsFilter(GroupID id, const Filter& filter) const {
  ASSERT(id < data().groups.size());
  for (const Filter& f: _group_states[id].participant_filters) {
    if (filter.id() == f.id()) {
      return true;
    }
  }
  return false;
}

bool State::studentIsExludedFromGroup(StudentID student, GroupID group) const {
  ASSERT(group < data().groups.size());
  const StudentData& s_data = data().students[student];
  if (!s_data.type_specific_assignment) {
    return false;
  }
  for (auto filter : _group_states[group].participant_filters) {
    if (filter.apply(s_data)) {
      return true;
    }
  }
  return false;
}

bool State::isExludedFromGroup(ParticipantID participant, GroupID group) const {
  ASSERT(participant < _participants.size());
  ASSERT(group < data().groups.size());

  if (isTeam(participant)) {
    // TODO: this might not really work for inhomogenous teams
    for (StudentID student : teamData(participant).members) {
      if (studentIsExludedFromGroup(student, group)) {
        return true;
      }
    }
    return false;
  } else {
    return studentIsExludedFromGroup(_participants[participant].index, group);
  }
}

bool State::assignParticipant(ParticipantID participant, GroupID target) {
  ASSERT(!isAssigned(participant));
  ASSERT(target < data().groups.size());

  if (isTeam(participant)) {
    const TeamData &data = teamData(participant);
    if (data.size() > groupCapacity(target)) {
      return false;
    }
    _group_states[target].capacity -= data.size();
    for (StudentID id : data.members) {
      _group_assignments[target].push_back(std::make_pair(id, participant));
    }
    _group_states[target].weight +=
        data.size() * rating(participant)[target].getValue(numGroups());
  } else {
    if (groupCapacity(target) == 0) {
      return false;
    }
    _group_states[target].capacity--;
    _group_assignments[target].push_back(
        std::make_pair(_participants[participant].index, participant));
    _group_states[target].weight += rating(participant)[target].getValue(numGroups());
  }
  _participants[participant].assignment = target;
  return true;
}

void State::unassignParticipant(ParticipantID participant, GroupID group) {
  ASSERT(isAssigned(participant));
  ASSERT(assignment(participant) == group);

  std::vector<std::pair<StudentID, ParticipantID>>& assign_list =
      _group_assignments[group];

  // because a participant might be a team, we need to remove all members in a loop
  while (true) {
    auto to_remove = std::find_if(
        assign_list.begin(), assign_list.end(),
        [&](const auto &pair) { return pair.second == participant; });
    if (to_remove != assign_list.end()) {
      _group_states[group].capacity++;
      _group_states[group].weight -= rating(participant)[group].getValue(numGroups());
      assign_list.erase(to_remove);
    } else {
      break;
    }
  }
  _participants[participant].assignment = -1;
}

// groups are still disabled and keep their filters
void State::reset() {
  for (GroupID group = 0; group < numGroups(); ++group) {
    GroupState& state = _group_states[group];
    state.capacity = groupData(group).capacity;
    state.weight = 0;
  }
  for (auto &assigned : _group_assignments) {
    assigned.clear();
  }
  for (Participant &part : _participants) {
    part.assignment = -1;
  }
}

void State::setCapacity(GroupID id, uint32_t val) {
  ASSERT(id < data().groups.size() && val < Config::get().max_group_size);
  _group_states[id].capacity = val;
}

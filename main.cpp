#include <iostream>
#include <string>
#include <vector>

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/maximum_weighted_matching.hpp>

using EdgeProperty = boost::property<boost::edge_weight_t, uint32_t>;
using Graph = boost::adjacency_matrix<boost::undirectedS, boost::no_property,
                                      EdgeProperty>;

static const uint32_t INITIAL_GROUP_CAPACITY = 18;
static const size_t NUM_RATINGS = 5;
static uint32_t RATING_VAL_TABLE[NUM_RATINGS] = {100, 120, 150, 180, 200};
static const char *RATING_NAME_TABLE[NUM_RATINGS] = {"--", "-", "O", "+", "++"};

// ####################################
// ########     Input Data     ########
// ####################################

class Rating {
  uint32_t _index;

public:
  Rating(uint32_t index) : _index(index) { assert(index < NUM_RATINGS); }

  uint32_t getValue() const { return RATING_VAL_TABLE[_index]; }

  const char *getName() const { return RATING_NAME_TABLE[_index]; }
};

using GroupID = uint32_t;
using StudentID = uint32_t;
using ParticipantID = uint32_t;

enum class CourseType {
  Info,
  Mathe,
  Any,
};

enum class DegreeType {
  Bachelor,
  Master,
  Any,
};

struct GroupData {
  std::string name;
  std::string main_group;
  CourseType course_type;
  DegreeType degree_type;
};

struct StudentData {
  std::string name;
  CourseType course_type;
  DegreeType degree_type;
  bool is_commuter;
};

struct TeamData {
  std::string name;
  std::vector<StudentID> members;

  size_t size() const { return members.size(); }
};

struct Input {
  std::vector<GroupData> groups;
  std::vector<StudentData> students;
  std::vector<TeamData> teams;
  std::vector<std::vector<Rating>> ratings;
};

int main(int argc, const char *argv[]) {
}

#pragma once

#include <stdint.h>
#include <iostream>
#include <sys/ioctl.h>

#include "boost/program_options.hpp"

#include "definitions.h"

namespace po = boost::program_options;

int getTerminalWidth();

enum class RatingInputType {
  Mapping,
  OrderedList
};

// holds all config options in a singleton
class Config {
 public:
  uint32_t verbosity_level = 3;

  // input and output
  RatingInputType rating_input_type = RatingInputType::Mapping;
  bool output_per_team = false;
  bool input_per_team = false;

  // algorithm
  GroupID disabled_groups_per_step = 3;
  uint32_t type_specific_assignment_treshold = 0;
  StudentID min_group_size = 10;
  StudentID max_group_size = 200;
  StudentID max_team_size = 5;
  bool allow_default_ratings = false;
  double capacity_buffer = 1.05;

  static const Config& get() {
    return get_mut();
  }

  static po::options_description getConfigOptions();

  static void check();

 private:
  // use only for initialization
  static Config& get_mut() {
    static Config instance;
    return instance;
  }
};

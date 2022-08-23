#pragma once

#include <stdint.h>
#include <iostream>

#include "boost/program_options.hpp"

#include "definitions.h"

namespace po = boost::program_options;

// holds all config options in a singleton
class Config {
 public:
  uint32_t verbosity_level = 1;
  GroupID disabled_groups_per_step = 1; // higher value will make the algorithm faster but quality worse
  StudentID min_group_size = 10; // smaller groups will be disabled
  StudentID max_group_size = 200; // ensure that groups are not larger than this
  double capacity_buffer = 1.05;
  int64_t search_steps_per_value = 1000;

  // TODO: Max team size

  static const Config& get() {
    return get_mut();
  }

  static po::options_description getConfigOptions();

 private:
  // use only for initialization
  static Config& get_mut() {
    static Config instance;
    return instance;
  }
};

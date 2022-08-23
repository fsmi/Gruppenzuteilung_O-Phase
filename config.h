#pragma once

#include <stdint.h>
#include <iostream>
#include <sys/ioctl.h>

#include "boost/program_options.hpp"

#include "definitions.h"

namespace po = boost::program_options;

int getTerminalWidth();

// holds all config options in a singleton
class Config {
 public:
  uint32_t verbosity_level = 2;
  GroupID disabled_groups_per_step = 3;
  StudentID min_group_size = 10;
  StudentID max_group_size = 200;
  double capacity_buffer = 1.05;
  int64_t search_steps_per_value = 1000;

  // TODO: enable move-based reassignment
  // TODO: Max team size

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

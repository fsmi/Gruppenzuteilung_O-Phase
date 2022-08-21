#pragma once

#include <stdint.h>
#include <iostream>

#include "boost/program_options.hpp"

namespace po = boost::program_options;

// holds all config options in a singleton
class Config {
 public:
  uint32_t verbosity_level = 1;

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

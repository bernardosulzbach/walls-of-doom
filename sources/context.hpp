#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "profiler.hpp"
#include "random.hpp"
#include "settings.hpp"

class Context {
public:
  Settings &settings;
  Profiler &profiler;
  RandomNumberGenerator &random_number_generator;
  Context(Settings &settings, Profiler &profiler, RandomNumberGenerator &generator);
};

#endif

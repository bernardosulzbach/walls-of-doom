#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "profiler.hpp"
#include "settings.hpp"

class Context {
public:
  Settings &settings;
  Profiler &profiler;
  Context(Settings &settings, Profiler &profiler);
};

#endif

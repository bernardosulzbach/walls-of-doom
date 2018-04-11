#include "context.hpp"

Context::Context(Settings &settings, Profiler &profiler, RandomNumberGenerator &generator)
    : settings(settings), profiler(profiler), random_number_generator(generator) {
}

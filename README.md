Walls of Doom
=============

# About

Walls of Doom (WoD) is a minimalistic
[platformer](https://en.wikipedia.org/wiki/Platform_game) written in C by
Bernardo Sulzbach and Leonardo Ferrazza.

See [the Bitbucket issue tracker](https://bitbucket.org/mafagafogigante/walls-of-doom/issues?status=new&status=open&sort=-priority).

# Continuous Integration Status

## Semaphore CI

This build server uses Ubuntu 14.04 64-bit and compiles with GCC 4.8.4.

[![Build Status](https://semaphoreci.com/api/v1/projects/7d008ba4-1535-427c-bfcb-f6dc9a53b963/828137/shields_badge.svg)](https://semaphoreci.com/mafagafogigante/walls-of-doom)

# Building and Running

## Terminal

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Running the tests

```bash
$ ./autotest/autotest
```

### Running the game

```bash
$ ./game/walls-of-doom
```

## Code::Blocks

```bash
$ cmake . -G"CodeBlocks - Unix Makefiles"
```

### Running the tests

+ Open Code::Blocks
+ Open the Code::Blocks project file on the root directory
+ Select `autotest` as the target
+ Press "Build and run"

### Running the game

+ Open Code::Blocks
+ Open the Code::Blocks project file on the root directory
+ Select `walls-of-doom` as the target
+ Press "Build and run"

# Generating the Images

```bash
$ cd blender
$ bash render.sh
```

And they should be written in to the output subdirectory.

# Implementation Notes

Before using the I/O capabilities of WoD, initialize() must be called.
Before quitting the game, finalize() must be called to free associated
resources.

Similarly, before using the logging capabilities, initialize_logger() should be
called and finalize_logger() should be called after quitting. Note, however,
that if the program makes use of the I/O capabilities, the calls to initialize()
and finalize() will also properly initialize and finalize the logger, so
handling logger initialization and finalization is only required if the I/O
initialization and finalization are not performed.

## **insertion-sort.h**

+ Our generic insertion sort implementation

We only need to keep track of the five best scores. Therefore, insertion sort is
a good enough algorithm for our requirements.

## **random.h**

We are using the **[xoroshiro+](http://xoroshiro.di.unimi.it/)** algorithm to
efficiently generate pseudo-random numbers with a big period.

The convenience function that returns an integer in the specified range uses
multiple random numbers modulo the next power of two to prevent the modulo bias
that comes with more naive approaches.

# Links

[Imgur album of the project](http://imgur.com/a/kiOY2).

# License

It is licensed under the BSD 3-Clause license. See LICENSE.txt for more
information.

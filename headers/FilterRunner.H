#ifndef FILTER_RUNNER_H
#define FILTER_RUNNER_H
#include "Execution.H"
using Executor = int (*)(const Filter&, const Image&, Image&, int);
int runFilter(int argc, char* argv[], const char* mode, Executor execute,
              bool openmpOptions = false);
#endif

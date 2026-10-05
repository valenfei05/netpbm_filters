#ifndef EXECUTION_H
#define EXECUTION_H
#include "filter.H"

struct Region { int xBegin, xEnd, yBegin, yEnd; };
// Devuelven el numero de trabajadores usados.
int applySequential(const Filter&, const Image&, Image&, int requestedThreads);
int applyThreaded(const Filter&, const Image&, Image&, int requestedThreads);
int applyOpenMP(const Filter&, const Image&, Image&, int requestedThreads);
void splitQuadrants(int width, int height, Region (&regions)[4]);
#endif

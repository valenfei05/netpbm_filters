#ifndef FILTER_H
#define FILTER_H

#include "Image.h"

// Filtro que transforma una imagen de entrada en una imagen de salida.
class Filter {
public:
    virtual ~Filter();
    virtual void apply(const Image& input, Image& output) const = 0;
};

// Suavizado: promedia cada pixel con sus vecinos (3x3).
class BlurFilter : public Filter {
public:
    void apply(const Image& input, Image& output) const;
};

// Deteccion de bordes: resalta zonas donde el valor cambia bruscamente.
class LaplaceFilter : public Filter {
public:
    void apply(const Image& input, Image& output) const;
};

// Realce: acentua los bordes sobre la imagen original.
class SharpenFilter : public Filter {
public:
    void apply(const Image& input, Image& output) const;
};

#endif
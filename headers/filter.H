#ifndef FILTER_H
#define FILTER_H

#include "Image.h"

// Filtro que transforma una imagen de entrada en una imagen de salida
class Filter {
public:
    virtual ~Filter();
    virtual void apply(const Image& input, Image& output) const = 0;
};

// Filtro de convolucion 3x3, aplicado canal por canal (sirve para PGM y PPM).
class ConvolutionFilter : public Filter {
protected:
    float kernel[3][3];
    bool normalize; // si es true, divide por la suma de pesos (util en bordes)

public:
    ConvolutionFilter(const float newKernel[3][3], bool shouldNormalize);
    void apply(const Image& input, Image& output) const;
};

/// Suavizado: promedia cada pixel con sus vecinos.
class BlurFilter : public ConvolutionFilter {
public:
    BlurFilter();
};

/// Deteccion de bordes (segunda derivada).
class LaplaceFilter : public ConvolutionFilter {
public:
    LaplaceFilter();
};

/// Realce: acentua los bordes sobre la imagen original.
class SharpenFilter : public ConvolutionFilter {
public:
    SharpenFilter();
};

#endif
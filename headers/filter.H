#ifndef FILTER_H
#define FILTER_H

#include "Image.H"

// Filtro que transforma una imagen de entrada en una imagen de salida.
class Filter {
public:
    virtual ~Filter();
    virtual void apply(const Image& input, Image& output) const;

    // Reserva una salida independiente antes de iniciar los trabajadores.
    static void prepareOutput(const Image& input, Image& output);

    // Intervalos semiabiertos. La salida ya debe estar reservada.
    // Se puede leer TODA input; solo se escribe el rectangulo asignado.
    virtual void applyRegion(const Image& input, Image& output,
                             int xBegin, int xEnd, int yBegin, int yEnd) const = 0;
};

// Suavizado: promedia cada pixel con sus vecinos (3x3).
class BlurFilter : public Filter {
public:
    void applyRegion(const Image& input, Image& output,
                     int xBegin, int xEnd, int yBegin, int yEnd) const override;
};

// Deteccion de bordes: resalta zonas donde el valor cambia bruscamente.
class LaplaceFilter : public Filter {
public:
    void applyRegion(const Image& input, Image& output,
                     int xBegin, int xEnd, int yBegin, int yEnd) const override;
};

// Realce: acentua los bordes sobre la imagen original.
class SharpenFilter : public Filter {
public:
    void applyRegion(const Image& input, Image& output,
                     int xBegin, int xEnd, int yBegin, int yEnd) const override;
};

#endif
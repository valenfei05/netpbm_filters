#ifndef IMAGE_H
#define IMAGE_H

#include <cstdio>


class Image {
protected:
    char magicNumber[3]; // "P2" o "P3"
    int width;
    int height;
    int maxColor;
    int channels;        // 1 = PGM, 3 = PPM
    int* pixels;

    // Avanza el archivo saltando espacios, saltos de linea y comentarios 
    static int skipWhitespaceAndComments(FILE* file);

    // Lee el siguiente entero del archivo (ignora comentarios/espacios antes de el).
    static int readNextInt(FILE* file);

public:
    Image();
    virtual ~Image();

    //Carga la imagen desde 'path'. Retorna false si no se pudo abrir o leer.
    bool loadFromFile(const char* path);

    // Guarda la imagen en 'path'. Retorna false si no se pudo crear el archivo.
    bool saveToFile(const char* path) const;

    //Reserva memoria para una imagen nueva (usado por los filtros).
    void allocate(int newWidth, int newHeight, int newChannels, int newMaxColor, const char* newMagicNumber);

    int getWidth() const;
    int getHeight() const;
    int getMaxColor() const;
    int getChannels() const;
    int getPixelCount() const;

    int* getPixels();
    const int* getPixels() const;
    const char* getMagicNumber() const;

    //Lee/escribe un pixel por posicion (x, y) y canal.
    int getPixelAt(int x, int y, int channel) const;
    void setPixelAt(int x, int y, int channel, int value);
};

#endif
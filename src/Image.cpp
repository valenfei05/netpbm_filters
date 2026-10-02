#include "../headers/Image.h"
#include <iostream>

using namespace std;

Image::Image() {
    magicNumber[0] = '\0';
    magicNumber[1] = '\0';
    magicNumber[2] = '\0';
    width = 0;
    height = 0;
    maxColor = 0;
    channels = 0;
    pixels = NULL;
}

Image::~Image() {
    if (pixels != NULL) {
        delete[] pixels;
    }
}

// Salta espacios/saltos de linea/comentarios y devuelve el siguiente caracter util.
int Image::skipWhitespaceAndComments(FILE* file) {
    int c = fgetc(file);
    while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '#') {
        if (c == '#') {
            while (c != '\n' && c != EOF) {
                c = fgetc(file);
            }
        }
        c = fgetc(file);
    }
    return c;
}

// Lee el siguiente entero, ignorando comentarios y espacios antes de el.
int Image::readNextInt(FILE* file) {
    int c = skipWhitespaceAndComments(file);
    int sign = 1;
    if (c == '-') {
        sign = -1;
        c = fgetc(file);
    }

    int value = 0;
    while (c >= '0' && c <= '9') {
        value = value * 10 + (c - '0');
        c = fgetc(file);
    }

    if (c != EOF) {
        ungetc(c, file);
    }

    return value * sign;
}

// Carga cabecera (magic, ancho, alto, maxColor) y los pixeles desde 'path'.
bool Image::loadFromFile(const char* path) {
    bool success = true;

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        cout << "Error: no se pudo abrir " << path << endl;
        success = false;
    }

    if (success) {
        magicNumber[0] = static_cast<char>(fgetc(file));
        magicNumber[1] = static_cast<char>(fgetc(file));
        magicNumber[2] = '\0';

        if (magicNumber[0] != 'P' || (magicNumber[1] != '2' && magicNumber[1] != '3')) {
            cout << "Error: numero magico no soportado en " << path << endl;
            success = false;
        }
    }

    if (success) {
        channels = (magicNumber[1] == '3') ? 3 : 1;
        width = readNextInt(file);
        height = readNextInt(file);
        maxColor = readNextInt(file);

        if (width <= 0 || height <= 0) {
            cout << "Error: dimensiones invalidas en " << path << endl;
            success = false;
        }
    }

    if (success) {
        pixels = new int[getPixelCount()];
        int i = 0;
        while (i < getPixelCount()) {
            pixels[i] = readNextInt(file);
            i++;
        }
    }

    if (file != NULL) {
        fclose(file);
    }

    return success;
}

// Escribe cabecera y pixeles en 'path', en formato Netpbm plano (ASCII).
bool Image::saveToFile(const char* path) const {
    bool success = true;

    FILE* output = fopen(path, "w");
    if (output == NULL) {
        cout << "Error: no se pudo crear " << path << endl;
        success = false;
    }

    if (success) {
        fprintf(output, "%s\n%d %d\n%d\n", magicNumber, width, height, maxColor);
        int i = 0;
        while (i < getPixelCount()) {
            fprintf(output, "%d\n", pixels[i]);
            i++;
        }
        fclose(output);
    }

    return success;
}

// Reserva un buffer nuevo de pixeles (usado por los filtros para la imagen de salida).
void Image::allocate(int newWidth, int newHeight, int newChannels, int newMaxColor, const char* newMagicNumber) {
    if (pixels != NULL) {
        delete[] pixels;
    }

    width = newWidth;
    height = newHeight;
    channels = newChannels;
    maxColor = newMaxColor;
    magicNumber[0] = newMagicNumber[0];
    magicNumber[1] = newMagicNumber[1];
    magicNumber[2] = '\0';

    pixels = new int[getPixelCount()];
}

int Image::getWidth() const { return width; }
int Image::getHeight() const { return height; }
int Image::getMaxColor() const { return maxColor; }
int Image::getChannels() const { return channels; }
int Image::getPixelCount() const { return width * height * channels; }

int* Image::getPixels() { return pixels; }
const int* Image::getPixels() const { return pixels; }
const char* Image::getMagicNumber() const { return magicNumber; }

// index = (y*width + x)*channels + canal
int Image::getPixelAt(int x, int y, int channel) const {
    return pixels[(y * width + x) * channels + channel];
}

void Image::setPixelAt(int x, int y, int channel, int value) {
    pixels[(y * width + x) * channels + channel] = value;
}
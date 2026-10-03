#include "../headers/Filter.h"

using namespace std;

// Lee un pixel de 'image' en (x, y, canal). Si esta fuera de los limites, retorna 0.
int safeGetPixel(const Image& image, int x, int y, int channel) {
    int value = 0;
    if (x >= 0 && x < image.getWidth() && y >= 0 && y < image.getHeight()) {
        value = image.getPixelAt(x, y, channel);
    }
    return value;
}

// Fuerza 'value' a quedar entre 0 y maxColor.
int clampValue(int value, int maxColor) {
    int result = value;
    if (result < 0) {
        result = 0;
    }
    if (result > maxColor) {
        result = maxColor;
    }
    return result;
}

Filter::~Filter() {}

// Suavizado: cada pixel se reemplaza por el promedio de sus vecinos. En los bordes de la imagen hay menos vecinos,
// por eso se divide por 'count' y no siempre por 9.
void BlurFilter::apply(const Image& input, Image& output) const {
    int width = input.getWidth();
    int height = input.getHeight();
    int channels = input.getChannels();
    int maxColor = input.getMaxColor();

    output.allocate(width, height, channels, maxColor, input.getMagicNumber());

    int y = 0;
    while (y < height) {
        int x = 0;
        while (x < width) {
            int c = 0;
            while (c < channels) {
                int sum = 0;
                int count = 0;

                int dy = -1;
                while (dy <= 1) {
                    int dx = -1;
                    while (dx <= 1) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            sum += input.getPixelAt(nx, ny, c);
                            count++;
                        }
                        dx++;
                    }
                    dy++;
                }

                int average = sum / count;
                output.setPixelAt(x, y, c, clampValue(average, maxColor));
                c++;
            }
            x++;
        }
        y++;
    }
}

// Deteccion de bordes: compara el pixel central con sus 8 vecinos.
void LaplaceFilter::apply(const Image& input, Image& output) const {
    int width = input.getWidth();
    int height = input.getHeight();
    int channels = input.getChannels();
    int maxColor = input.getMaxColor();

    output.allocate(width, height, channels, maxColor, input.getMagicNumber());

    int y = 0;
    while (y < height) {
        int x = 0;
        while (x < width) {
            int c = 0;
            while (c < channels) {
                int center    = input.getPixelAt(x, y, c);
                int top       = safeGetPixel(input, x,     y - 1, c);
                int bottom    = safeGetPixel(input, x,     y + 1, c);
                int left      = safeGetPixel(input, x - 1, y,     c);
                int right     = safeGetPixel(input, x + 1, y,     c);
                int topLeft   = safeGetPixel(input, x - 1, y - 1, c);
                int topRight  = safeGetPixel(input, x + 1, y - 1, c);
                int botLeft   = safeGetPixel(input, x - 1, y + 1, c);
                int botRight  = safeGetPixel(input, x + 1, y + 1, c);

                int neighborSum = top + bottom + left + right + topLeft + topRight + botLeft + botRight;
                int value = 8 * center - neighborSum;
                output.setPixelAt(x, y, c, clampValue(value, maxColor));
                c++;
            }
            x++;
        }
        y++;
    }
}

// Realce: es como laplace, pero sumado sobre la imagen original. El resultado se parece a la imagen de entrada, pero con los bordes
// mas marcados.
void SharpenFilter::apply(const Image& input, Image& output) const {
    int width = input.getWidth();
    int height = input.getHeight();
    int channels = input.getChannels();
    int maxColor = input.getMaxColor();

    output.allocate(width, height, channels, maxColor, input.getMagicNumber());

    int y = 0;
    while (y < height) {
        int x = 0;
        while (x < width) {
            int c = 0;
            while (c < channels) {
                int center = input.getPixelAt(x, y, c);
                int top    = safeGetPixel(input, x, y - 1, c);
                int bottom = safeGetPixel(input, x, y + 1, c);
                int left   = safeGetPixel(input, x - 1, y, c);
                int right  = safeGetPixel(input, x + 1, y, c);

                int value = 5 * center - top - bottom - left - right;
                output.setPixelAt(x, y, c, clampValue(value, maxColor));
                c++;
            }
            x++;
        }
        y++;
    }
}
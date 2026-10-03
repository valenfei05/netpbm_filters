#include "../headers/Filter.h"
#include <algorithm>

using namespace std;

static const float blurKernel[3][3] = {
    {1.0 / 9, 1.0 / 9, 1.0 / 9},
    {1.0 / 9, 1.0 / 9, 1.0 / 9},
    {1.0 / 9, 1.0 / 9, 1.0 / 9}
};

static const float laplaceKernel[3][3] = {
    { 0, -1,  0},
    {-1,  4, -1},
    { 0, -1,  0}
};

static const float sharpenKernel[3][3] = {
    { 0, -1,  0},
    {-1,  5, -1},
    { 0, -1,  0}
};

Filter::~Filter() {}

ConvolutionFilter::ConvolutionFilter(const float newKernel[3][3], bool shouldNormalize) {
    normalize = shouldNormalize;
    int y = 0;
    while (y < 3) {
        int x = 0;
        while (x < 3) {
            kernel[y][x] = newKernel[y][x];
            x++;
        }
        y++;
    }
}

// Convolucion 3x3 generica: recorre cada pixel y canal, combina los vecinos
// segun el kernel, y escribe el resultado (acotado a [0, maxColor]) en 'output'.
void ConvolutionFilter::apply(const Image& input, Image& output) const {
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
                float sum = 0.0f;
                float weightSum = 0.0f;

                int ky = -1;
                while (ky <= 1) {
                    int kx = -1;
                    while (kx <= 1) {
                        int nx = x + kx;
                        int ny = y + ky;
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            float weight = kernel[ky + 1][kx + 1];
                            sum += input.getPixelAt(nx, ny, c) * weight;
                            weightSum += weight;
                        }
                        kx++;
                    }
                    ky++;
                }

                float divisor = (normalize && weightSum != 0.0f) ? weightSum : 1.0f;
                int result = static_cast<int>(sum / divisor);
                result = max(0, min(maxColor, result));

                output.setPixelAt(x, y, c, result);
                c++;
            }
            x++;
        }
        y++;
    }
}

BlurFilter::BlurFilter() : ConvolutionFilter(blurKernel, true) {}
LaplaceFilter::LaplaceFilter() : ConvolutionFilter(laplaceKernel, false) {}
SharpenFilter::SharpenFilter() : ConvolutionFilter(sharpenKernel, false) {}
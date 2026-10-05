#include <iostream>
#include "../headers/Image.H"

using namespace std;

// Lee una imagen PGM/PPM y la vuelve a guardar (base para los demas disenos).
int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "missing input and output paths" << endl;
        cout << "usage: " << argv[0] << " input_image.pgm output_image.pgm" << endl;
        cout << "or " << argv[0] << " input_image.ppm output_image.ppm" << endl;
        return 1;
    }

    Image image;
    int result = 1;

    bool loaded = image.loadFromFile(argv[1]);
    if (loaded) {
        bool saved = image.saveToFile(argv[2]);
        if (saved) {
            cout << "Imagen procesada: " << image.getWidth() << "x" << image.getHeight();
            cout << " (" << (image.getChannels() == 3 ? "PPM" : "PGM") << ")" << endl;
            result = 0;
        }
    }

    return result;
}
#include <iostream>
#include <cstring>
#include <ctime>
#include <chrono>
#include "../headers/Image.h"
#include "../headers/Filter.h"

using namespace std;

// Crea el filtro pedido por nombre, o NULL si el nombre no es reconocido.
Filter* createFilter(const char* name) {
    Filter* filter = NULL;
    if (strcmp(name, "blur") == 0) {
        filter = new BlurFilter();
    }
    if (strcmp(name, "laplace") == 0) {
        filter = new LaplaceFilter();
    }
    if (strcmp(name, "sharpen") == 0) {
        filter = new SharpenFilter();
    }
    return filter;
}

int main(int argc, char* argv[]) {
    if (argc < 5 || strcmp(argv[3], "--f") != 0) {
        cout << "usage: " << argv[0] << " input_image output_image --f <blur|laplace|sharpen>" << endl;
        return 1;
    }

    Filter* filter = createFilter(argv[4]);
    if (filter == NULL) {
        cout << "Error: filtro desconocido '" << argv[4] << "'" << endl;
        return 1;
    }

    int result = 1;
    Image input;
    bool loaded = input.loadFromFile(argv[1]);

    if (loaded) {
        Image output;

        clock_t cpuStart = clock();
        auto wallStart = chrono::high_resolution_clock::now();

        filter->apply(input, output);

        clock_t cpuEnd = clock();
        auto wallEnd = chrono::high_resolution_clock::now();

        bool saved = output.saveToFile(argv[2]);
        if (saved) {
            double cpuSeconds = double(cpuEnd - cpuStart) / CLOCKS_PER_SEC;
            chrono::duration<double> wallSeconds = wallEnd - wallStart;

            cout << "Filtro: " << argv[4] << endl;
            cout << "Tiempo CPU: " << cpuSeconds << " s" << endl;
            cout << "Tiempo total: " << wallSeconds.count() << " s" << endl;
            result = 0;
        }
    }

    delete filter;
    return result;
}
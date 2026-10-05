#include "../headers/FilterRunner.H"
#include "../headers/Timer.H"
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>

namespace {
void usage(const char* program, bool openmpOptions) {
    std::cout << "Uso: " << program << " entrada salida"
              << (openmpOptions ? " [--f blur|laplace|sharpen]" : " --f blur|laplace|sharpen");
    if (openmpOptions) std::cout << " [--threads N]";
    std::cout << "\n";
    if (openmpOptions) std::cout << "OpenMP: filtro predeterminado blur; hilos predeterminados 4.\n";
}
bool positiveInteger(const char* text, int& result) {
    char* end = nullptr;
    errno = 0;
    const long value = std::strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' || value <= 0 || value > INT_MAX) return false;
    result = static_cast<int>(value);
    return true;
}
Filter* createFilter(const char* name) {
    if (std::strcmp(name, "blur") == 0) return new BlurFilter();
    if (std::strcmp(name, "laplace") == 0) return new LaplaceFilter();
    if (std::strcmp(name, "sharpen") == 0) return new SharpenFilter();
    return nullptr;
}
}
int runFilter(int argc, char* argv[], const char* mode, Executor execute, bool openmpOptions) {
    if (argc == 2 && std::strcmp(argv[1], "--help") == 0) {
        usage(argv[0], openmpOptions); return 0;
    }
    if (argc < 3) { usage(argv[0], openmpOptions); return 1; }
    const char* filterName = openmpOptions ? "blur" : nullptr;
    int requestedThreads = openmpOptions ? 4 : 1;
    bool seenFilter = false, seenThreads = false;
    for (int i = 3; i < argc; ++i) {
        if (std::strcmp(argv[i], "--f") == 0 && !seenFilter && i + 1 < argc) {
            filterName = argv[++i]; seenFilter = true;
        } else if (openmpOptions && std::strcmp(argv[i], "--threads") == 0 &&
                   !seenThreads && i + 1 < argc) {
            seenThreads = true;
            if (!positiveInteger(argv[++i], requestedThreads)) {
                std::cerr << "Error: --threads requiere un entero positivo.\n"; return 1;
            }
        } else {
            std::cerr << "Error: argumento incompleto, repetido o desconocido: " << argv[i] << "\n";
            usage(argv[0], openmpOptions); return 1;
        }
    }
    if (!filterName) { usage(argv[0], openmpOptions); return 1; }
    try {
        std::unique_ptr<Filter> filter(createFilter(filterName));
        if (!filter) { std::cerr << "Error: filtro desconocido: " << filterName << "\n"; return 1; }
        const Timer totalTimer; // Carga + reserva + calculo + guardado.
        Image input;
        if (!input.loadFromFile(argv[1])) return 1;
        Image output;
        const Timer filterTimer;
        const int actualThreads = execute(*filter, input, output, requestedThreads);
        const Elapsed filtering = filterTimer.elapsed();
        if (!output.saveToFile(argv[2])) return 1;
        const Elapsed total = totalTimer.elapsed();
        std::cout << std::fixed << std::setprecision(9)
                  << "Modo: " << mode << "\nFiltro: " << filterName
                  << "\nImagen: " << input.getWidth() << "x" << input.getHeight()
                  << "\nCanales: " << input.getChannels() << "\nHilos: " << actualThreads;
        if (openmpOptions) std::cout << "\nHilos solicitados: " << requestedThreads;
        std::cout << "\nCPU filtrado (s): " << filtering.cpu
                  << "\nWall filtrado (s): " << filtering.wall
                  << "\nCPU total (s): " << total.cpu
                  << "\nWall total (s): " << total.wall << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n"; return 1;
    }
}

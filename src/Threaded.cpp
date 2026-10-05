#include "../headers/Execution.H"
#include <exception>
#include <thread>

void splitQuadrants(int width, int height, Region (&regions)[4]) {
    const int midX = width / 2, midY = height / 2;
    regions[0] = {0, midX, 0, midY};
    regions[1] = {midX, width, 0, midY};
    regions[2] = {0, midX, midY, height};
    regions[3] = {midX, width, midY, height};
}
int applyThreaded(const Filter& filter, const Image& input, Image& output, int) {
    Filter::prepareOutput(input, output);
    Region regions[4];
    splitQuadrants(input.getWidth(), input.getHeight(), regions);
    std::thread workers[4];
    std::exception_ptr failures[4] = {};
    try {
        for (int i = 0; i < 4; ++i) {
            // i por valor: cada hilo conserva su indice aunque avance el bucle.
            workers[i] = std::thread([&, i]() {
                try {
                    const Region& r = regions[i];
                    filter.applyRegion(input, output, r.xBegin, r.xEnd, r.yBegin, r.yEnd);
                } catch (...) { failures[i] = std::current_exception(); }
            });
        }
    } catch (...) {
        // Ante fallo de creacion, espera a los trabajadores ya iniciados.
        for (auto& worker : workers) if (worker.joinable()) worker.join();
        throw;
    }
    for (auto& worker : workers) worker.join();
    for (const auto& failure : failures) if (failure) std::rethrow_exception(failure);
    return 4;
}

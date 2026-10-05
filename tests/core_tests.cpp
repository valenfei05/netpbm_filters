#include "../headers/Execution.H"
#include <iostream>
#include <stdexcept>

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
class FailingFilter : public Filter {
    void applyRegion(const Image&, Image&, int, int, int, int) const override {
        throw std::runtime_error("Fallo simulado del trabajador");
    }
};
int main() {
    try {
        for (int width = 1; width <= 9; ++width) {
            for (int height = 1; height <= 9; ++height) {
                Region regions[4];
                splitQuadrants(width, height, regions);
                int visits[81] = {};
                for (const auto& r : regions) {
                    require(r.xBegin >= 0 && r.xEnd <= width && r.xBegin <= r.xEnd,
                            "Cuadrante fuera de ancho");
                    require(r.yBegin >= 0 && r.yEnd <= height && r.yBegin <= r.yEnd,
                            "Cuadrante fuera de alto");
                    for (int y = r.yBegin; y < r.yEnd; ++y)
                        for (int x = r.xBegin; x < r.xEnd; ++x) ++visits[y * width + x];
                }
                for (int i = 0; i < width * height; ++i)
                    require(visits[i] == 1, "Pixel omitido o compartido entre escritores");
            }
        }
        Image input, output;
        input.allocate(3, 3, 1, 255, "P2");
        for (int i = 0; i < 9; ++i) input.getPixels()[i] = i * 20;
        BlurFilter blur;
        for (int mode = 0; mode < 3; ++mode) {
            bool rejected = false;
            try {
                if (mode == 0) blur.apply(input, input);
                if (mode == 1) applyThreaded(blur, input, input, 4);
                if (mode == 2) applyOpenMP(blur, input, input, 4);
            } catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "Se permitio filtrar sobre la misma imagen");
        }
        applyThreaded(blur, input, output, 4);
        applyOpenMP(blur, input, output, 4);
        for (int i = 0; i < 9; ++i)
            require(input.getPixels()[i] == i * 20, "Se modifico la entrada");
        FailingFilter failure;
        for (int mode = 0; mode < 2; ++mode) {
            bool propagated = false;
            try {
                if (mode == 0) applyThreaded(failure, input, output, 4);
                else applyOpenMP(failure, input, output, 4);
            } catch (const std::runtime_error&) { propagated = true; }
            require(propagated, "Se perdio el error del trabajador");
        }
        std::cout << "OK: 81 particiones, entrada inmutable, alias rechazado y excepciones.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}

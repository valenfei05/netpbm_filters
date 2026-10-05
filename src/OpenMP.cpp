#include "../headers/Execution.H"
#include <exception>
#include <omp.h>
#include <stdexcept>
#ifndef _OPENMP
#error "Compila OpenMP.cpp con -fopenmp (o equivalente)"
#endif

int applyOpenMP(const Filter& filter, const Image& input, Image& output, int requestedThreads) {
    if (requestedThreads <= 0) throw std::invalid_argument("Numero de hilos invalido");
    Filter::prepareOutput(input, output);
    const int width = input.getWidth(), height = input.getHeight();
    int actualThreads = 0;
    std::exception_ptr failure;
    omp_set_dynamic(0);
    #pragma omp parallel num_threads(requestedThreads) default(none) \
        shared(filter, input, output, actualThreads, failure) firstprivate(width, height)
    {
        #pragma omp single
        actualThreads = omp_get_num_threads();
        #pragma omp for schedule(static)
        for (int y = 0; y < height; ++y) {
            try {
                filter.applyRegion(input, output, 0, width, y, y + 1);
            } catch (...) {
                // Sincronizacion solo en caso de excepcion, nunca por pixel.
                #pragma omp critical(filter_failure)
                {
                    if (!failure) failure = std::current_exception();
                }
            }
        }
    } // Barrera: salida completa antes del guardado.
    if (failure) std::rethrow_exception(failure);
    return actualThreads;
}

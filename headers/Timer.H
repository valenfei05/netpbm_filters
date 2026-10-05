#ifndef TIMER_H
#define TIMER_H
#include <chrono>
#include <stdexcept>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <time.h>
#endif

// CPU acumulada de TODOS los hilos del proceso, sin tiempos de espera.
inline double processCpuSeconds() {
#ifdef _WIN32
    FILETIME creation, exit, kernel, user;
    if (!GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user))
        throw std::runtime_error("No fue posible medir CPU con GetProcessTimes");
    ULARGE_INTEGER k, u;
    k.LowPart = kernel.dwLowDateTime;
    k.HighPart = kernel.dwHighDateTime;
    u.LowPart = user.dwLowDateTime;
    u.HighPart = user.dwHighDateTime;
    return static_cast<double>(k.QuadPart + u.QuadPart) * 1e-7;
#else
    timespec value;
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &value) != 0)
        throw std::runtime_error("No fue posible medir CPU con clock_gettime");
    return static_cast<double>(value.tv_sec) + value.tv_nsec * 1e-9;
#endif
}
struct Elapsed { double cpu; double wall; };
class Timer {
    double cpuStart;
    std::chrono::steady_clock::time_point wallStart;
public:
    Timer() : cpuStart(processCpuSeconds()), wallStart(std::chrono::steady_clock::now()) {}
    Elapsed elapsed() const {
        const auto wallEnd = std::chrono::steady_clock::now();
        const double cpuEnd = processCpuSeconds();
        return {cpuEnd - cpuStart,
                std::chrono::duration<double>(wallEnd - wallStart).count()};
    }
};
#endif

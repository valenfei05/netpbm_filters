#include "../headers/Execution.H"
int applySequential(const Filter& filter, const Image& input, Image& output, int) {
    filter.apply(input, output);
    return 1;
}

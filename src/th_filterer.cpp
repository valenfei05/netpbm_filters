#include "../headers/FilterRunner.H"
int main(int argc, char* argv[]) {
    return runFilter(argc, argv, "threads", applyThreaded);
}

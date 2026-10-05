"""Compila con GCC sin necesitar Make/CMake: python scripts/build.py."""
import argparse
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"))
    parser.add_argument("--debug", action="store_true")
    parser.add_argument("--bin-dir", type=Path, default=ROOT / "build")
    args = parser.parse_args()
    args.bin_dir.mkdir(parents=True, exist_ok=True)
    suffix = ".exe" if os.name == "nt" else ""
    common = ["src/Image.cpp", "src/filter.cpp", "src/FilterRunner.cpp"]
    targets = [
        ("processor", ["src/Image.cpp", "src/processor.cpp"], []),
        ("filterer", common + ["src/Sequential.cpp", "src/filterer.cpp"], []),
        ("th_filterer", common + ["src/Threaded.cpp", "src/th_filterer.cpp"], ["-pthread"]),
        ("omp_filterer", common + ["src/OpenMP.cpp", "src/omp_filterer.cpp"], ["-fopenmp"]),
        ("core_tests", ["src/Image.cpp", "src/filter.cpp", "src/Threaded.cpp",
                        "src/OpenMP.cpp", "tests/core_tests.cpp"], ["-pthread", "-fopenmp"]),
    ]
    flags = ["-std=c++17", "-Wall", "-Wextra", "-Wpedantic"]
    flags += ["-O0", "-g"] if args.debug else ["-O2"]
    print(subprocess.check_output([args.cxx, "--version"], text=True).splitlines()[0], flush=True)
    for name, sources, extra in targets:
        # MinGW puede corromper las tildes al pasar una ruta absoluta al linker.
        destination = os.path.relpath(args.bin_dir.resolve() / (name + suffix), ROOT)
        command = [args.cxx, *flags, *sources, *extra, "-o", destination]
        print("Compilando", name, flush=True)
        subprocess.run(command, cwd=ROOT, check=True)
    print("Compilacion completa.", flush=True)

if __name__ == "__main__":
    main()

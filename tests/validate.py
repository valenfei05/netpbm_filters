"""Valida resultados independientes, bordes, CLI y las imagenes exigidas."""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FILTERS = ("blur", "laplace", "sharpen")

def read_image(path):
    tokens = []
    for line in path.read_text().splitlines():
        tokens.extend(line.split("#", 1)[0].split())
    magic, width, height, maximum = tokens[:4]
    width, height, maximum = int(width), int(height), int(maximum)
    channels = 3 if magic == "P3" else 1
    samples = list(map(int, tokens[4:]))
    assert len(samples) == width * height * channels, path
    assert all(0 <= x <= maximum for x in samples), path
    return magic, width, height, maximum, samples

def reference(data, width, height, channels, maximum, name):
    # Oraculo independiente, expresado como vecindades ponderadas.
    weights = {
        "laplace": [(-1, -1, -1), (0, -1, -1), (1, -1, -1),
                    (-1, 0, -1), (0, 0, 8), (1, 0, -1),
                    (-1, 1, -1), (0, 1, -1), (1, 1, -1)],
        "sharpen": [(0, 0, 5), (-1, 0, -1), (1, 0, -1), (0, -1, -1), (0, 1, -1)],
        "blur": [(x, y, 1) for y in (-1, 0, 1) for x in (-1, 0, 1)],
    }[name]
    result = []
    for y in range(height):
        for x in range(width):
            for c in range(channels):
                terms = [data[((y + dy) * width + x + dx) * channels + c] * weight
                         for dx, dy, weight in weights
                         if 0 <= x + dx < width and 0 <= y + dy < height]
                value = sum(terms)
                if name == "blur":
                    value //= len(terms)
                result.append(max(0, min(maximum, value)))
    return result

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin-dir", type=Path, default=ROOT / "build")
    parser.add_argument("--baseline", type=Path, help="Ejecutable original del Diseno 2 (opcional)")
    args = parser.parse_args()
    suffix = ".exe" if os.name == "nt" else ""
    def exe(name): return str((args.bin_dir / (name + suffix)).resolve())
    def run(name, *options, success=True):
        p = subprocess.run([exe(name), *map(str, options)], capture_output=True, text=True, timeout=120)
        if success:
            assert p.returncode == 0, p.stdout + p.stderr
        else:
            assert p.returncode != 0, options
        return p.stdout

    checks = 0
    with tempfile.TemporaryDirectory(prefix="netpbm_validation_") as directory:
        tmp = Path(directory)
        source, output = tmp / "input.pnm", tmp / "output.pnm"
        configurations = [("filterer", []), ("th_filterer", [])]
        configurations += [("omp_filterer", ["--threads", n]) for n in (1, 2, 4, 8)]
        for width, height in [(1, 1), (1, 7), (7, 1), (2, 2), (3, 3), (5, 7), (8, 6)]:
            for channels in (1, 3):
                for maximum in (255, 65535):
                    magic = "P3" if channels == 3 else "P2"
                    data = [((i * 7919) ^ (i * i * 17)) % (maximum + 1)
                            for i in range(width * height * channels)]
                    # Bordes y centro brillantes fuerzan saturacion y cruces de cuadrantes.
                    data[len(data) // 2] = maximum
                    source.write_text(f"{magic}\n# prueba con comentario\n{width} {height}\n{maximum}\n"
                                      + " ".join(map(str, data)) + "\n")
                    for name in FILTERS:
                        expected = reference(data, width, height, channels, maximum, name)
                        for binary, extra in configurations:
                            run(binary, source, output, "--f", name, *extra)
                            actual = read_image(output)
                            assert actual == (magic, width, height, maximum, expected), (binary, name, width, height)
                            checks += 1
        # Valores pequenos comprobables a mano: blur en esquina conserva solo 4 vecinos.
        source.write_text("P2\n2 2\n255\n0 10 20 30\n")
        run("filterer", source, output, "--f", "blur")
        assert read_image(output)[4] == [15] * 4
        run("omp_filterer", source, output)  # El ejemplo del MP2 sin --f usa blur.
        assert read_image(output)[4] == [15] * 4
        for binary in ("filterer", "th_filterer", "omp_filterer"):
            run(binary, "--help")
            run(binary, source, output, "--f", "desconocido", success=False)
            run(binary, source, output, "--f", success=False)
            run(binary, source, output, "--f", "blur", "--f", "laplace", success=False)
            run(binary, tmp / "missing.pgm", output, "--f", "blur", success=False)
            run(binary, source, tmp / "missing" / "out.pgm", "--f", "blur", success=False)
        for count in ("0", "-1", "2x", "999999999999999999999999"):
            run("omp_filterer", source, output, "--threads", count, success=False)

        # Comparacion byte a byte (mismo escritor) sobre damma/sulfur PGM y PPM.
        for image in ("damma.pgm", "damma.ppm", "sulfur.pgm", "sulfur.ppm"):
            source = ROOT / "images" / image
            for name in FILTERS:
                run("filterer", source, output, "--f", name)
                expected_hash = hashlib.sha256(output.read_bytes()).hexdigest()
                for binary in ("th_filterer", "omp_filterer"):
                    run(binary, source, output, "--f", name)
                    assert hashlib.sha256(output.read_bytes()).hexdigest() == expected_hash, (image, name, binary)
                    checks += 1
                if args.baseline:
                    p = subprocess.run([str(args.baseline.resolve()), str(source), str(output), "--f", name],
                                       capture_output=True, text=True, timeout=120)
                    assert p.returncode == 0, p.stderr
                    assert hashlib.sha256(output.read_bytes()).hexdigest() == expected_hash, ("Diseno 2", image, name)
                    checks += 1
            print("OK:", image, flush=True)
    print(f"OK: {checks} comparaciones de imagen, mas CLI, default OpenMP y caso manual.")

if __name__ == "__main__":
    main()

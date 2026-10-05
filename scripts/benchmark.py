"""Mide damma/sulfur, verifica igualdad y genera CSV e informe reproducible."""
import argparse
import csv
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
LABELS = {"CPU filtrado (s)": "cpu_filter_s", "Wall filtrado (s)": "wall_filter_s",
          "CPU total (s)": "cpu_total_s", "Wall total (s)": "wall_total_s"}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bin-dir", type=Path, default=ROOT / "build")
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--omp-threads", type=int, nargs="+", default=[1, 2, 4, 8])
    parser.add_argument("--results", type=Path, default=ROOT / "results")
    args = parser.parse_args()
    if args.repeats < 2 or any(n <= 0 for n in args.omp_threads):
        parser.error("Usa al menos 2 repeticiones e hilos positivos")
    args.results.mkdir(parents=True, exist_ok=True)
    suffix = ".exe" if os.name == "nt" else ""
    modes = [("secuencial", "filterer", 1), ("threads", "th_filterer", 4)]
    modes += [("openmp", "omp_filterer", n) for n in sorted(set(args.omp_threads))]
    cases = [(image, f, mode, binary, n)
             for image in ("damma.pgm", "damma.ppm", "sulfur.pgm", "sulfur.ppm")
             for f in ("blur", "laplace", "sharpen") for mode, binary, n in modes]
    fields = ["image", "filter", "mode", "requested_threads", "actual_threads",
              "repeat", *LABELS.values(), "sha256"]
    rows, expected = [], {}
    metadata = {
        "started_utc": datetime.now(timezone.utc).isoformat(),
        "system": platform.platform(), "processor": platform.processor(),
        "logical_cpus": os.cpu_count(), "repeats": args.repeats,
        "warmups_per_case": 1, "omp_threads": args.omp_threads,
        "compiler": subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
        "build_expected": "python scripts/build.py: C++17, -O2, sin LTO; mismo kernel",
        "clock": "GetProcessTimes" if os.name == "nt" else "CLOCK_PROCESS_CPUTIME_ID",
        "wall_clock": "std::chrono::steady_clock",
        "filter_scope": "reserva de salida + creacion/equipo de hilos + calculo + espera",
        "total_scope": "carga + filtrado + guardado; no incluye arranque del proceso",
        "scheduling": "1 calentamiento por caso; orden aleatorio reproducible en cada ronda",
    }
    try:
        metadata["revision"] = subprocess.check_output(["git", "rev-parse", "HEAD"],
                                                       cwd=ROOT, text=True).strip()
        metadata["worktree_changes"] = subprocess.check_output(
            ["git", "status", "--short"], cwd=ROOT, text=True)
    except subprocess.CalledProcessError:
        pass
    with tempfile.TemporaryDirectory(prefix="netpbm_benchmark_") as directory:
        out = Path(directory) / "result.pnm"
        def measure(case, repeat):
            image, f, mode, binary, threads = case
            command = [str((args.bin_dir / (binary + suffix)).resolve()),
                       "images/" + image, str(out), "--f", f]
            if mode == "openmp":
                command += ["--threads", str(threads)]
            p = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                               check=True, timeout=180)
            values = dict(line.split(": ", 1) for line in p.stdout.splitlines() if ": " in line)
            row = dict(image=image, filter=f, mode=mode, requested_threads=threads,
                       actual_threads=int(values["Hilos"]), repeat=repeat)
            row.update({key: float(values[label]) for label, key in LABELS.items()})
            row["sha256"] = hashlib.sha256(out.read_bytes()).hexdigest()
            key = (image, f)
            if key not in expected:
                assert mode == "secuencial"
                expected[key] = row["sha256"]
            assert expected[key] == row["sha256"], ("Imagen diferente", case)
            return row
        for i, case in enumerate(cases, 1):
            measure(case, 0)
            if i % 12 == 0: print(f"Calentamiento {i}/{len(cases)}", flush=True)
        rng = random.Random(20261004)
        for repeat in range(1, args.repeats + 1):
            shuffled = cases[:]
            rng.shuffle(shuffled)
            for case in shuffled: rows.append(measure(case, repeat))
            with (args.results / "timings.csv").open("w", newline="", encoding="utf-8") as file:
                writer = csv.DictWriter(file, fields)
                writer.writeheader()
                writer.writerows(rows)
            print(f"Ronda {repeat}/{args.repeats}: {len(rows)} mediciones, todas identicas.", flush=True)
    metadata["finished_utc"] = datetime.now(timezone.utc).isoformat()
    (args.results / "environment.json").write_text(json.dumps(metadata, indent=2, ensure_ascii=False),
                                                 encoding="utf-8")
    summaries = []
    for image, f, mode, binary, threads in cases:
        group = [r for r in rows if (r["image"], r["filter"], r["mode"], r["requested_threads"])
                 == (image, f, mode, threads)]
        summary = dict(image=image, filter=f, mode=mode, requested_threads=threads,
                       actual_threads=group[0]["actual_threads"])
        assert len({r["actual_threads"] for r in group}) == 1, "Equipo variable: revisar entorno"
        summary.update({key: statistics.median(r[key] for r in group) for key in LABELS.values()})
        summary["wall_filter_min_s"] = min(r["wall_filter_s"] for r in group)
        summary["wall_filter_max_s"] = max(r["wall_filter_s"] for r in group)
        summaries.append(summary)
    for row in summaries:
        seq = next(r for r in summaries if r["image"] == row["image"] and
                   r["filter"] == row["filter"] and r["mode"] == "secuencial")
        row["speedup_filter"] = seq["wall_filter_s"] / row["wall_filter_s"]
        row["efficiency_filter"] = row["speedup_filter"] / row["actual_threads"]
        row["speedup_total"] = seq["wall_total_s"] / row["wall_total_s"]
    with (args.results / "summary.csv").open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, summaries[0].keys())
        writer.writeheader()
        writer.writerows(summaries)
    report = [
        "# Resultados del Diseno 3", "",
        f"Inicio UTC: {metadata['started_utc']}.",
        f"Equipo: {metadata['system']}; {metadata['logical_cpus']} procesadores logicos.",
        f"Compilador: {metadata['compiler']}.", "",
        f"Medianas de {args.repeats} ejecuciones independientes, despues de un calentamiento por caso.",
        "Cada ejecucion carga la imagen original. Todos los hashes coinciden con el secuencial.",
        "CPU suma el tiempo consumido por todos los hilos; wall mide tiempo transcurrido.",
        "Filtrado incluye reserva de salida, creacion de trabajadores y espera.",
        "Total incluye lectura y escritura, pero no el arranque del proceso.",
        "Las mediciones son locales, con otros procesos y caches del sistema; no prueban un speedup universal.",
        "En Windows el contador CPU puede tener una resolucion gruesa para trabajos cortos.", "",
        "| Imagen | Filtro | Modo | Hilos reales | CPU filtro ms | Wall filtro ms | CPU total ms | Wall total ms | Speedup filtro | Eficiencia | Speedup total |",
        "|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for r in summaries:
        report.append(f"| {r['image']} | {r['filter']} | {r['mode']} | {r['actual_threads']} | "
                      f"{r['cpu_filter_s']*1000:.3f} | {r['wall_filter_s']*1000:.3f} | "
                      f"{r['cpu_total_s']*1000:.3f} | {r['wall_total_s']*1000:.3f} | "
                      f"{r['speedup_filter']:.2f} | {r['efficiency_filter']*100:.1f}% | "
                      f"{r['speedup_total']:.2f} |")
    report += ["", "Datos crudos: timings.csv. Minimos/maximos y medianas: summary.csv.",
               "Entorno y metodologia: environment.json.", ""]
    (args.results / "RESULTADOS.md").write_text("\n".join(report), encoding="utf-8")
    print("Informe:", args.results / "RESULTADOS.md", flush=True)

if __name__ == "__main__":
    main()

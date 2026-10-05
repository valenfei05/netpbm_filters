# MP2 - Filtros PGM/PPM en C++

Implementaciones secuencial (Diseño 2) y de memoria compartida (Diseño 3), sobre la misma lógica de filtros.

- `filterer`: un hilo, referencia secuencial.
- `th_filterer`: cuatro cuadrantes con `std::thread`.
- `omp_filterer`: filas con OpenMP, cuatro hilos por defecto.
- `processor`: carga y guarda una imagen (Diseño 1).

Se admiten PGM P2 y PPM P3 de texto, con arreglos de píxeles. Filtros: `blur`, `laplace`, `sharpen`.

## Compilar en tu equipo Windows

Desde la raíz de este repositorio, con Python 3 y GCC/MinGW con OpenMP en PATH:

```powershell
python scripts/build.py
```

En este equipo se usó GCC 15.2.0 de MSYS2 UCRT64. Si hace falta configurar el PATH en una nueva terminal:

```powershell
$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
python scripts/build.py
```

El script usa C++17, `-O2`, advertencias y los indicadores `-pthread`/`-fopenmp` donde corresponden.
El directorio `build/` contiene los ejecutables. No se requiere Make ni CMake para esta ruta.
En Linux también se usa `python3 scripts/build.py`; los ejecutables no llevan `.exe`.

## Ejecutar

```powershell
New-Item -ItemType Directory -Force output | Out-Null
./build/filterer.exe images/damma.pgm output/damma_seq_blur.pgm --f blur
./build/th_filterer.exe images/damma.pgm output/damma_th_blur.pgm --f blur
./build/omp_filterer.exe images/damma.pgm output/damma_omp_blur.pgm --f blur --threads 4
./build/th_filterer.exe images/sulfur.ppm output/sulfur_th_laplace.ppm --f laplace
./build/omp_filterer.exe images/sulfur.ppm output/sulfur_omp_sharpen.ppm --f sharpen --threads 4
```

`th_filterer` siempre crea cuatro hilos, uno por cuadrante. `--threads` solo se admite en OpenMP.
Se aplica un filtro por invocación. Para comparar filtros se usa siempre la misma imagen original.
La salida conserva P2/P3 y el máximo de color de la entrada; la extensión no convierte formatos.

El ejemplo OpenMP del enunciado omite `--f`. Se admite esa forma y se definió explícitamente **blur** como filtro predeterminado:

```powershell
./build/omp_filterer.exe images/sulfur.pgm output/sulfur_N.pgm
```

Los tres programas aceptan `--help` e imprimen CPU/wall para filtrado y para carga + filtrado + guardado.

## Verificar

```powershell
python tests/validate.py
```

Comprueba 528 resultados: un cálculo independiente para imágenes pequeñas, todos los filtros y distintas cantidades de hilos, más comparación exacta sobre damma/sulfur PGM/PPM. También prueba argumentos inválidos y rutas ausentes.

Hay pruebas C++ adicionales de cobertura de cuadrantes, entrada inmutable y errores:

```powershell
./build/core_tests.exe
```

En la máquina de desarrollo, Control de aplicaciones de Windows bloqueó este último ejecutable y la copia del programa antiguo (error 4551). Las pruebas de imágenes sobre los tres ejecutables de filtrado sí corrieron. No se desactivó esa protección. Véase la sección de validación de la guía para el alcance exacto.

## Medir rendimiento

```powershell
python scripts/benchmark.py
```

Un calentamiento y cinco repeticiones por caso; los casos se mezclan en cada ronda. Prueba damma y sulfur en los dos formatos, los tres filtros y OpenMP con 1/2/4/8 hilos. Verifica la igualdad de cada salida antes de aceptar su tiempo.

- [Interpretación de los resultados](results/ANALISIS.md)
- [Resultados medidos](results/RESULTADOS.md)
- [Medianas y speedup](results/summary.csv)
- [Tiempos individuales](results/timings.csv)
- [Entorno de ejecución](results/environment.json)

Los scripts de pruebas y medición usan archivos temporales y no sobrescriben las imágenes originales.

## Entender el Diseño 3

Lee [la explicación completa](docs/DISENO_3.md): arquitectura, fórmulas, bordes, referencias compartidas, límites de cuadrantes, `join`, OpenMP, relojes, pruebas y respuestas para el informe del MP2.

## Alternativa con CMake

Se proporciona configuración para sistemas con CMake y un compilador con OpenMP:

```bash
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-cmake --config Release
ctest --test-dir build-cmake -C Release --output-on-failure
```

La ruta efectivamente compilada y medida en esta máquina fue `scripts/build.py` con GCC; CMake no estaba instalado.

## Referencias del curso

MP2_2026 (1).pdf, CL9a_21102026.pdf, TC1_28092026.pdf y CL08aprog_14092026.html.
Repositorio base indicado por el curso: https://github.com/japeto/netpbm_filters

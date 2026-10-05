# filterer - referencia secuencial

Desde la raíz del repositorio:

```powershell
python scripts/build.py
New-Item -ItemType Directory -Force output | Out-Null
./build/filterer.exe images/lena.pgm output/lena_blur.pgm --f blur
```

Filtros disponibles: blur, laplace, sharpen. Admite PGM P2 y PPM P3.
En Linux usa `python3` y omite `.exe`.
El cálculo es el mismo del Diseño 2, reorganizado para compartir regiones con los ejecutores paralelos.
Ver `../docs/DISENO_3.md` y `../README.md` para los puntos de medición y las variantes paralelas.

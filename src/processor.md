# processor - Diseño 1

Desde la raíz del repositorio:

```powershell
python scripts/build.py
New-Item -ItemType Directory -Force output | Out-Null
./build/processor.exe images/lena.ppm output/lena_copia.ppm
```

En Linux usa `python3` y omite `.exe`. El programa conserva el formato P2/P3 de la entrada.
Para aplicar filtros y comparar las variantes del Diseño 3, consulta `../README.md`.

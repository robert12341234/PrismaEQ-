# Prisma EQ - como obtener el plugin (sin compilar nada en tu PC)

GitHub compila el plugin por ti en sus servidores (gratis). Solo subes los archivos y descargas el resultado.

## Paso a paso

1. Crea una cuenta gratuita en https://github.com (Sign up).
2. Arriba a la derecha pulsa "+" y luego "New repository". Ponle de nombre `PrismaEQ` y pulsa "Create repository".
3. En la pagina del repositorio pulsa "uploading an existing file".
4. Descomprime este ZIP en tu PC. Abre la carpeta y arrastra TODO su contenido (las carpetas `Source` y `.github`, y los archivos sueltos) a la pagina de GitHub. Espera a que termine de subir.
5. Abajo pulsa "Commit changes".
6. Ve a la pestana "Actions". Veras una tarea "Compilar plugin" en marcha. Tarda entre 5 y 10 minutos.
7. Cuando aparezca con una palomita verde, entra en ella. Abajo, en "Artifacts", descarga `PrismaEQ-Windows-VST3`.
8. Descomprime la descarga. Copia la carpeta `Prisma EQ.vst3` a:
   `C:\Program Files\Common Files\VST3`
9. Abre tu DAW y vuelve a escanear plugins (Reaper: Preferences > Plug-ins > VST > Re-scan).

Tambien se genera `PrismaEQ-Windows-Standalone`: un programa independiente para probar el plugin sin abrir un DAW.

## Si la tarea sale en rojo

Entra en la tarea fallida, copia el texto del error (las lineas que dicen "error") y mandaselo a Claude para que lo corrija.

## Si la carpeta .github no se subio

En el repositorio pulsa "Add file" > "Create new file", escribe el nombre `.github/workflows/build.yml` y pega el contenido del archivo `.github/workflows/build.yml` de este ZIP.

## Que hace el plugin

- 6 bandas: tipo (campana, shelf grave/agudo, paso alto/bajo), frecuencia, ganancia (+/-12 dB) y Q.
- Arrastra los puntos de la pantalla: horizontal = frecuencia, vertical = ganancia. Rueda del raton sobre un punto = Q. Doble clic en un punto = ganancia a 0.
- Espectro en vivo de fondo, medidores L/R de entrada (IN) y salida (OUT).
- Auto gain: compensa la ganancia de salida para igualar el nivel de entrada. Con el activado, el control de Salida se vuelve automatico.
- La ventana se puede redimensionar.

## Novedades de esta version

- 21 presets de fabrica, agrupados por categoria (guitarra electrica, guitarra
  acustica, voz femenina, voz masculina, bateria, bajo, piano, metales) en el
  desplegable de arriba de los controles.
- Botones Guardar y Cargar: guardan o abren un preset propio como archivo
  .prismaeq (dialogo normal de Windows para elegir carpeta).
- Botones A / B: guardan dos versiones del ajuste actual y permiten comparar
  entre ellas con un clic.
- Doble clic en un hueco vacio de la pantalla activa una banda libre ahi mismo
  (agrega un punto de EQ nuevo). Como son 6 bandas, el limite es 6 puntos a la
  vez; doble clic en un punto ya puesto sigue reseteando su ganancia a 0.
- Medidores de nivel (IN y OUT) en verde mas claro y luminoso, que pasa a
  amarillo intenso cerca de 0 dB y a rojo si se pasa.

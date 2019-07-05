fucheck
=======

Utilidad para verificar el FU de sistemas de tiempo real.

Archivos de entrada
-------------------
Se aceptan únicamente ficheros XML creados por el Generador de Conjuntos de Tareas para Simulación de Sistemas de Tiempo
Real de la UNPSJB.

Instalación
-----------
Para compilar y generar el ejecutable src/fucheck:

    $ ./configure
    $ make

Para instalar el programa en el sistema, ejecutar como root o mediante sudo:

    # make install

Dependencias
------------
+ libxml2-dev - LibXML 2
+ libgsl-dev - GNU Scientific Library

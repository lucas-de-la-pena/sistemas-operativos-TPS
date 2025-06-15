#!/bin/bash

set -e
set -x

MOUNT_DIR="./prueba"
DISK_FILE="mi_disco.fisopfs"
LOG_FILE="test_output.log"

function setup() {
    echo "--- Iniciando Setup ---"
    make clean
    make
    umount "$MOUNT_DIR" || true
    rm -f "$DISK_FILE"
    rm -rf "$MOUNT_DIR"
    mkdir -p "$MOUNT_DIR"
    ./fisopfs "$MOUNT_DIR" --filedisk "$DISK_FILE"
    sleep 2
    mount | grep "$MOUNT_DIR"
    echo "--- Setup Finalizado ---"
}

function cleanup() {
    echo "--- Iniciando Limpieza ---"
    umount "$MOUNT_DIR" || true
    rm -rf "$MOUNT_DIR"
    echo "--- Limpieza Finalizada ---"
}

function test_crear_y_verificar_archivo() {
    echo "--- Test: Crear y verificar archivo (touch) ---"
    touch "$MOUNT_DIR/hola.txt"
    test -f "$MOUNT_DIR/hola.txt"
    ls -l "$MOUNT_DIR"
}

function test_escribir_y_leer_archivo() {
    echo "--- Test: Escribir y leer archivo (echo, cat) ---"
    echo -n "Hola Sistemas Operativos" > "$MOUNT_DIR/saludo.txt"
    CONTENIDO=$(cat "$MOUNT_DIR/saludo.txt")
    test "$CONTENIDO" == "Hola Sistemas Operativos"
}

function test_append_a_archivo() {
    echo "--- Test: Hacer append a un archivo (echo >>) ---"
    echo -n " y FUSE!" >> "$MOUNT_DIR/saludo.txt"
    CONTENIDO=$(cat "$MOUNT_DIR/saludo.txt")
    test "$CONTENIDO" == "Hola Sistemas Operativos y FUSE!"
    stat "$MOUNT_DIR/saludo.txt"
}

function test_crear_directorio() {
    echo "--- Test: Crear directorio (mkdir) ---"
    mkdir "$MOUNT_DIR/docs"
    test -d "$MOUNT_DIR/docs"
    ls -l "$MOUNT_DIR"
}

function test_directorio_anidado() {
    echo "--- Test: Crear directorio anidado ---"
    mkdir "$MOUNT_DIR/docs/personales"
    touch "$MOUNT_DIR/docs/personales/cv.pdf"
    test -f "$MOUNT_DIR/docs/personales/cv.pdf"
    ls -l "$MOUNT_DIR/docs"
    ls -l "$MOUNT_DIR/docs/personales"
}

function test_eliminar_archivo() {
    echo "--- Test: Eliminar archivo (rm) ---"
    rm "$MOUNT_DIR/hola.txt"
    if [ -f "$MOUNT_DIR/hola.txt" ]; then
        echo "Error: El archivo hola.txt no fue eliminado."
        exit 1
    fi
    ls -l "$MOUNT_DIR"
}

function test_eliminar_directorio_vacio() {
    echo "--- Test: Eliminar directorio vacío (rmdir) ---"
    mkdir "$MOUNT_DIR/vacio"
    rmdir "$MOUNT_DIR/vacio"
    if [ -d "$MOUNT_DIR/vacio" ]; then
        echo "Error: El directorio vacio no fue eliminado."
        exit 1
    fi
    ls -l "$MOUNT_DIR"
}

function test_error_eliminar_directorio_no_vacio() {
    echo "--- Test: Error al eliminar directorio no vacío ---"
    if rmdir "$MOUNT_DIR/docs"; then
        echo "Error: Se permitió eliminar un directorio no vacío."
        exit 1
    fi
    echo "Prueba de error exitosa."
}

function test_persistencia() {
    echo "--- Test: Verificar persistencia ---"
    cleanup
    echo "Volviendo a montar para probar persistencia..."
    mkdir -p "$MOUNT_DIR"
    ./fisopfs "$MOUNT_DIR" --filedisk "$DISK_FILE"
    sleep 2
    test -f "$MOUNT_DIR/saludo.txt"
    test -d "$MOUNT_DIR/docs"
    test -f "$MOUNT_DIR/docs/personales/cv.pdf"
    CONTENIDO=$(cat "$MOUNT_DIR/saludo.txt")
    test "$CONTENIDO" == "Hola Sistemas Operativos y FUSE!"
    echo "Persistencia verificada."
}

trap cleanup EXIT
exec > >(tee "$LOG_FILE") 2>&1

setup

test_crear_y_verificar_archivo
test_escribir_y_leer_archivo
test_append_a_archivo
test_crear_directorio
test_directorio_anidado
test_eliminar_archivo
test_error_eliminar_directorio_no_vacio
test_eliminar_directorio_vacio
test_persistencia

echo ""
echo "----------------------------------------------------"
echo "--- ¡TODAS LAS PRUEBAS PASARON EXITOSAMENTE! ---"
echo "----------------------------------------------------"
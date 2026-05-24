# Makefile
#
# Este archivo automatiza la compilación del proyecto.
# Permite compilar todos los archivos fuente ubicados en la carpeta src/
# y generar un ejecutable del compilador.
#
# También puede incluir reglas para limpiar archivos temporales u objetos
# generados durante la compilación.

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc

SRC = src/main.c src/lexer.c
OUT = compilador

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
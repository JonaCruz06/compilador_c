/*
 * lexer.c
 *
 * Este archivo implementa el analizador léxico del compilador.
 * Su función principal es recorrer el archivo fuente carácter por carácter
 * para identificar palabras reservadas, identificadores, números, cadenas,
 * operadores, delimitadores y símbolos válidos del lenguaje.
 *
 * También se encarga de detectar errores léxicos, como caracteres no válidos
 * o tokens mal formados, e informar la línea donde ocurren.
 *
 * La salida de este módulo es una lista o secuencia de tokens que será
 * utilizada por el analizador sintáctico.
 */
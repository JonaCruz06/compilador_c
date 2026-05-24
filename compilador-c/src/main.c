/*
 * main.c
 *
 * Este archivo funciona como punto de entrada del programa.
 * Se encarga de recibir el archivo fuente con extensión personalizada,
 * validar que pueda abrirse correctamente y coordinar las fases principales
 * del Front-End del compilador:
 *
 * 1. Análisis léxico: generación de tokens.
 * 2. Análisis sintáctico: validación de la gramática y construcción del AST.
 * 3. Análisis semántico: verificación de tipos, variables y ámbitos.
 *
 * Desde este archivo se mandan llamar las funciones principales del lexer,
 * parser y analizador semántico.
 */
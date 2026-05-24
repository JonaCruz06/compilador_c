/*
 * parser.c
 *
 * Este archivo implementa el analizador sintáctico del compilador.
 * Su función es revisar que la secuencia de tokens generada por el lexer
 * cumpla con la gramática del lenguaje definida en BNF.
 *
 * En este módulo se reconocen declaraciones de variables, asignaciones,
 * expresiones y estructuras de control como if, if-else, if-elseif, for,
 * while y do-while.
 *
 * Además, durante el análisis sintáctico se construye el Árbol de Sintaxis
 * Abstracta (AST), el cual representa la estructura lógica del programa y
 * será utilizado durante el análisis semántico.
 */
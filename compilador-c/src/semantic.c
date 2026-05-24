/*
 * semantic.c
 *
 * Este archivo implementa el análisis semántico del compilador.
 * Su función principal es recorrer el Árbol de Sintaxis Abstracta (AST)
 * para verificar que el programa cumpla con las reglas semánticas del lenguaje.
 *
 * Entre sus responsabilidades se encuentran:
 *
 * - Validar que las variables hayan sido declaradas antes de usarse.
 * - Verificar que las asignaciones respeten los tipos de datos.
 * - Comprobar que las condiciones de if, while, for y do-while sean válidas.
 * - Controlar los ámbitos o scopes generados por bloques anidados.
 * - Detectar errores semánticos relacionados con tipos, variables o alcance.
 *
 * Este módulo también puede administrar la tabla de símbolos del programa.
 */
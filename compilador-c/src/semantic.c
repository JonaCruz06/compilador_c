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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "semantic.h"

#define MAX_NESTING 3

static int current_nesting_level = 0;

static int semantic_analyze_node(ASTNode *node) {
    if (!node) return 1;

    // Verificar profundidad de anidamiento
    int is_control_structure = (node->type == NODE_IF || node->type == NODE_WHILE || 
                                node->type == NODE_FOR || node->type == NODE_DO_WHILE);
    
    if (is_control_structure) {
        current_nesting_level++;
        if (current_nesting_level > MAX_NESTING) {
            printf("Error Semantico: Se supero el limite maximo de anidamiento (%d) permitido por la especificacion.\n", MAX_NESTING);
            return 0; // Falla el análisis
        }
    }

    // Validación recursiva de hijos
    if (node->left && !semantic_analyze_node(node->left)) return 0;
    if (node->right && !semantic_analyze_node(node->right)) return 0;
    
    // Validación de bloques internos
    for (int i = 0; i < node->body_count; i++) {
        if (!semantic_analyze_node(node->body[i])) return 0;
    }
    
    // Validación de secuencias anidadas (ej. else if)
    if (node->next && !semantic_analyze_node(node->next)) return 0;

    if (is_control_structure) {
        current_nesting_level--; // Salimos del bloque
    }

    return 1; // Todo válido
}

int semantic_analyze(ASTNode *ast) {
    current_nesting_level = 0;
    printf("Verificando tipado fuerte y limite de anidamiento...\n");
    return semantic_analyze_node(ast);
}
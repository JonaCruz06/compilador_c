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

// Tabla de símbolos y variables de estado
static Symbol sym_table[MAX_SYMBOLS];
static int sym_count = 0;
static int current_scope = 0;
static int current_nesting_level = 0;
static int semantic_errors = 0;

static DataType get_type_from_token(TokenType t) {
    if (t == TOKEN_KW_INT) return TYPE_INT;
    if (t == TOKEN_KW_FLOAT) return TYPE_FLOAT;
    if (t == TOKEN_KW_BOOL) return TYPE_BOOL;
    if (t == TOKEN_KW_STRING) return TYPE_STRING;
    return TYPE_UNKNOWN;
}

static const char* type_to_string(DataType t) {
    if (t == TYPE_INT) return "int";
    if (t == TYPE_FLOAT) return "float";
    if (t == TYPE_BOOL) return "bool";
    if (t == TYPE_STRING) return "string";
    return "desconocido";
}

// Busca una variable desde el ámbito más interno al más externo
static Symbol* lookup_symbol(const char* name) {
    for (int i = sym_count - 1; i >= 0; i--) {
        if (strcmp(sym_table[i].name, name) == 0 && sym_table[i].scope_level <= current_scope) {
            return &sym_table[i];
        }
    }
    return NULL;
}

// Retira de la tabla las variables declaradas en el scope que acaba de terminar
static void pop_scope() {
    while (sym_count > 0 && sym_table[sym_count - 1].scope_level == current_scope) {
        sym_count--;
    }
    current_scope--;
}

// Evalúa recursivamente el tipo de una expresión
static DataType get_expression_type(ASTNode *expr) {
    if (!expr) return TYPE_UNKNOWN;
    
    if (expr->type == NODE_LITERAL) {
        if (expr->token.type == TOKEN_INT_LITERAL) return TYPE_INT;
        if (expr->token.type == TOKEN_FLOAT_LITERAL) return TYPE_FLOAT;
        if (expr->token.type == TOKEN_STRING_LITERAL) return TYPE_STRING;
        if (expr->token.type == TOKEN_BOOL_LITERAL) return TYPE_BOOL;
    } else if (expr->type == NODE_IDENTIFIER) {
        Symbol *sym = lookup_symbol(expr->token.lexeme);
        if (sym) {
            return sym->type;
        } else {
            printf("Error Semantico linea %d: Variable '%s' no declarada.\n", expr->token.line, expr->token.lexeme);
            semantic_errors++;
            return TYPE_UNKNOWN;
        }
    } else if (expr->type == NODE_BINOP) {
        DataType left_t = get_expression_type(expr->left);
        DataType right_t = get_expression_type(expr->right);
        
        // En operadores matemáticos, validamos que los tipos coincidan
        if (left_t != TYPE_UNKNOWN && right_t != TYPE_UNKNOWN && left_t != right_t) {
            printf("Error Semantico linea %d: Incompatibilidad de tipos en operacion binaria (%s y %s).\n", 
                   expr->token.line, type_to_string(left_t), type_to_string(right_t));
            semantic_errors++;
        }
        return left_t; // Retornamos el tipo izquierdo como resultante por simplicidad
    }
    return TYPE_UNKNOWN;
}

static void semantic_analyze_node(ASTNode *node) {
    if (!node) return;

    int is_control_structure = (node->type == NODE_IF || node->type == NODE_WHILE || 
                                node->type == NODE_FOR || node->type == NODE_DO_WHILE);
    
    if (is_control_structure) {
        current_nesting_level++;
        current_scope++; // Nuevo ámbito para variables dentro del bloque
        
        if (current_nesting_level > MAX_NESTING) {
            printf("Error Semantico linea %d: Limite maximo de anidamiento (%d) superado.\n", node->token.line, MAX_NESTING);
            semantic_errors++;
        }
    }

    // Análisis según el tipo de nodo
    if (node->type == NODE_VAR_DECL) {
        char *var_name = node->left->token.lexeme;
        
        // Validar si ya existe en este scope
        for (int i = sym_count - 1; i >= 0 && sym_table[i].scope_level == current_scope; i--) {
            if (strcmp(sym_table[i].name, var_name) == 0) {
                printf("Error Semantico linea %d: Variable '%s' ya declarada en este ambito.\n", node->token.line, var_name);
                semantic_errors++;
                break;
            }
        }
        
        // Registrar variable
        if (sym_count < MAX_SYMBOLS) {
            strcpy(sym_table[sym_count].name, var_name);
            sym_table[sym_count].type = get_type_from_token(node->token.type);
            sym_table[sym_count].scope_level = current_scope;
            
            // Validar la asignación inicial
            DataType expr_type = get_expression_type(node->right);
            if (expr_type != TYPE_UNKNOWN && sym_table[sym_count].type != expr_type) {
                printf("Error Semantico linea %d: Incompatibilidad de tipos. No se puede asignar '%s' a variable '%s' de tipo '%s'.\n", 
                       node->token.line, type_to_string(expr_type), var_name, type_to_string(sym_table[sym_count].type));
                semantic_errors++;
            }
            sym_count++;
        }
    } 
    else if (node->type == NODE_ASSIGN) {
        char *var_name = node->token.lexeme;
        Symbol *sym = lookup_symbol(var_name);
        
        if (!sym) {
            printf("Error Semantico linea %d: Variable '%s' no ha sido declarada.\n", node->token.line, var_name);
            semantic_errors++;
        } else {
            DataType expr_type = get_expression_type(node->right);
            if (expr_type != TYPE_UNKNOWN && sym->type != expr_type) {
                printf("Error Semantico linea %d: Incompatibilidad de tipos al reasignar la variable '%s'.\n", node->token.line, var_name);
                semantic_errors++;
            }
        }
    }

    // Recorrer hijos estructurales
    if (node->type != NODE_VAR_DECL && node->type != NODE_ASSIGN) {
        if (node->left) semantic_analyze_node(node->left);
        if (node->right) semantic_analyze_node(node->right);
    }
    
    // Recorrer el cuerpo (instrucciones internas del bloque)
    for (int i = 0; i < node->body_count; i++) {
        semantic_analyze_node(node->body[i]);
    }

    if (is_control_structure) {
        pop_scope(); // Al salir de la estructura, limpiamos las variables de ese bloque
        current_nesting_level--;
    }

    // Análisis de nodos encadenados (ej. else, elseif)
    if (node->next) {
        semantic_analyze_node(node->next);
    }
}

int semantic_analyze(ASTNode *ast) {
    sym_count = 0;
    current_scope = 0;
    current_nesting_level = 0;
    semantic_errors = 0;

    semantic_analyze_node(ast);

    if (semantic_errors > 0) {
        printf("\nEl analisis semantico termino con %d error(es).\n", semantic_errors);
        return 0; // Falló
    }
    
    return 1; // Éxito
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "semantic.h"

#define MAX_NESTING 3

// Se definen la tabla de símbolos y las variables de estado para el control del análisis
static Symbol sym_table[MAX_SYMBOLS];
static int sym_count = 0;
static int current_scope = 0;
static int current_nesting_level = 0;
static int semantic_errors = 0;

// Se convierte el token reservado del lenguaje a su tipo de dato interno correspondiente
static DataType get_type_from_token(TokenType t) {
    if (t == TOKEN_KW_INT) return TYPE_INT;
    if (t == TOKEN_KW_FLOAT) return TYPE_FLOAT;
    if (t == TOKEN_KW_BOOL) return TYPE_BOOL;
    if (t == TOKEN_KW_STRING) return TYPE_STRING;
    return TYPE_UNKNOWN;
}

// Se traduce el tipo de dato interno a una cadena de texto para generar mensajes de error legibles
static const char* type_to_string(DataType t) {
    if (t == TYPE_INT) return "int";
    if (t == TYPE_FLOAT) return "float";
    if (t == TYPE_BOOL) return "bool";
    if (t == TYPE_STRING) return "string";
    return "desconocido";
}

// Se busca una variable en la tabla de símbolos, revisando desde el ámbito (scope) actual hacia los más externos
static Symbol* lookup_symbol(const char* name) {
    for (int i = sym_count - 1; i >= 0; i--) {
        if (strcmp(sym_table[i].name, name) == 0 && sym_table[i].scope_level <= current_scope) {
            return &sym_table[i];
        }
    }
    return NULL;
}

// Se retiran de la tabla de símbolos aquellas variables cuyo tiempo de vida terminó al cerrar su bloque de código
static void pop_scope() {
    while (sym_count > 0 && sym_table[sym_count - 1].scope_level == current_scope) {
        sym_count--;
    }
    current_scope--;
}

// Se evalúa de manera recursiva el tipo de dato final que resulta de una expresión
static DataType get_expression_type(ASTNode *expr) {
    if (!expr) return TYPE_UNKNOWN;
    
    if (expr->type == NODE_LITERAL) {
        if (expr->token.type == TOKEN_INT_LITERAL) return TYPE_INT;
        if (expr->token.type == TOKEN_FLOAT_LITERAL) return TYPE_FLOAT;
        if (expr->token.type == TOKEN_STRING_LITERAL) return TYPE_STRING;
        if (expr->token.type == TOKEN_BOOL_LITERAL) return TYPE_BOOL;
    } else if (expr->type == NODE_IDENTIFIER) {
        Symbol *sym = lookup_symbol(expr->token.lexeme);
        // Se verifica que la variable utilizada exista previamente en la tabla de símbolos
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
        
        // Se comprueba que los tipos de datos coincidan a la hora de realizar operaciones aritméticas o relacionales
        if (left_t != TYPE_UNKNOWN && right_t != TYPE_UNKNOWN && left_t != right_t) {
            printf("Error Semantico linea %d: Incompatibilidad de tipos en operacion binaria (%s y %s).\n", 
                   expr->token.line, type_to_string(left_t), type_to_string(right_t));
            semantic_errors++;
        }
        return left_t; 
    }
    return TYPE_UNKNOWN;
}

// Se analiza cada nodo del árbol validando reglas de anidamiento, declaración y tipado
static void semantic_analyze_node(ASTNode *node) {
    if (!node) return;

    int is_control_structure = (node->type == NODE_IF || node->type == NODE_WHILE || 
                                node->type == NODE_FOR || node->type == NODE_DO_WHILE);
    
    // Se incrementa el nivel de anidamiento y se crea un nuevo ámbito al entrar en una estructura de control
    if (is_control_structure) {
        current_nesting_level++;
        current_scope++; 
        
        // Se valida que no se supere el límite de profundidad permitido por las especificaciones
        if (current_nesting_level > MAX_NESTING) {
            printf("Error Semantico linea %d: Limite maximo de anidamiento (%d) superado.\n", node->token.line, MAX_NESTING);
            semantic_errors++;
        }
    }

    if (node->type == NODE_VAR_DECL) {
        char *var_name = node->left->token.lexeme;
        
        // Se inspecciona el ámbito actual para evitar declarar dos veces la misma variable
        for (int i = sym_count - 1; i >= 0 && sym_table[i].scope_level == current_scope; i--) {
            if (strcmp(sym_table[i].name, var_name) == 0) {
                printf("Error Semantico linea %d: Variable '%s' ya declarada en este ambito.\n", node->token.line, var_name);
                semantic_errors++;
                break;
            }
        }
        
        // Se registra la nueva variable en la tabla de símbolos
        if (sym_count < MAX_SYMBOLS) {
            strcpy(sym_table[sym_count].name, var_name);
            sym_table[sym_count].type = get_type_from_token(node->token.type);
            sym_table[sym_count].scope_level = current_scope;
            
            // Se valida que el valor asignado en la declaración sea compatible con el tipo de la variable
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
        
        // Se exige que toda variable haya sido declarada antes de recibir un nuevo valor
        if (!sym) {
            printf("Error Semantico linea %d: Variable '%s' no ha sido declarada.\n", node->token.line, var_name);
            semantic_errors++;
        } else {
            // Se garantiza que el nuevo valor coincida con el tipo de dato original de la variable
            DataType expr_type = get_expression_type(node->right);
            if (expr_type != TYPE_UNKNOWN && sym->type != expr_type) {
                printf("Error Semantico linea %d: Incompatibilidad de tipos al reasignar la variable '%s'.\n", node->token.line, var_name);
                semantic_errors++;
            }
        }
    }

    if (node->type != NODE_VAR_DECL && node->type != NODE_ASSIGN) {
        if (node->left) semantic_analyze_node(node->left);
        if (node->right) semantic_analyze_node(node->right);
    }
    
    // Se analizan de forma recursiva todas las instrucciones contenidas dentro del bloque actual.
    for (int i = 0; i < node->body_count; i++) {
        semantic_analyze_node(node->body[i]);
    }

    // Se restaura el nivel de anidamiento y se purgan las variables locales al abandonar la estructura de control.
    if (is_control_structure) {
        pop_scope(); 
        current_nesting_level--;
    }

    if (node->next) {
        semantic_analyze_node(node->next);
    }
}

// Se reinician los contadores y se arranca el proceso principal de análisis semántico.
int semantic_analyze(ASTNode *ast) {
    sym_count = 0;
    current_scope = 0;
    current_nesting_level = 0;
    semantic_errors = 0;

    semantic_analyze_node(ast);

    // Se reporta si la evaluación finalizó con violaciones a las reglas del lenguaje.
    if (semantic_errors > 0) {
        printf("\nEl analisis semantico termino con %d error(es).\n", semantic_errors);
        return 0; // Falló
    }
    
    return 1; // Éxito
}
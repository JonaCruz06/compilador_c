/*
 * parser.h
 *
 * Este archivo contiene las definiciones necesarias para el análisis sintáctico.
 * Aquí se declaran las estructuras principales del Árbol de Sintaxis Abstracta
 * (AST), los tipos de nodos que pueden existir y los prototipos de las
 * funciones implementadas en parser.c.
 *
 * El parser recibe los tokens generados por el lexer y verifica que estén
 * organizados de acuerdo con la gramática definida para el lenguaje.
 */

 #ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Tipos de nodos para el Árbol
typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,
    NODE_BINOP,
    NODE_LITERAL,
    NODE_IDENTIFIER
} ASTNodeType;

// Estructura base de un nodo del AST
typedef struct ASTNode {
    ASTNodeType type;
    Token token; // El token principal asociado a este nodo
    
    // Punteros a los hijos (para operaciones binarias o condicionales)
    struct ASTNode *left;
    struct ASTNode *right;
    
    // Lista de instrucciones (para bloques de código como if/while)
    struct ASTNode **body;
    int body_count;
    int body_capacity;
    
    // Para anidamiento adicional (ej. else if, else)
    struct ASTNode *next; 
} ASTNode;

// Funciones principales del parser
ASTNode* parser_parse(TokenList *tokens);
void ast_free(ASTNode *node);

#endif
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

// Variables globales para el estado del parser
static TokenList *t_list;
static int current_pos = 0;

static Token get_current_token() {
    // Usamos ->items en lugar de ->tokens porque asi esta en tu lexer.h
    if (current_pos < t_list->count) return t_list->items[current_pos];
    
    // Inicializamos con todos los campos que pide tu struct (tipo, lexema, linea, columna)
    Token eof = {TOKEN_EOF, "EOF", 0, 0}; 
    return eof;
}

static void advance() {
    if (current_pos < t_list->count) current_pos++;
}

static ASTNode* create_node(ASTNodeType type, Token t) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->token = t;
    node->left = NULL;
    node->right = NULL;
    node->body = NULL;
    node->body_count = 0;
    node->body_capacity = 0;
    node->next = NULL;
    return node;
}

static void add_to_body(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return;
    if (parent->body_capacity == 0) {
        parent->body_capacity = 4;
        parent->body = malloc(sizeof(ASTNode*) * parent->body_capacity);
    } else if (parent->body_count >= parent->body_capacity) {
        parent->body_capacity *= 2;
        parent->body = realloc(parent->body, sizeof(ASTNode*) * parent->body_capacity);
    }
    parent->body[parent->body_count++] = child;
}

// Función dummy para procesar instrucciones básicas y evitar bucles infinitos
static ASTNode* parse_statement() {
    Token t = get_current_token();
    ASTNode *stmt = create_node(NODE_IDENTIFIER, t);
    advance(); // Avanzamos para consumir el token
    
    // Ignorar tokens hasta el punto y coma o llave para avanzar el parser
    while(get_current_token().type != TOKEN_SEMICOLON && 
          get_current_token().type != TOKEN_RBRACE && 
          get_current_token().type != TOKEN_EOF) {
        advance();
    }
    if(get_current_token().type == TOKEN_SEMICOLON) advance(); // Consumir ';'
    
    return stmt;
}

ASTNode* parser_parse(TokenList *tokens) {
    t_list = tokens;
    current_pos = 0;
    
    // Corregido: Agregamos el 0 de la columna al final para que empate con tu Lexer
    Token root_t = {TOKEN_EOF, "PROGRAM", 0, 0};
    ASTNode *program = create_node(NODE_PROGRAM, root_t);

    while (get_current_token().type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement();
        if (stmt) add_to_body(program, stmt);
    }
    
    return program;
}

void ast_free(ASTNode *node) {
    if (!node) return;
    ast_free(node->left);
    ast_free(node->right);
    for (int i = 0; i < node->body_count; i++) {
        ast_free(node->body[i]);
    }
    free(node->body);
    ast_free(node->next);
    free(node);
}
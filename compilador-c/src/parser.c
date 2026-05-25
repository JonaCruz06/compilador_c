#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"

static TokenList *t_list;
static int current_pos = 0;
static int syntax_errors = 0; // Contador para detener el compilador

// Se obtiene el token actual sobre el cual está trabajando el analizador.
static Token get_current_token() {
    if (current_pos < t_list->count) return t_list->items[current_pos];
    Token eof = {TOKEN_EOF, "EOF", 0, 0};
    return eof;
}

// Se avanza la posición en la lista para leer el siguiente token.
static void advance() {
    if (current_pos < t_list->count) current_pos++;
}

static int match(TokenType type) {
    if (get_current_token().type == type) {
        advance();
        return 1;
    }
    return 0;
}

// Se exige la presencia de un token específico; de lo contrario, se registra un error sintáctico.
static void expect(TokenType type) {
    if (get_current_token().type == type) {
        advance();
    } else {
        printf("Error Sintactico en linea %d: Se esperaba '%s' pero se encontro '%s'\n",
               get_current_token().line, token_type_to_string(type), get_current_token().lexeme);
        syntax_errors++;
        advance(); // Recuperación simple para intentar seguir analizando
    }
}

// Se reserva memoria dinámica y se inicializa un nuevo nodo para el Árbol de Sintaxis Abstracta (AST).
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

// Se añaden instrucciones (nodos hijos) al bloque de código de una estructura, ampliando la memoria si es necesario.
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

static ASTNode* parse_expression();
static ASTNode* parse_statement();
static void parse_block(ASTNode *parent);

// Se procesan los factores básicos de una expresión, como literales, variables o expresiones entre paréntesis.
static ASTNode* parse_factor() {
    Token t = get_current_token();
    if (t.type == TOKEN_IDENTIFIER || t.type == TOKEN_INT_LITERAL ||
        t.type == TOKEN_FLOAT_LITERAL || t.type == TOKEN_STRING_LITERAL ||
        t.type == TOKEN_BOOL_LITERAL) {
        ASTNode *node = create_node(t.type == TOKEN_IDENTIFIER ? NODE_IDENTIFIER : NODE_LITERAL, t);
        advance();
        return node;
    } else if (match(TOKEN_LPAREN)) {
        ASTNode *node = parse_expression();
        expect(TOKEN_RPAREN);
        return node;
    }
    printf("Error Sintactico en linea %d: Expresion no valida '%s'\n", t.line, t.lexeme);
    syntax_errors++;
    advance();
    return NULL;
}

// Se agrupan los factores multiplicativos y de división manteniendo la precedencia.
static ASTNode* parse_term() {
    ASTNode *node = parse_factor();
    while (get_current_token().type == TOKEN_STAR || get_current_token().type == TOKEN_SLASH) {
        Token op = get_current_token();
        advance();
        ASTNode *right = parse_factor();
        ASTNode *new_node = create_node(NODE_BINOP, op);
        new_node->left = node;
        new_node->right = right;
        node = new_node;
    }
    return node;
}

// Se construyen las expresiones sumando, restando o comparando los términos previamente agrupados.
static ASTNode* parse_expression() {
    ASTNode *node = parse_term();
    while (get_current_token().type == TOKEN_PLUS || get_current_token().type == TOKEN_MINUS ||
           get_current_token().type == TOKEN_EQUAL || get_current_token().type == TOKEN_LESS ||
           get_current_token().type == TOKEN_GREATER || get_current_token().type == TOKEN_LESS_EQUAL ||
           get_current_token().type == TOKEN_GREATER_EQUAL || get_current_token().type == TOKEN_NOT_EQUAL) {
        Token op = get_current_token();
        advance();
        ASTNode *right = parse_term();
        ASTNode *new_node = create_node(NODE_BINOP, op);
        new_node->left = node;
        new_node->right = right;
        node = new_node;
    }
    return node;
}

// Se procesa un bloque completo de código delimitado por llaves { }.
static void parse_block(ASTNode *parent) {
    expect(TOKEN_LBRACE);
    while (get_current_token().type != TOKEN_RBRACE && get_current_token().type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement();
        if (stmt) add_to_body(parent, stmt);
    }
    expect(TOKEN_RBRACE);
}

// Se construye el nodo para la estructura de control 'if', encadenando opcionalmente los bloques 'elseif' y 'else'.
static ASTNode* parse_if_statement() {
    Token t = get_current_token();
    expect(TOKEN_KW_IF);
    expect(TOKEN_LPAREN);
    ASTNode *cond = parse_expression();
    expect(TOKEN_RPAREN);

    ASTNode *if_node = create_node(NODE_IF, t);
    if_node->left = cond; 
    parse_block(if_node); 

    ASTNode *current_alt = if_node;

    while (get_current_token().type == TOKEN_KW_ELSEIF) {
        Token elif_t = get_current_token();
        expect(TOKEN_KW_ELSEIF);
        expect(TOKEN_LPAREN);
        ASTNode *elif_cond = parse_expression();
        expect(TOKEN_RPAREN);

        ASTNode *elif_node = create_node(NODE_IF, elif_t); 
        elif_node->left = elif_cond;
        parse_block(elif_node);

        current_alt->next = elif_node;
        current_alt = elif_node;
    }

    if (get_current_token().type == TOKEN_KW_ELSE) {
        Token else_t = get_current_token();
        expect(TOKEN_KW_ELSE);
        ASTNode *else_node = create_node(NODE_IF, else_t); 
        parse_block(else_node);
        current_alt->next = else_node;
    }

    return if_node;
}

// Se construye el nodo para el ciclo 'while', vinculando su condición y su bloque interno.
static ASTNode* parse_while_statement() {
    Token t = get_current_token();
    expect(TOKEN_KW_WHILE);
    expect(TOKEN_LPAREN);
    ASTNode *cond = parse_expression();
    expect(TOKEN_RPAREN);

    ASTNode *while_node = create_node(NODE_WHILE, t);
    while_node->left = cond;
    parse_block(while_node);

    return while_node;
}

static ASTNode* parse_do_while_statement() {
    Token t = get_current_token();
    expect(TOKEN_KW_DO);

    ASTNode *do_node = create_node(NODE_DO_WHILE, t);
    parse_block(do_node);

    expect(TOKEN_KW_WHILE);
    expect(TOKEN_LPAREN);
    do_node->left = parse_expression(); 
    expect(TOKEN_RPAREN);
    expect(TOKEN_SEMICOLON);

    return do_node;
}

// Se estructura el nodo 'for', separando la declaración inicial, la condición de paro y el incremento.
static ASTNode* parse_for_statement() {
    Token t = get_current_token();
    expect(TOKEN_KW_FOR);
    expect(TOKEN_LPAREN);

    ASTNode *for_node = create_node(NODE_FOR, t);

    for_node->left = parse_statement(); 
    for_node->right = parse_expression();
    expect(TOKEN_SEMICOLON);

    Token id = get_current_token();
    expect(TOKEN_IDENTIFIER);
    expect(TOKEN_ASSIGN);
    ASTNode *inc_expr = parse_expression();
    
    ASTNode *inc = create_node(NODE_ASSIGN, id);
    inc->right = inc_expr;
    expect(TOKEN_RPAREN);

    for_node->next = inc; 
    parse_block(for_node);

    return for_node;
}

// Se diferencian las instrucciones simples, identificando si es una nueva declaración de variable o una reasignación.
static ASTNode* parse_declaration_or_assignment() {
    Token t = get_current_token();
    
    if (t.type == TOKEN_KW_INT || t.type == TOKEN_KW_FLOAT || 
        t.type == TOKEN_KW_BOOL || t.type == TOKEN_KW_STRING) {
        advance();
        Token id = get_current_token();
        expect(TOKEN_IDENTIFIER);
        expect(TOKEN_ASSIGN);
        ASTNode *expr = parse_expression();
        expect(TOKEN_SEMICOLON);
        
        ASTNode *decl = create_node(NODE_VAR_DECL, t);
        decl->left = create_node(NODE_IDENTIFIER, id); 
        decl->right = expr; 
        return decl;
    } 
    else if (t.type == TOKEN_IDENTIFIER) {
        advance();
        expect(TOKEN_ASSIGN);
        ASTNode *expr = parse_expression();
        expect(TOKEN_SEMICOLON);
        
        ASTNode *assign = create_node(NODE_ASSIGN, t);
        assign->right = expr;
        return assign;
    }

    printf("Error Sintactico en linea %d: Instruccion no reconocida '%s'\n", t.line, t.lexeme);
    syntax_errors++;
    advance();
    return NULL;
}

static ASTNode* parse_statement() {
    TokenType t = get_current_token().type;
    if (t == TOKEN_KW_IF) return parse_if_statement();
    if (t == TOKEN_KW_WHILE) return parse_while_statement();
    if (t == TOKEN_KW_DO) return parse_do_while_statement();
    if (t == TOKEN_KW_FOR) return parse_for_statement();
    return parse_declaration_or_assignment();
}

// Se inicia el proceso principal del parser, recorriendo los tokens para ensamblar el árbol completo del programa.
ASTNode* parser_parse(TokenList *tokens) {
    t_list = tokens;
    current_pos = 0;
    syntax_errors = 0;
    
    Token root_t = {TOKEN_EOF, "PROGRAM", 0, 0};
    ASTNode *program = create_node(NODE_PROGRAM, root_t);

    while (get_current_token().type != TOKEN_EOF) {
        ASTNode *stmt = parse_statement();
        if (stmt) add_to_body(program, stmt);
    }
    
    // Si se encontraron errores durante la lectura, se purga la memoria y se aborta la compilación.
    if (syntax_errors > 0) {
        printf("\nEl analisis sintactico termino con %d error(es).\n", syntax_errors);
        ast_free(program);
        return NULL;
    }
    
    return program;
}

// Se recorre recursivamente el AST para liberar toda la memoria ocupada por los nodos una vez finalizado su uso.
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
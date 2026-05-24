/*
 * lexer.h
 *
 * Este archivo contiene las definiciones necesarias para el análisis léxico.
 * Aquí se declaran los tipos de tokens que reconoce el lenguaje, las
 * estructuras utilizadas para representar cada token y los prototipos de
 * las funciones implementadas en lexer.c.
 *
 * El lexer es responsable de leer el código fuente y convertirlo en una
 * secuencia de tokens que posteriormente serán utilizados por el parser.
 */

#ifndef LEXER_H
#define LEXER_H

#define MAX_LEXEME_LENGTH 128

typedef enum {
    TOKEN_EOF,

    TOKEN_IDENTIFIER,

    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOL_LITERAL,

    TOKEN_KW_INT,
    TOKEN_KW_FLOAT,
    TOKEN_KW_BOOL,
    TOKEN_KW_STRING,

    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_ELSEIF,
    TOKEN_KW_FOR,
    TOKEN_KW_WHILE,
    TOKEN_KW_DO,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_MOD,

    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    TOKEN_SEMICOLON,
    TOKEN_COMMA,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME_LENGTH];
    int line;
    int column;
} Token;

typedef struct {
    Token *items;
    int count;
    int capacity;
} TokenList;

void token_list_init(TokenList *list);
void token_list_free(TokenList *list);

int lexer_analyze_file(const char *file_path, TokenList *tokens);
void lexer_print_tokens(const TokenList *tokens);

const char *token_type_to_string(TokenType type);

#endif
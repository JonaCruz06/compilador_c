/*
 * lexer.c
 *
 * Este archivo implementa el analizador léxico del compilador.
 * Su función principal es recorrer el archivo fuente carácter por carácter
 * para identificar palabras reservadas, identificadores, números, cadenas,
 * operadores, delimitadores y símbolos válidos del lenguaje.
 *
 * También se encarga de detectar errores léxicos, como caracteres no válidos
 * o tokens mal formados, e informar la línea donde ocurren.
 *
 * La salida de este módulo es una lista o secuencia de tokens que será
 * utilizada por el analizador sintáctico.
 */

#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char *TOKEN_NAMES[] = {
    "TOKEN_EOF",

    "TOKEN_IDENTIFIER",

    "TOKEN_INT_LITERAL",
    "TOKEN_FLOAT_LITERAL",
    "TOKEN_STRING_LITERAL",
    "TOKEN_BOOL_LITERAL",

    "TOKEN_KW_INT",
    "TOKEN_KW_FLOAT",
    "TOKEN_KW_BOOL",
    "TOKEN_KW_STRING",

    "TOKEN_KW_IF",
    "TOKEN_KW_ELSE",
    "TOKEN_KW_ELSEIF",
    "TOKEN_KW_FOR",
    "TOKEN_KW_WHILE",
    "TOKEN_KW_DO",

    "TOKEN_PLUS",
    "TOKEN_MINUS",
    "TOKEN_STAR",
    "TOKEN_SLASH",
    "TOKEN_MOD",

    "TOKEN_ASSIGN",
    "TOKEN_EQUAL",
    "TOKEN_NOT_EQUAL",
    "TOKEN_LESS",
    "TOKEN_LESS_EQUAL",
    "TOKEN_GREATER",
    "TOKEN_GREATER_EQUAL",

    "TOKEN_AND",
    "TOKEN_OR",
    "TOKEN_NOT",

    "TOKEN_SEMICOLON",
    "TOKEN_COMMA",

    "TOKEN_LPAREN",
    "TOKEN_RPAREN",
    "TOKEN_LBRACE",
    "TOKEN_RBRACE",

    "TOKEN_UNKNOWN"
};

void token_list_init(TokenList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void token_list_free(TokenList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static int add_token(TokenList *list, TokenType type, const char *lexeme, int line, int column) {
    if (list->count >= list->capacity) {
        int new_capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        Token *new_items = realloc(list->items, sizeof(Token) * new_capacity);

        if (new_items == NULL) {
            printf("Error: no se pudo reservar memoria.\n");
            return 0;
        }

        list->items = new_items;
        list->capacity = new_capacity;
    }

    Token token;
    token.type = type;
    token.line = line;
    token.column = column;

    strncpy(token.lexeme, lexeme, MAX_LEXEME_LENGTH - 1);
    token.lexeme[MAX_LEXEME_LENGTH - 1] = '\0';

    list->items[list->count] = token;
    list->count++;

    return 1;
}

static TokenType get_keyword_type(const char *word) {
    if (strcmp(word, "int") == 0) return TOKEN_KW_INT;
    if (strcmp(word, "float") == 0) return TOKEN_KW_FLOAT;
    if (strcmp(word, "bool") == 0) return TOKEN_KW_BOOL;
    if (strcmp(word, "string") == 0) return TOKEN_KW_STRING;

    if (strcmp(word, "if") == 0) return TOKEN_KW_IF;
    if (strcmp(word, "else") == 0) return TOKEN_KW_ELSE;
    if (strcmp(word, "elseif") == 0) return TOKEN_KW_ELSEIF;
    if (strcmp(word, "for") == 0) return TOKEN_KW_FOR;
    if (strcmp(word, "while") == 0) return TOKEN_KW_WHILE;
    if (strcmp(word, "do") == 0) return TOKEN_KW_DO;

    if (strcmp(word, "true") == 0 || strcmp(word, "false") == 0) {
        return TOKEN_BOOL_LITERAL;
    }

    return TOKEN_IDENTIFIER;
}

int lexer_analyze_file(const char *file_path, TokenList *tokens) {
    FILE *file = fopen(file_path, "r");

    if (file == NULL) {
        printf("Error: no se pudo abrir el archivo '%s'.\n", file_path);
        return 0;
    }

    int c;
    int line = 1;
    int column = 1;

    while ((c = fgetc(file)) != EOF) {
        if (c == ' ' || c == '\t' || c == '\r') {
            column++;
            continue;
        }

        if (c == '\n') {
            line++;
            column = 1;
            continue;
        }

        if (isalpha(c) || c == '_') {
            char buffer[MAX_LEXEME_LENGTH];
            int i = 0;
            int start_column = column;

            do {
                if (i < MAX_LEXEME_LENGTH - 1) {
                    buffer[i++] = (char)c;
                }

                c = fgetc(file);
                column++;
            } while (isalnum(c) || c == '_');

            buffer[i] = '\0';

            if (c != EOF) {
                ungetc(c, file);
            }

            TokenType type = get_keyword_type(buffer);

            if (!add_token(tokens, type, buffer, line, start_column)) {
                fclose(file);
                return 0;
            }

            continue;
        }

        if (isdigit(c)) {
            char buffer[MAX_LEXEME_LENGTH];
            int i = 0;
            int start_column = column;
            int has_dot = 0;

            do {
                if (c == '.') {
                    has_dot = 1;
                }

                if (i < MAX_LEXEME_LENGTH - 1) {
                    buffer[i++] = (char)c;
                }

                c = fgetc(file);
                column++;
            } while (isdigit(c) || c == '.');

            buffer[i] = '\0';

            if (c != EOF) {
                ungetc(c, file);
            }

            TokenType type = has_dot ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL;

            if (!add_token(tokens, type, buffer, line, start_column)) {
                fclose(file);
                return 0;
            }

            continue;
        }

        if (c == '"') {
            char buffer[MAX_LEXEME_LENGTH];
            int i = 0;
            int start_column = column;

            column++;
            c = fgetc(file);

            while (c != EOF && c != '"' && c != '\n') {
                if (i < MAX_LEXEME_LENGTH - 1) {
                    buffer[i++] = (char)c;
                }

                column++;
                c = fgetc(file);
            }

            if (c != '"') {
                printf("Error lexico en linea %d, columna %d: cadena sin cerrar.\n",
                       line, start_column);
                fclose(file);
                return 0;
            }

            buffer[i] = '\0';

            if (!add_token(tokens, TOKEN_STRING_LITERAL, buffer, line, start_column)) {
                fclose(file);
                return 0;
            }

            column++;
            continue;
        }

        int start_column = column;
        char lexeme[3] = { (char)c, '\0', '\0' };
        int next = fgetc(file);

        if (next != EOF) {
            lexeme[1] = (char)next;
        }

        if (c == '=' && next == '=') {
            add_token(tokens, TOKEN_EQUAL, lexeme, line, start_column);
            column += 2;
            continue;
        }

        if (c == '!' && next == '=') {
            add_token(tokens, TOKEN_NOT_EQUAL, lexeme, line, start_column);
            column += 2;
            continue;
        }

        if (c == '<' && next == '=') {
            add_token(tokens, TOKEN_LESS_EQUAL, lexeme, line, start_column);
            column += 2;
            continue;
        }

        if (c == '>' && next == '=') {
            add_token(tokens, TOKEN_GREATER_EQUAL, lexeme, line, start_column);
            column += 2;
            continue;
        }

        if (c == '&' && next == '&') {
            add_token(tokens, TOKEN_AND, lexeme, line, start_column);
            column += 2;
            continue;
        }

        if (c == '|' && next == '|') {
            add_token(tokens, TOKEN_OR, lexeme, line, start_column);
            column += 2;
            continue;
        }

        if (next != EOF) {
            ungetc(next, file);
        }

        lexeme[1] = '\0';

        switch (c) {
            case '+': add_token(tokens, TOKEN_PLUS, lexeme, line, start_column); break;
            case '-': add_token(tokens, TOKEN_MINUS, lexeme, line, start_column); break;
            case '*': add_token(tokens, TOKEN_STAR, lexeme, line, start_column); break;
            case '/': add_token(tokens, TOKEN_SLASH, lexeme, line, start_column); break;
            case '%': add_token(tokens, TOKEN_MOD, lexeme, line, start_column); break;

            case '=': add_token(tokens, TOKEN_ASSIGN, lexeme, line, start_column); break;
            case '<': add_token(tokens, TOKEN_LESS, lexeme, line, start_column); break;
            case '>': add_token(tokens, TOKEN_GREATER, lexeme, line, start_column); break;
            case '!': add_token(tokens, TOKEN_NOT, lexeme, line, start_column); break;

            case ';': add_token(tokens, TOKEN_SEMICOLON, lexeme, line, start_column); break;
            case ',': add_token(tokens, TOKEN_COMMA, lexeme, line, start_column); break;

            case '(': add_token(tokens, TOKEN_LPAREN, lexeme, line, start_column); break;
            case ')': add_token(tokens, TOKEN_RPAREN, lexeme, line, start_column); break;
            case '{': add_token(tokens, TOKEN_LBRACE, lexeme, line, start_column); break;
            case '}': add_token(tokens, TOKEN_RBRACE, lexeme, line, start_column); break;

            default:
                printf("Error lexico en linea %d, columna %d: caracter no valido '%c'.\n",
                       line, column, c);
                fclose(file);
                return 0;
        }

        column++;
    }

    add_token(tokens, TOKEN_EOF, "EOF", line, column);

    fclose(file);
    return 1;
}

void lexer_print_tokens(const TokenList *tokens) {
    printf("\nTokens generados:\n");
    printf("---------------------------------------------\n");

    for (int i = 0; i < tokens->count; i++) {
        printf("%-22s  %-12s  Linea: %d\n",
               token_type_to_string(tokens->items[i].type),
               tokens->items[i].lexeme,
               tokens->items[i].line);
    }

    printf("---------------------------------------------\n");
}

const char *token_type_to_string(TokenType type) {
    if (type >= TOKEN_EOF && type <= TOKEN_UNKNOWN) {
        return TOKEN_NAMES[type];
    }

    return "TOKEN_INVALID";
}
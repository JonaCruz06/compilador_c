#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Nombres de los tokens usados para imprimirlos de forma legible.
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

// Inicializa una lista vacía para almacenar tokens
void token_list_init(TokenList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

// Libera la memoria reservada para la lista de tokens
void token_list_free(TokenList *list) {
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

// Agrega un nuevo token a la lista dinámica de tokens.
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

// Determina si una palabra es reservada o un identificador.
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

// Leer el archivo fuente y generar los tokens del lenguaje.
int lexer_analyze_file(const char *file_path, TokenList *tokens) {
    // Se abre el archivo fuente en modo lectura.
    FILE *file = fopen(file_path, "r");

    // Se valida que el archivo exista y se pueda abrir correctamente.
    if (file == NULL) {
        printf("Error: no se pudo abrir el archivo '%s'.\n", file_path);
        return 0;
    }

    int c;
    int line = 1;
    int column = 1;

    // Se recorre el archivo carácter por carácter hasta llegar al final (EOF).
    while ((c = fgetc(file)) != EOF) {
        // Ignorar espacios en blanco, tabulaciones y retornos de carro.
        if (c == ' ' || c == '\t' || c == '\r') {
            column++;
            continue;
        }

        // Se incrementa el contador de líneas y reiniciar la columna al detectar un salto de línea.
        if (c == '\n') {
            line++;
            column = 1;
            continue;
        }

        // Se identifican palabras reservadas o nombres de variables
        if (isalpha(c) || c == '_') {
            char buffer[MAX_LEXEME_LENGTH];
            int i = 0;
            int start_column = column;

            // Se extraen todos los caracteres alfanuméricos consecutivos de la palabra
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

            // Se clasifica la palabra extraída para saber su tipo específico.
            TokenType type = get_keyword_type(buffer);

            if (!add_token(tokens, type, buffer, line, start_column)) {
                fclose(file);
                return 0;
            }

            continue;
        }

        // Se identifican números enteros o flotantes
        if (isdigit(c)) {
            char buffer[MAX_LEXEME_LENGTH];
            int i = 0;
            int start_column = column;
            int has_dot = 0;

            // Se extraen los dígitos numéricos con un punto decimal
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

            // Se asigna el tipo de token dependiendo de si contiene un punto decimal o no
            TokenType type = has_dot ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL;

            if (!add_token(tokens, type, buffer, line, start_column)) {
                fclose(file);
                return 0;
            }

            continue;
        }

        // Se procesan cadenas de texto delimitadas por comillas.
        if (c == '"') {
            char buffer[MAX_LEXEME_LENGTH];
            int i = 0;
            int start_column = column;

            column++;
            c = fgetc(file);

            // Se leen caracteres hasta encontrar el cierre de comillas o un salto de línea
            while (c != EOF && c != '"' && c != '\n') {
                if (i < MAX_LEXEME_LENGTH - 1) {
                    buffer[i++] = (char)c;
                }

                column++;
                c = fgetc(file);
            }

            // Se reporta un error si la cadena se termina sin comillas de cierre.
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

        // Se reconocen operadores relacionales o lógicos compuestos por dos caracteres.
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

        // Clasificar operadores aritméticos, de asignación y delimitadores de un solo carácter.
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

            // Se reporta un error léxico si el carácter leído no pertenece a los permitidos por el lenguaje
            default:
                printf("Error lexico en linea %d, columna %d: caracter no valido '%c'.\n",
                       line, column, c);
                fclose(file);
                return 0;
        }

        column++;
    }

    // Se agrega el token final para indicar la conclusión de la lectura del archivo
    add_token(tokens, TOKEN_EOF, "EOF", line, column);

    fclose(file);
    return 1;
}

// Imprime en pantalla la lista de tokens generados.
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

// Devuelve el nombre en texto de un tipo de token.
const char *token_type_to_string(TokenType type) {
    if (type >= TOKEN_EOF && type <= TOKEN_UNKNOWN) {
        return TOKEN_NAMES[type];
    }

    return "TOKEN_INVALID";
}
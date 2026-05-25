#ifndef LEXER_H
#define LEXER_H

// Se define la longitud máxima permitida para el texto de un token
#define MAX_LEXEME_LENGTH 128

// Se agrupan y enumeran todos los tipos de tokens que el lenguaje reconoce
typedef enum {
    TOKEN_EOF, // Se marca el final del archivo

    TOKEN_IDENTIFIER, // Se identifican nombres de variables o funciones

    // Se clasifican los valores literales, números, cadenas, booleanos
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BOOL_LITERAL,

    // Se definen las palabras reservadas para los tipos de datos
    TOKEN_KW_INT,
    TOKEN_KW_FLOAT,
    TOKEN_KW_BOOL,
    TOKEN_KW_STRING,

    // Se definen las palabras reservadas para las estructuras de control
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_ELSEIF,
    TOKEN_KW_FOR,
    TOKEN_KW_WHILE,
    TOKEN_KW_DO,

    // Se listan los operadores aritméticos básicos
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_MOD,

    // Se listan los operadores de asignación y comparación
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    // Se listan los operadores lógicos
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    // Se definen los delimitadores y símbolos de agrupación
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_UNKNOWN // Se utiliza para atrapar caracteres no válidos
} TokenType;

// Se estructura la información de un token individual
typedef struct {
    TokenType type;                   // Se almacena el tipo de token identificado
    char lexeme[MAX_LEXEME_LENGTH];   // Se guarda el texto original extraído del código
    int line;                         // Se registra la línea donde se encontró
    int column;                       // Se registra la columna exacta donde inicia
} Token;

// Se crea una estructura para manejar un arreglo dinámico de tokens
typedef struct {
    Token *items;     // Se apunta al bloque de memoria con los tokens
    int count;        // Se lleva el conteo de cuántos tokens hay guardados
    int capacity;     // Se registra el tamaño máximo actual antes de redimensionar
} TokenList;

// Se inicializa la lista dinámica donde se guardarán los tokens generados
void token_list_init(TokenList *list);

// Se libera la memoria utilizada por la lista de tokens al terminar
void token_list_free(TokenList *list);

// Se analiza un archivo fuente completo y se genera la lista de tokens correspondiente
int lexer_analyze_file(const char *file_path, TokenList *tokens);

// Se muestran en pantalla los tokens generados para facilitar su depuración
void lexer_print_tokens(const TokenList *tokens);

// Se convierte el valor numérico de un tipo de token a texto legible para poder imprimirlo
const char *token_type_to_string(TokenType type);

#endif
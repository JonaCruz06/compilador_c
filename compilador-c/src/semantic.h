#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

// Tipos de datos soportados para la tabla de símbolos
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_UNKNOWN
} DataType;

// Estructura de un símbolo (variable)
typedef struct {
    char name[50];
    DataType type;
    int scope_level;
} Symbol;

#define MAX_SYMBOLS 1000

// Inicia el análisis semántico y valida reglas como tipos y anidamiento
int semantic_analyze(ASTNode *ast);

#endif
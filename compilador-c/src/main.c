/*
 * main.c
 *
 * Este archivo funciona como punto de entrada del programa.
 * Se encarga de recibir el archivo fuente con extensión personalizada,
 * validar que pueda abrirse correctamente y coordinar las fases principales
 * del Front-End del compilador:
 *
 * 1. Análisis léxico: generación de tokens.
 * 2. Análisis sintáctico: validación de la gramática y construcción del AST.
 * 3. Análisis semántico: verificación de tipos, variables y ámbitos.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "semantic.h"

#define SOURCE_EXTENSION ".cmra"

static int has_valid_extension(const char *file_path) {
    const char *dot = strrchr(file_path, '.');

    if (dot == NULL) {
        return 0;
    }

    return strcmp(dot, SOURCE_EXTENSION) == 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso correcto:\n");
        printf("  ./compilador archivo.cmra\n");
        return 1;
    }

    const char *file_path = argv[1];

    if (!has_valid_extension(file_path)) {
        printf("Error: el archivo debe tener extension %s\n", SOURCE_EXTENSION);
        return 1;
    }

    TokenList tokens;
    token_list_init(&tokens);

    printf("Analizando archivo: %s\n", file_path);

    if (!lexer_analyze_file(file_path, &tokens)) {
        printf("\nAnalisis lexico fallido.\n");
        token_list_free(&tokens);
        return 1;
    }

    printf("\nAnalisis lexico completado correctamente.\n");
    lexer_print_tokens(&tokens);

    // --- FASE 2: PARSER (Análisis Sintáctico) ---
    printf("\nIniciando Analisis Sintactico...\n");
    ASTNode *ast = parser_parse(&tokens);
    if (ast == NULL) {
        printf("Error: Analisis sintactico fallido.\n");
        token_list_free(&tokens);
        return 1;
    }
    printf("Analisis sintactico (AST) generado correctamente.\n");

    // --- FASE 3: SEMÁNTICO (Análisis Semántico) ---
    printf("\nIniciando Analisis Semantico...\n");
    if (!semantic_analyze(ast)) {
        printf("Error: Analisis semantico fallido.\n");
        ast_free(ast);
        token_list_free(&tokens);
        return 1;
    }
    printf("Analisis semantico completado. El codigo es valido.\n");

    // Liberación de memoria de todas las fases
    ast_free(ast);
    token_list_free(&tokens);

    return 0;
}
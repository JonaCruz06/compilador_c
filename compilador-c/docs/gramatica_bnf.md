# Gramática BNF del Lenguaje

Este documento contiene la definición formal de la gramática del lenguaje diseñado
para el proyecto de compiladores.

La gramática se expresa usando Notación de Backus-Naur (BNF) y describe la forma
válida en que pueden escribirse las instrucciones del lenguaje, incluyendo:

- Declaración de variables.
- Asignaciones.
- Expresiones aritméticas y lógicas.
- Estructuras de control.
- Bloques de código.
- Anidamiento de instrucciones.

Esta gramática es utilizada como base para el desarrollo del analizador sintáctico.

## Reglas de Producción

```bnf
<programa> ::= <lista_instrucciones>
<lista_instrucciones> ::= <instruccion> <lista_instrucciones> | ε

<instruccion> ::= <declaracion> | <asignacion> | <estructura_control>
<declaracion> ::= <tipo> TOKEN_IDENTIFIER TOKEN_ASSIGN <expresion> TOKEN_SEMICOLON
<tipo> ::= TOKEN_KW_INT | TOKEN_KW_FLOAT | TOKEN_KW_BOOL | TOKEN_KW_STRING

<asignacion> ::= TOKEN_IDENTIFIER TOKEN_ASSIGN <expresion> TOKEN_SEMICOLON

<estructura_control> ::= <if_stmt> | <while_stmt> | <for_stmt> | <do_while_stmt>

<if_stmt> ::= TOKEN_KW_IF TOKEN_LPAREN <expresion> TOKEN_RPAREN TOKEN_LBRACE <lista_instrucciones> TOKEN_RBRACE <elseif_list> <else_opt>
<elseif_list> ::= TOKEN_KW_ELSEIF TOKEN_LPAREN <expresion> TOKEN_RPAREN TOKEN_LBRACE <lista_instrucciones> TOKEN_RBRACE <elseif_list> | ε
<else_opt> ::= TOKEN_KW_ELSE TOKEN_LBRACE <lista_instrucciones> TOKEN_RBRACE | ε

<while_stmt> ::= TOKEN_KW_WHILE TOKEN_LPAREN <expresion> TOKEN_RPAREN TOKEN_LBRACE <lista_instrucciones> TOKEN_RBRACE
<do_while_stmt> ::= TOKEN_KW_DO TOKEN_LBRACE <lista_instrucciones> TOKEN_RBRACE TOKEN_KW_WHILE TOKEN_LPAREN <expresion> TOKEN_RPAREN TOKEN_SEMICOLON
<for_stmt> ::= TOKEN_KW_FOR TOKEN_LPAREN <declaracion> <expresion> TOKEN_SEMICOLON <asignacion_sin_puntoycoma> TOKEN_RPAREN TOKEN_LBRACE <lista_instrucciones> TOKEN_RBRACE

<expresion> ::= <termino> ((TOKEN_PLUS | TOKEN_MINUS | TOKEN_EQUAL | TOKEN_LESS | TOKEN_GREATER) <termino>)*
<termino> ::= <factor> ((TOKEN_STAR | TOKEN_SLASH) <factor>)*
<factor> ::= TOKEN_IDENTIFIER | TOKEN_INT_LITERAL | TOKEN_FLOAT_LITERAL | TOKEN_STRING_LITERAL | TOKEN_BOOL_LITERAL | TOKEN_LPAREN <expresion> TOKEN_RPAREN
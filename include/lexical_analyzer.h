#ifndef _LEXICAL_ANALYZER_H_
#define _LEXICAL_ANALYZER_H_

#include <stdio.h>

extern int fileno (FILE *__stream) __THROW __wur;

#ifndef YYTOKENTYPE
#define YYTOKENTYPE
typedef enum cminus_token_type {
    //运算
    ADD = 259,              // +
    SUB = 260,              // -
    MUL = 261,              // *
    DIV = 262,              // /
    LT = 263,               // <
    LTE = 264,              // <=
    GT = 265,               // >
    GTE = 266,              // >=
    EQ = 267,               // ==
    NEQ = 268,              // !=
    ASSIN = 269,            // =
    //符号
    SEMICOLON = 270,        // ;
    COMMA = 271,            // ,
    LPARENTHESE = 272,      // (
    RPARENTHESE = 273,      // )
    LBRACKET = 274,         // [
    RBRACKET = 275,         // ]
    LBRACE = 276,           // {
    RBRACE = 277,           // }
    //关键字
    ELSE = 278,             // else 
    IF = 279,               // if 
    INT = 280,              // int
    FLOAT = 281,            // float
    RETURN = 282,           // return
    VOID = 283,             // void
    WHILE = 284,            // while
    //ID和NUM
    IDENTIFIER = 285,       // "Indicates identifiers, such as variable names, function names, etc."
    INTEGER = 286,          // "Indicates an integer value, such as 1, 2, 3, etc."
    FLOATPOINT = 287,       // "Indicates a floating point value, such as 1.1, 2.1, etc."
    ARRAY = 288,            // "Indicates an array declaration, such as int data[10]."
    LETTER = 289,           // "Indicates an alphabetic character, such as 'a'."
    //others
    EOL = 290,              // "Indicates a line terminator, such as '\n'."
    COMMENT = 291,          // "Indicates an annotation, such as // ..., etc."
    BLANK = 292,            // "Indicates blank characters, such as spaces, tabs, etc."
    ERROR = 258             // "Indicates an error or unrecognizable mark."

} Token;


typedef struct{
    char text[256];
    int token;
    int lines;
    int pos_start;
    int pos_end;
} Token_Node;

#define MAX_NUM_TOKEN_NODE 1024

void analyzer(char* input_file, Token_Node* token_stream);

#endif /* YYTOKENTYPE */
#endif /* lexical_analyzer.h */

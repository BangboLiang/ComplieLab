%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}
%}

/***************
 TO STUDENTS: Copy your Lab1 here. Make adjustments if necessary.

 Note: don't modify the prologue unless you know what you are doing.
***************/

/* Example for you :-) */
%%
\+ {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return ADD;}  //ADD = 259,
\- {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return SUB;}  //SUB = 260,
\* {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return MUL;}    //MUL = 261,
\/ {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return DIV;}    //DIV = 262,
\< {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return LT;}   //LT = 263,
\<\= {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return LTE;}   //LTE = 264,识别2个字符
\> {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return GT;}   //GT = 265,
\>\= {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return GTE;}   //GTE = 266,
\=\= {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return EQ;}    //EQ = 267,
\!\= {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return NEQ;}    //NEQ = 268,
\= {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return ASSIN;}    //ASSIN = 269,
\; {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return SEMICOLON;}    //SEMICOLON = 270,
\, {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return COMMA;}    //COMMA = 271,
\( {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return LPARENTHESE;}    //LPARENTHESE = 272,
\) {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return RPARENTHESE;}    //RPARENTHESE = 273,
\[ {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return LBRACKET;}   //LBRACKET = 274,
\] {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return RBRACKET;}   //RBRACKET = 275,
\{ {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return LBRACE;}   //LBRACE = 276,
\} {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return RBRACE;}   //RBRACE = 277,
else {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return ELSE;}   //ELSE = 278,
if {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return IF;}    //IF = 279,
int {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return INT;}    //INT = 280,
float {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return FLOAT;}    //FLOAT = 281,
return {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return RETURN;}    //RETURN = 282,
void {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return VOID;}    //VOID = 283,
while {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return WHILE;}    //WHILE = 284,
[a-zA-Z]+ {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return IDENTIFIER;}    //IDENTIFIER = 285,
[0-9]+ {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return INTEGER;}    //INTEGER = 286,
[0-9]+\.|[0-9]*\.[0-9]+ {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return FLOATPOINT;}    //FLOATPOINT = 287,
\[\] {pos_start=pos_end;pos_end+=strlen(yytext);pass_node(yytext);return ARRAY;}    //ARRAY = 288,
[a-zA-Z] {pos_start=pos_end;pos_end+=1;pass_node(yytext);return LETTER;}    //LETTER = 289,
\n {pos_start=1;pos_end=1;lines+=strlen(yytext);}    //EOL = 290,
\/\*(?:[^\*]|\*+[^\/\*])*\*+\/ {pos_start=pos_end;pos_end+=strlen(yytext);for(int i=0;i<strlen(yytext);i++){
		if(yytext[i]=='\n'){
			pos_start=1;
			pos_end=1;
			lines++;
		}
		else pos_end++;} 
		}   //COMMENT = 291
[ ]+ {pos_start=pos_end;pos_end+=strlen(yytext);}    //BLANK = 292,
[\t]+ {pos_start=pos_end;pos_end+=strlen(yytext);}    //TAB,我自己设的


. {return 0;}

%%
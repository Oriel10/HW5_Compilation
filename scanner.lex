%{
#include "production_rules.h"
#include "error_output.hpp"
#include "parser.tab.hpp"

%}

%option noyywrap
%option yylineno

num     [0]|([1-9][0-9]*)
id	    [A-Za-z][A-Za-z0-9]*
relop_eq ==|!=
relop_size <=|>=|<|>
binop_muldiv   \*|\/
binop_plusminus   \+|\-
whitespace  ([\t\n\r ])
string                  \"([^\n\r\"\\]|\\[rnt"\\])+\"
line_comment            \/\/[^\r\n]*[\r|\n|\r\n]?

%%
void                {yylval = new Node(yytext, "VOID", yylineno); return VOID;}
int                 {yylval = new Node(yytext, "INT", yylineno); return INT;}
byte                {yylval = new Node(yytext, "BYTE", yylineno); return BYTE;}
b                   {yylval = new Node(yytext, "B", yylineno); return B;} 
bool                {yylval = new Node(yytext, "BOOL", yylineno); return BOOL;}
auto                {yylval = new Node(yytext, "AUTO", yylineno); return AUTO;}
and                 {yylval = new Node(yytext, "AND", yylineno); return AND;}
or                  {yylval = new Node(yytext, "OR", yylineno); return OR;}
not                 {yylval = new Node(yytext, "NOT", yylineno); return NOT;}
true                {yylval = new Node(yytext, "TRUE", yylineno); return TRUE;}
false               {yylval = new Node(yytext, "FALSE", yylineno); return FALSE;}
return              {yylval = new Node(yytext, "RETURN", yylineno); return RETURN;}
if                  {yylval = new Node(yytext, "IF", yylineno); return IF;}
else                {yylval = new Node(yytext, "ELSE", yylineno); return ELSE;}
while               {yylval = new Node(yytext, "WHILE", yylineno); return WHILE;}
break               {yylval = new Node(yytext, "BREAK", yylineno); return BREAK;}
continue            {yylval = new Node(yytext, "CONTINUE", yylineno); return CONTINUE;}
\;                  {yylval = new Node(yytext, "SC", yylineno); return SC;}
\,                  {yylval = new Node(yytext, "COMMA", yylineno); return COMMA;}
\(                  {yylval = new Node(yytext, "LPAREN", yylineno); return LPAREN;}
\)                  {yylval = new Node(yytext, "RPAREN", yylineno); return RPAREN;}
\{                  {yylval = new Node(yytext, "LBRACE", yylineno); return LBRACE;}
\}                  {yylval = new Node(yytext, "RBRACE", yylineno); return RBRACE;}
=                   {yylval = new Node(yytext, "ASSIGN", yylineno); return ASSIGN;}
{relop_eq}          {yylval = new Node(yytext, "RELOP_EQ", yylineno); return RELOP_EQ;} /*{relop}          return RELOP;*/
{relop_size}        {yylval = new Node(yytext, "RELOP_SIZE", yylineno); return RELOP_SIZE;}
{binop_muldiv}      {yylval = new Node(yytext, "BINOP_MULDIV", yylineno); return BINOP_MULDIV;} /* {binop}             return BINOP; */
{binop_plusminus}   {yylval = new Node(yytext, "BINOP_PLUSMINUS", yylineno); return BINOP_PLUSMINUS;} 
{id}                {yylval = new Node(yytext, "ID", yylineno); return ID;}
{num}               {yylval = new Node(yytext, "NUM", yylineno); return NUM;}
{whitespace}        ;
{line_comment}      ;
{string}            {yylval = new Node(yytext, "STRING", yylineno); return STRING;}
.                   {return YYUNDEF;}


%%
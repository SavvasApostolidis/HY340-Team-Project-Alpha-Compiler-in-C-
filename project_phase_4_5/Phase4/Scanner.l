%{
    #include <string>
    #include <stdlib.h>
    #include <string.h>
    #include "Parser.hpp"

    #ifdef __cplusplus
    static int yyinput (void );
    #else
    static int input (void );
    #endif  

    #define YY_DECL int yylex(void)

    extern YYSTYPE yylval;
%}

%option header-file="./al.h"
%option yylineno
%option noyywrap

digit           [0-9]
id              [a-zA-Z][a-z_A-Z[:digit:]]*
integer         {digit}{digit}*
real            {digit}+\.{digit}+
whitespace      [ \n\t\r]+
line_comment    "//".*

%%

"if"        {return IF;}
"else"      {return ELSE;} 
"while"     {return WHILE;}
"for"       {return FOR;}
"function"  {return FUNCTION;}
"return"    {return RETURN;}
"break"     {return BREAK;}
"continue"  {return CONTINUE;}
"and"       {return AND;}
"not"       {return NOT;}
"or"        {return OR;}
"local"     {return LOCAL;}
"true"      {return TRUE;}
"false"     {return FALSE;}
"nil"       {return NIL;}

"="         {return '=';}
"+"         {return '+';}
"-"         {return '-';}
"*"         {return '*';}
"/"         {return '/';}
"%"         {return '%';}
"=="        {return EQUAL;}
"!="        {return NOT_EQUAL;}
"++"        {return PLUS_PLUS;}
"--"        {return MINUS_MINUS;}
">"         {return '>';}
"<"         {return '<';}
">="        {return GREATER_EQUAL;}
"<="        {return LESS_EQUAL;}

"{"         {return '{';}
"}"         {return '}';}
"["         {return '[';}
"]"         {return ']';}
"("         {return '(';}
")"         {return ')';}
";"         {return ';';}
","         {return ',';}
":"         {return ':';}
"::"        {return DOUBLE_COLON;}
"."         {return '.';}
".."        {return DOUBLE_DOT;}

{id}        {yylval.stringValue = strdup(yytext); return ID;}
{integer}   {yylval.integerValue = atoi(yytext); return NUMBER;}
{real}      {yylval.realValue = atof(yytext); return REAL;}

\"  {
        std::string temp;
        std::string original;
        
        char c;

        int starting_line;

        starting_line = yylineno;

        while((c = yyinput())){

            if(c == EOF)
            {
                fprintf(stderr,"[ERROR] : unclosed string in starting line [%d] \n",starting_line);
                return -1;
            }

            if(c == '\"')
                break;  
            
            original = original + c;

            if(c == '\\')
            {
                c = yyinput();

                if(c == EOF)
                {
                    fprintf(stderr,"[ERROR] : unclosed string starting line [%d] \n",starting_line);
                    return -1;
                }

                original = original + c;

                if(c == 't')
                    c = '\t';
                else if(c == 'n')
                    c = '\n';
                else if(c =='r')
                    c = '\r';
                else if(c == '\\')
                    c = '\\';
                else if(c == 'b')
                    c = '\b';
                else if(c == '\"')
                    c = '\"';
                else
                {
                    printf("[ERROR] : %c after slash, sequence not recognized in line [%d] \n", c , yylineno);
                    return -1; 
                }
            } 
            temp = temp + c;
        }
        yylval.stringValue = strdup(const_cast<char *>(temp.c_str()));
        return STRING;
    }

"/*"    {
            int line = 0;
            int tobeclosed = 1;
            int numb_of_nested_comment = 0;
            register int c;
            
            std::string block_comment_lines;
            std::string nested_number;
            
            while((c = yyinput()) != EOF ){    
                if (c =='\n'){
                    line++;
                }
                if(c == '/'){
                    c = yyinput();
                    if(c == '*'){
                        numb_of_nested_comment++;
                        tobeclosed++;
                    }else{
                        unput(c);
                    }
                }else if (c == '*'){ 
                    while ( (c = yyinput()) == '*');
                    if ( c == '/' ){
                        tobeclosed--;
                        if(tobeclosed==0){
                            break;
                        }
                    }
                }
            } 

            if(tobeclosed > 0)
            {
                fprintf(stderr,"[ERROR] : unclosed comment in starting line [%d] \n" , yylineno - line);
                return -1;
            }
        }

{line_comment}  {}
{whitespace}    {}

<<EOF>>     {return EOF;}

.           {fprintf(stderr,"[ERROR] : unsupported input  [%s] in line [%d] \n",yytext , yylineno); return -1;}

%%
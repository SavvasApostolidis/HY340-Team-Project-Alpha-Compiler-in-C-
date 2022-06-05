%{
    #include <stdio.h>
    #include "Quad.h"
    #include "Instructions.h"

    int scope = 0;

    unsigned int currentQuad            = 0;

    unsigned int functionLocalOffset    = 0;
    unsigned int programVarOffset       = 0;
    unsigned int formalArgOffset        = 0;
    unsigned int scopeSpaceCounter      = 1;

    int br_cont_counter_allowed         = 0;
    int function_name_counter           = 0;

    int call_was_used                   = 0;

    bool redifine_name_function         = false;
    bool redifined_arguments_function   = false;
    bool is_anonymous_function          = false;
    bool check_for_name                 = true;
    bool elist_is_empty                 = false;

    SymbolTable* symbolTable            = new SymbolTable();
    SymbolTableEntry* function_curr     = nullptr;

    FILE*  syntax_file                  = nullptr;

    std::stack<SymbolTableEntry*> functions_stack;

    std::stack<int> loopCounterStack;

    std::stack<int> functionLocalOffsetStack;

    std::stack<int> functionJumpStack;

    std::stack<std::list<unsigned int> > break_list;
    std::stack<std::list<unsigned int> > continue_list;
    std::stack<std::list<unsigned int> > return_list;

    expr*  expr1temp;
    expr*  expr1andtemp;
    expr*  expr1equaltemp;
    expr*  expr1notequaltemp;
%}

%code requires
{
    #include "Quad.h"
    #include "Instructions.h"
}

%error-verbose

%union
{
    int                 integerValue;
    double              realValue;
    char*               stringValue;
    SymbolTableEntry*   symentry;
    expr*               expr_token;
    forloop*            for_loop_expr;
    unsigned int        label_targets;
}

%start program

/**/
%token <stringValue>    ID
%token <integerValue>   NUMBER
%token <realValue>      REAL
%token <stringValue>    STRING

/*Keywords*/
%token  IF ELSE WHILE FOR FUNCTION RETURN BREAK
%token  CONTINUE AND NOT OR LOCAL TRUE FALSE NIL

/*Operators*/
%token  '=' '+' '-' '*' '/' '%' '>' '<'
%token  NOT_EQUAL PLUS_PLUS MINUS_MINUS EQUAL
%token  GREATER_EQUAL LESS_EQUAL

/*Punctuation Marks*/
%token  '{' '}' '[' ']' '(' ')' ';' ',' ':' '.'
%token   DOUBLE_COLON DOUBLE_DOT

%right      '='
%left       OR
%left       AND
%nonassoc   EQUAL NOT_EQUAL
%nonassoc   '>' GREATER_EQUAL '<' LESS_EQUAL
%left       '+' '-'
%left       '*' '/' '%'
%right      NOT PLUS_PLUS MINUS_MINUS UMINUS
%left       '.' DOUBLE_DOT
%left       '[' ']'
%left       '(' ')'

%type<stringValue>      id_optional
%type<symentry>         funcdef funcprefix

%type<expr_token>       tableitem lvalue term expr member primary assignexpr jumper andjumper
%type<expr_token>       const list_comma_expr elist call objectdef indexedelem
%type<expr_token>       indexed list_comma_indexed methodcall callsuffix normcall
%type<label_targets>    ifprefix elseprefix whilestart whilecond fortest N

%type<for_loop_expr>    forprefix

%%

program: zero_or_more_statements            {
                                                fprintf(syntax_file ,"Programm \n");
                                            }
         ;

zero_or_more_statements: zero_or_more_statements stmt
                         | %empty
                         ;

stmt:  expr ';'                             {
                                                if($1->type == bool_e)
                                                {
                                                    handleTriada(&$1);
                                                }

                                                //resetTemporaryVariable();

                                                fprintf(syntax_file, "stmt: [ Exprension; ] \n");
                                            }
        | ifstmt                            {
                                                //resetTemporaryVariable();

                                                fprintf(syntax_file, "stmt: [ If ] \n");
                                            }
        | whilestmt                         {
                                                //resetTemporaryVariable();

                                                fprintf(syntax_file, "stmt: [ While ] \n");
                                            }
        | forstmt                           {
                                                //resetTemporaryVariable();

                                                fprintf(syntax_file, "stmt: [ For ] \n");
                                            }
        | returnstmt                        {
                                                //resetTemporaryVariable();

                                                fprintf(syntax_file, "stmt: [ Return ] \n");
                                            }
        | BREAK ';'                         {
                                                //resetTemporaryVariable();

                                                if(!br_cont_counter_allowed){
                                                    ERROR(" [break] must be in a loop",yylineno);
                                                    exit(0);
                                                }
                                                else
                                                {
                                                    break_list.top().push_back(nextQuadLabel());
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }

                                                fprintf(syntax_file, "stmt: [ Break; ] \n");
                                            }
        | CONTINUE ';'                      {
                                                //resetTemporaryVariable();

                                                if(!br_cont_counter_allowed){
                                                    ERROR(" [continue] must be in a loop",yylineno);
                                                    exit(0);
                                                }
                                                else
                                                {
                                                    continue_list.top().push_back(nextQuadLabel());
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }

                                                fprintf(syntax_file, "stmt: [ Continue; ] \n");
                                            }
        | block                             {
                                                //resetTemporaryVariable();

                                                fprintf(syntax_file, "stmt: [ Block ] \n");
                                            }
        | funcdef                           {
                                                //resetTemporaryVariable();

                                                fprintf(syntax_file, "stmt: [ FuncDef ] \n");
                                            }
        | ';'                               {
                                                //resetTemporaryVariable();

                                                fprintf(syntax_file, "stmt: [ ; ] \n");
                                            }
        ;

jumper:  %empty                             {
                                                expr* tmp;
                                                tmp = new expr(bool_e);

                                                tmp->truelist=std::list<unsigned int>();
                                                tmp->falselist=std::list<unsigned int>();

                                                if(expr1temp->type == bool_e)
                                                {
                                                    if(!(expr1temp->falselist.empty()))
                                                    {
                                                        for(auto x:expr1temp->falselist)
                                                        {
                                                            patchLabel(x,nextQuadLabel());
                                                        }
                                                    }

                                                    tmp->truelist.merge(expr1temp->truelist);
                                                }
                                                else
                                                {
                                                    tmp->truelist.push_back(nextQuadLabel());

                                                    emit(if_eq , nullptr , expr1temp , new expr(constbool_e,true) , 0 , yylineno);

                                                    //no need to patch!!!!!!!!!!!!!!!!
                                                    emit(jump , nullptr , nullptr , nullptr , nextQuadLabel() + 1 , yylineno);
                                                }

                                                $$ = tmp;
                                            }
          ;

andjumper: %empty                           {
                                                expr* tmp;
                                                tmp = new expr(bool_e);

                                                tmp->truelist  = std::list<unsigned int>();
                                                tmp->falselist = std::list<unsigned int>();

                                                if(expr1andtemp->type == bool_e)
                                                {
                                                    if(!(expr1andtemp->truelist.empty()))
                                                    {
                                                        for(auto x:expr1andtemp->truelist)
                                                        {
                                                            patchLabel(x,nextQuadLabel());
                                                        }
                                                    }

                                                    tmp->falselist.merge(expr1andtemp->falselist);
                                                }else
                                                {
                                                    tmp->falselist.push_back(nextQuadLabel()+1);

                                                    emit(if_eq , nullptr , expr1andtemp , new expr(constbool_e,true) , nextQuadLabel() + 2 , yylineno);
                                                    emit(jump , nullptr , nullptr , nullptr , 0 ,yylineno);
                                                }

                                                $$ = tmp;
                                            }
           ;

expr:  assignexpr                           {
                                                $$=$1;

                                                fprintf(syntax_file, "expr: [ Assingment ] \n");
                                            }
        | expr '+' expr                     {
                                                arithmeticMissuse("+" , $1 , $3);

                                                emitArithmeticOperation(add_op , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension + Exprension ] \n");
                                            }
        | expr '-' expr                     {
                                                arithmeticMissuse("-" , $1 , $3);

                                                emitArithmeticOperation(sub_op , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension - Exprension ] \n");
                                            }
        | expr '*' expr                     {
                                                arithmeticMissuse("*" , $1 , $3);

                                                emitArithmeticOperation(mul_op , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension * Exprension ] \n");
                                            }
        | expr '/' expr                     {
                                                arithmeticMissuse("/" , $1 , $3);

                                                emitArithmeticOperation(div_op , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension / Exprension ] \n");
                                            }
        | expr '%' expr                     {
                                                arithmeticMissuse("%" , $1 , $3); //na trexei gia non double input se kapoia fash

                                                emitArithmeticOperation(mod_op , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension % Exprension ] \n");
                                            }
        | expr '>' expr                     {
                                                emitReOperation(if_greater , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension > Exprension ] \n");
                                            }
        | expr GREATER_EQUAL expr           {
                                                emitReOperation(if_greatereq , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension >= Exprension ] \n");
                                            }
        | expr '<' expr                     {
                                                emitReOperation(if_less , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension < Exprension ] \n");
                                            }
        | expr LESS_EQUAL expr              {
                                                emitReOperation(if_lesseq , &$$ , $1 , $3);

                                                fprintf(syntax_file, "expr: [ Exprension <= Exprension ] \n");
                                            }
        | expr EQUAL                        {
                                                if($1->type == bool_e)
                                                    handleTriada(&$1);
                                            }
          expr                              {
                                                expr* expr1tmp = $1;
                                                expr* expr2tmp = $4;

                                                if($4->type == bool_e)
                                                {
                                                    handleTriada(&expr2tmp);
                                                }

                                                $$            = new expr(bool_e);
                                                $$->truelist  = std::list<unsigned int>();
                                                $$->falselist = std::list<unsigned int>();

                                                $$->truelist.push_back(nextQuadLabel());
                                                $$->falselist.push_back(nextQuadLabel()+1);

                                                //tobepatched
                                                emit(if_eq , nullptr , expr1tmp , expr2tmp , 0 , yylineno);
                                                //tobepatched
                                                emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);

                                                fprintf(syntax_file, "expr: [ Exprension == Exprension ] \n");
                                            }
        | expr NOT_EQUAL                    {
                                                if($1->type == bool_e)
                                                    handleTriada(&$1);
                                            }
          expr                              {
                                                expr* expr1tmp = $1;
                                                expr* expr2tmp = $4;

                                                if($4->type == bool_e)
                                                {
                                                    handleTriada(&expr2tmp);
                                                }

                                                $$            = new expr(bool_e);
                                                $$->truelist  = std::list<unsigned int>();
                                                $$->falselist = std::list<unsigned int>();

                                                $$->truelist.push_back(nextQuadLabel());
                                                $$->falselist.push_back(nextQuadLabel()+1);

                                                //tobepatched
                                                emit(if_noteq,nullptr,expr1tmp,expr2tmp,0,yylineno);
                                                emit(jump,nullptr,nullptr,nullptr,0,yylineno);//tobepatched

                                                fprintf(syntax_file, "expr: [ Exprension != Exprension ] \n");
                                            }
        | expr AND                          {
                                                expr1andtemp = $1;
                                            }
          andjumper expr                    {
                                                $$ = $4;

                                                if($5->type == bool_e)
                                                {
                                                    $$->truelist = $5->truelist;
                                                    $$->falselist.merge($5->falselist);
                                                }
                                                else
                                                {
                                                    $$->truelist.push_back(nextQuadLabel());
                                                    $$->falselist.push_back(nextQuadLabel() + 1);

                                                    emit(if_eq , nullptr , $5 , new expr(constbool_e,true) , 0 , yylineno);
                                                    //tobepatched!!!!!!!!!!!!!!!!!!!!
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }

                                                fprintf(syntax_file, "expr: [ Exprension and Exprension ] \n");
                                            }
        | expr OR                           {
                                                expr1temp=$1;
                                            }
          jumper expr                       {
                                                $$ = $4;

                                                if($5->type == bool_e)
                                                {
                                                    $$->truelist.merge($5->truelist);
                                                    $$->falselist=$5->falselist;
                                                }
                                                else
                                                {
                                                    $$->truelist.push_back(nextQuadLabel());
                                                    $$->falselist.push_back(nextQuadLabel()+1);

                                                    emit(if_eq  ,nullptr , $5 , new expr(constbool_e,true) , 0 , yylineno);
                                                    //tobepatched!!!!!!!!!!!!!!!!!!!!
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }

                                                fprintf(syntax_file, "expr: [ Exprension or Exprension ] \n");
                                            }
        | term                              {
                                                $$ = $1;

                                                fprintf(syntax_file, "expr: [ Term ] \n");
                                            }
        ;

term: '(' expr ')'                          {
                                                $$ = $2;

                                                fprintf(syntax_file, "term: [ ( Exprension ) ] \n");
                                            }
        | '-' expr  %prec UMINUS            {
                                                if(!uminusMissuse($2))
                                                {
                                                    $$ = new expr(arithmetic_e);
                                                    $$->symbol = isTemporaryExpression($2)? $2->symbol :newTemporaryVariable();
                                                    emit(uminus_op , $$ , $2 , nullptr , 0 , yylineno);
                                                }
                                                call_was_used = 0;

                                                fprintf(syntax_file, "term: [ - Exprension ] \n");
                                            }
        | NOT expr                          {
                                                std::list<unsigned int> tmplist;

                                                if($2->type != bool_e)
                                                {
                                                    $$              = new expr(bool_e);
                                                    $$->truelist    = std::list<unsigned int>();
                                                    $$->falselist   = std::list<unsigned int>();

                                                    $$->truelist.push_back(nextQuadLabel()+1);
                                                    $$->falselist.push_back(nextQuadLabel());

                                                    emit(if_eq,nullptr,$2,new expr(constbool_e,true),0,yylineno);

                                                    emit(jump,nullptr,nullptr,nullptr,0,yylineno);
                                                }
                                                else
                                                {
                                                    $$              = $2;

                                                    tmplist         = $$->truelist;
                                                    $$->truelist    = $$->falselist;
                                                    $$->falselist   = tmplist;
                                                }

                                                fprintf(syntax_file, "term: [ not Exprension ] \n");
                                            }
        | PLUS_PLUS lvalue                  {
                                                functionMissuse($2->symbol , "Trying ++function");

                                                if($2->type == tableitem_e)
                                                {
                                                    $$ = emitIfTableItem($2);

                                                    emit(add_op , $$ , $$  , new expr(1) , 0 , yylineno);
                                                    emit(tablesetelem , $2 , $2->index , $$, 0 , yylineno);
                                                }else
                                                {
                                                    emit(add_op , $2 , $2 , new expr(1) , 0 , yylineno);

                                                    $$          = new expr(arithmetic_e);
                                                    $$->symbol  = newTemporaryVariable();

                                                    emit(assign_op , $$ , $2 , nullptr , 0 , yylineno);
                                                }

                                                fprintf(syntax_file, "term: [ ++Exprension ] \n");
                                            }
        | lvalue PLUS_PLUS                  {
                                                expr* value = nullptr;

                                                functionMissuse($1->symbol , "Trying function++");

                                                $$          = new expr(var_e);
                                                $$->symbol  = newTemporaryVariable();

                                                if($1->type == tableitem_e)
                                                {
                                                    value = emitIfTableItem($1);

                                                    emit(assign_op , $$ , value , nullptr , 0 , yylineno);
                                                    emit(add_op , value , value , new expr(1) , 0 , yylineno);
                                                    emit(tablesetelem , $1 , $1->index , value , 0 , yylineno);
                                                }else
                                                {
                                                    emit(assign_op , $$ , $1 , nullptr , 0 , yylineno);
                                                    emit(add_op , $1 , $1 , new expr(1) , 0 , yylineno);
                                                }

                                                fprintf(syntax_file, "term: [ Exprension++ ] \n");
                                            }
        | MINUS_MINUS lvalue                {
                                                functionMissuse($2->symbol , "Trying --function");

                                                if($2->type == tableitem_e)
                                                {
                                                    $$ = emitIfTableItem($2);

                                                    emit(sub_op , $$ ,$$  , new expr(1) , 0 , yylineno);
                                                    emit(tablesetelem , $2 , $2->index , $$, 0 , yylineno);
                                                }else
                                                {
                                                    emit(sub_op , $2 , $2 , new expr(1) , 0 , yylineno);

                                                    $$          = new expr(arithmetic_e);
                                                    $$->symbol  = newTemporaryVariable();

                                                    emit(assign_op , $$ , $2 , nullptr , 0 , yylineno);
                                                }

                                                fprintf(syntax_file, "term: [ --Exprension ) ] \n");
                                            }
        | lvalue MINUS_MINUS                {
                                                expr* value = nullptr;

                                                functionMissuse($1->symbol , "Trying function++");

                                                $$          = new expr(var_e);
                                                $$->symbol  = newTemporaryVariable();

                                                if($1->type == tableitem_e)
                                                {
                                                    value = emitIfTableItem($1);

                                                    emit(assign_op , $$ , value , nullptr , 0 , yylineno);
                                                    emit(sub_op , value , value , new expr(1) , 0 , yylineno);
                                                    emit(tablesetelem , $1 , $1->index , value , 0 , yylineno);
                                                }else
                                                {
                                                    emit(assign_op , $$ , $1 , nullptr , 0 , yylineno);
                                                    emit(sub_op , $1 , $1 , new expr(1) , 0 , yylineno);
                                                }

                                                fprintf(syntax_file, "term: [ Exprension-- ] \n");
                                            }
        | primary                           {
                                                $$ = $1;

                                                fprintf(syntax_file, "term: [ Primary ] \n");
                                            }
        ;

assignexpr: lvalue '=' expr                 {
                                                expr* exprtmp = $3;

                                                functionMissuse($1->symbol , "cannot use function as lvalue");

                                                if($1->type == tableitem_e )
                                                {
                                                    if($3->type != bool_e)
                                                    {
                                                        emit(tablesetelem , $1 , $1->index , $3 , 0 , yylineno);
                                                        $$          = emitIfTableItem($1);
                                                        $$->type    = assign_e;
                                                    }
                                                    else
                                                    {
                                                        handleTriada(&exprtmp);

                                                        emit(tablesetelem , $1 , $1->index , exprtmp , 0 , yylineno);
                                                        $$          = emitIfTableItem($1);
                                                        $$->type    = assign_e;
                                                    }
                                                }else
                                                {
                                                    if($3->type != bool_e)
                                                    {
                                                        emit(assign_op , $1 , $3 , nullptr , 0 , yylineno);

                                                        $$          = new expr(assign_e);
                                                        $$->symbol  = newTemporaryVariable();

                                                        emit(assign_op , $$ , $1 , nullptr , 0 , yylineno);
                                                    }
                                                    else
                                                    {
                                                        handleTriada(&exprtmp);

                                                        emit(assign_op , $1 , exprtmp , nullptr , 0 , yylineno);
                                                        $$          = new expr(assign_e);
                                                        $$->symbol  = newTemporaryVariable();

                                                        emit(assign_op , $$ , $1 , nullptr , 0 , yylineno);
                                                    }
                                                }

                                                fprintf(syntax_file, "assignexpr: [ Lvalue = Exprension ] \n");
                                            }
             ;

primary: lvalue                             {
                                                $$ = emitIfTableItem($1);

                                                fprintf(syntax_file, "primary: [ Lvalue ] \n");
                                            }
        | call                              {
                                                call_was_used = 1;
                                                $$            = $1;

                                                fprintf(syntax_file, "primary: [ Call ] \n");
                                            }
        | objectdef                         {
                                                $$ = $1;

                                                fprintf(syntax_file, "primary: [ ObjectDef ] \n");}
        | '(' funcdef ')'                   {
                                                $$          = new expr(programfunc_e);
                                                $$->symbol  = $2;

                                                fprintf(syntax_file, "primary: [ ( Funcdef ) ] \n");
                                            }
        | const                             {
                                                $$ = $1;

                                                fprintf(syntax_file, "primary: [ Const ] \n");
                                            }
        ;

lvalue: ID                                  {
                                                $$= new expr();
                                                handleIdVariable(&($$->symbol) , $1);
                                                setSymScope($$->symbol);

                                                $$ = new expr($$->symbol);
                                            }
        | LOCAL ID                          {
                                                $$= new expr();
                                                
                                                
                                                handleLocalVariable(&($$->symbol) , $2);
                                                
                                                setSymScope($$->symbol);

                                                $$ = new expr($$->symbol);
                                            }
        | DOUBLE_COLON ID                   {
                                                $$= new expr();
                                                handleGlobalVariable(&($$->symbol) , $2);
                                                setSymScope($$->symbol);

                                                $$ = new expr($$->symbol);
                                            }
        | member                            {
                                                $$ = $1;
                                                fprintf(syntax_file, "lvalue: [ Member ] \n");

                                            }
        ;

tableitem: lvalue '.' ID                    {
                                                $$ = memberItem(&$1 , const_cast<char*>(( std::string($3) ).c_str()));
                                                
                                                fprintf(syntax_file, "tableitem: [ Lvalue.ID -> Lvalue.%s] \n" , $3);
                                            }
            ;

tableitem: lvalue '[' expr ']'              {
                                                
                                                expr* exprtmp = $3;

                                                $1          = emitIfTableItem($1);
                                                $$          = new expr(tableitem_e);
                                                $$->symbol  = $1->symbol;

                                                if($3->type == bool_e)
                                                    handleTriada(&exprtmp);

                                                $$->index  = exprtmp;

                                                fprintf(syntax_file, "tableitem: [ Lvalue [ Exprension ] ] \n");
                                            };

member: tableitem                          {
                                              $$ = $1;
                                           }
        | call '.' ID                      {
                                                SymbolTableEntry *temp = new SymbolTableEntry("\"" + std::string($3) + "\"", LOCAL_VAR , var_s , scope , yylineno , 1);

                                                $$        = $1;
                                                $$->index = new expr(temp);
                                                $$->type  = tableitem_e;

                                                fprintf(syntax_file, "member: [ Call.ID -> Call.%s] \n" , $3);
                                            }
        | call '[' expr ']'                 {
                                                expr* exprtmp = $3;
                                                $$            = $1;

                                                if($3->type == bool_e)
                                                    handleTriada(&exprtmp);

                                                $$->index = exprtmp;
                                                $$->type  = tableitem_e;

                                                fprintf(syntax_file, "member: [ Call [ Exprension ] ] \n");
                                            }
        ;

call: call '(' elist ')'                    {
                                                $$ = makeCall($$ , $3);

                                                fprintf(syntax_file, "call: [ Call ( Elist ) ] \n");
                                            };

call:  lvalue callsuffix                    {
                                                

                                                if($2->type == methcall_e)
                                                {
                                                    std::vector<expr*> temp;
                                                    int i;

                                                    $1=emitIfTableItem($1);
                                                    
                                                    expr* self = $1;

                                                    temp.push_back(self);

                                                    for(auto x : ($2->exprlist))
                                                        temp.push_back(x);

                                                    for(i=0; i < temp.size() - 1; i++)
                                                        temp[i]->next = temp[i + 1];
                                                    temp[i]->next = nullptr;

                                                    $1 = emitIfTableItem(memberItem(&self , const_cast<char*>(($2->symbol->name).c_str())));

                                                    $2->exprlist = std::vector<expr*>(temp);
                                                }

                                                $$ = makeCall($1 , $2);

                                                fprintf(syntax_file, "call: [ Lvalue CallSufix ] \n");
                                            };

call: '(' funcdef ')' '(' elist ')'         {
                                                expr* function   = new expr(programfunc_e);
                                                function->symbol = $2;

                                                $$ = makeCall(function , $5);

                                                fprintf(syntax_file, "call: [ ( FuncDef ) ( Elist ) ] \n");
                                            };

callsuffix: normcall                        {
                                                $$ = $1;

                                                fprintf(syntax_file, "callsuffix: [ NormCall ] \n");
                                            };

callsuffix: methodcall                      {
                                                $$ = $1;

                                                fprintf(syntax_file, "callsuffix: [ MethodCall ] \n");
                                            };

normcall: '(' elist ')'                     {
                                                $$           = new expr(normcall_e);
                                                $$->exprlist = $2->exprlist;

                                                fprintf(syntax_file, "normcall: [ ( Elist ) ] \n");
                                            }
            ;

methodcall: DOUBLE_DOT
            ID
            '('
            elist
            ')'                             {
                                                $$           = new expr(methcall_e , std::string($2));
                                                $$->exprlist = $4->exprlist;

                                                fprintf(syntax_file , "methodcall: [ ..ID ( Elist ) -> ..%s (Elist) ]\n" , $2);
                                            }
            ;

elist: expr                                 {
                                                if($1->type == bool_e)
                                                    handleTriada(&$1);
                                            }
list_comma_expr                             {
                                                std::vector<expr*> temp;
                                                int i;

                                                temp.push_back($1);

                                                for(auto x : ($3->exprlist))
                                                    temp.push_back(x);

                                                for(i=0; i < temp.size() - 1; i++)
                                                    temp[i]->next = temp[i + 1];
                                                temp[i]->next = nullptr;

                                                $$ = new expr();
                                                $$->exprlist = std::vector<expr*>(temp);
                                            }
	    | %empty                            {
                                                $$ = new expr(elist_e);
                                                $$->exprlist = std::vector<expr*>();
                                            }
	    ;

list_comma_expr: list_comma_expr ',' expr   {
                                                expr* exprtmp = $3;

                                                if($3->type == bool_e)
                                                    handleTriada(&exprtmp);

                                                ($$->exprlist).push_back(exprtmp);
                                            }
		        | %empty                    {
                                                $$ = new expr(elist_e);
                                                $$->exprlist = std::vector<expr*>();
                                            }
		        ;

objectdef: '[' elist ']'                    {
                                                $$ = makeNewTable($2 , true);

                                                fprintf(syntax_file, "objectdef: [ ObjectDef [ elist ] ] \n");
                                            }
	        ;

objectdef: '[' indexed ']'                  {
                                                $$ = makeNewTable($2 , false);

                                                fprintf(syntax_file, "objectdef: [ ObjectDef [ indexed ] ] \n");
                                            }
	        ;

indexed: indexedelem list_comma_indexed     {
                                                std::vector<expr*> temp;
                                                int i;

                                                temp.push_back($1);

                                                for(auto x : ($2->exprlist))
                                                    temp.push_back(x);

                                                for(i=0; i < temp.size() - 1; i++)
                                                    temp[i]->next = temp[i + 1];
                                                temp[i]->next = nullptr;

                                                $$ = new expr(indexed_e);
                                                $$->exprlist = std::vector<expr*>(temp);
                                            }
	      ;

list_comma_indexed: list_comma_indexed
                    ','
                    indexedelem             {
                                                ($$->exprlist).push_back($3);
                                            }
		            | %empty                {
                                                $$ = new expr(indexed_e);
                                                $$->exprlist = std::vector<expr*>();
                                            }
		            ;

indexedelem: '{' expr                       {
                                                if($2->type == bool_e)
                                                    handleTriada(&$2);
                                            }
              ':' expr '}'                  {
                                                expr* expr1tmp  = $2;
                                                expr* expr2tmp  = $5;

                                                if($5->type == bool_e)
                                                    handleTriada(&expr2tmp);

                                                expr1tmp->index = expr2tmp;
                                                $$ = expr1tmp;

                                                fprintf(syntax_file, "indexedelem: { Exprension : Exprension } \n");
                                            }
               ;

block: '{'                                  {scope++;}
            zero_or_more_statements
        '}'                                 {
                                                symbolTable->hide(scope);
                                                scope--;
                                            }
	     ;

id_optional: ID                             {
                                                handleFunctionId($1);
                                                $$ = $1;
                                            }
	        | %empty                        {
                                                $$ = newTemporaryFunctionName();
                                            }
	        ;

funcprefix: FUNCTION id_optional            {
                                                //Reset the loop counter
                                                loopCounterStack.push(br_cont_counter_allowed);
                                                br_cont_counter_allowed = 0;

                                                function_curr = handleFunctionPrefix($2);

                                                functions_stack.push(function_curr);

                                                functionJumpStack.push(nextQuadLabel());

                                                //sth print +1 to index einai neo pedio,to line einai to yyline
                                                emit(jump , nullptr , nullptr , nullptr , 0 , nextQuadLabel());

                                                emit(funcstart , new expr(function_curr) , nullptr , nullptr , 0 , yylineno);

                                                functionLocalOffsetStack.push(functionLocalOffset);

                                                enterScopeSpace();

                                                resetFormalArgOffset();

                                                $$ = function_curr;
                                            };

funcargs: '('                               {scope++;}
           idlist                           {
                                                enterScopeSpace();
                                                handleFunctionInsert();
                                                resetFunctionLocalOffset();
                                            }
          ')'                               {
                                                scope--;
                                                return_list.push(std::list<unsigned int>());
                                            }
          ;

funcbody: block                             {
                                                exitScopeSpace();
                                            };

funcdef: funcprefix funcargs funcbody       {
                                                exitScopeSpace();

                                                $1->totalLocal = functionLocalOffset;

                                                if(!(return_list.top().empty()))
                                                {
                                                    for(auto x:return_list.top())
                                                    {
                                                        patchLabel(x,nextQuadLabel());
                                                    }
                                                }else
                                                {
                                                  //dimos
                                                  $1->function_has_return = false;
                                                }
                                                return_list.pop();

                                                functionLocalOffset = functionLocalOffsetStack.top();
                                                functionLocalOffsetStack.pop();

                                                emit(funcend , new expr($1) , nullptr , nullptr , 0 , yylineno);

                                                patchLabel(functionJumpStack.top() , nextQuadLabel());
                                                functionJumpStack.pop();

                                                functions_stack.pop();

                                                br_cont_counter_allowed = loopCounterStack.top();
                                                loopCounterStack.pop();

                                                $$ = $1;
                                            };

const: NUMBER                               {
                                                $$ = new expr($1);

                                                fprintf(syntax_file, "const: [ Number -> %d] \n" , $1);
                                            }
        | REAL                              {
                                                $$ = new expr(constreal_e , $1);

                                                fprintf(syntax_file, "const: [ Real -> %lf] \n" , $1);
                                            }
        | STRING                            {
                                                $$ = new expr(conststring_e , std::string($1));

                                                fprintf(syntax_file, "const: [ String -> %s] \n" , $1);
                                            }
        | NIL                               {
                                                $$ = new expr(nil_e , "\"nil\"");

                                                fprintf(syntax_file, "const: [ Nil ] \n");
                                            }
        | TRUE                              {
                                                $$ = new expr(constbool_e , true);

                                                fprintf(syntax_file, "const: [ True ] \n");
                                            }
        | FALSE                             {
                                                $$ = new expr(constbool_e , false);

                                                fprintf(syntax_file, "const: [ False ] \n");
                                            }
        ;

idlist: ID                                  {
                                                addFunctionArgument($1);
                                            }
        list_comma_ID                       {   fprintf(syntax_file, "idlist: [ ID , list_comma_Id ] \n"); }
	    | %empty
	    ;

list_comma_ID:  list_comma_ID ',' ID        {
                                                addFunctionArgument($3);
                                            }
		        | %empty
		        ;

ifprefix: IF '(' expr ')'                   {
                                                expr* exprtmp = $3;

                                                if($3->type != bool_e)
                                                {
                                                    emit(if_eq  ,nullptr , $3 , new expr(constbool_e,true) , nextQuadLabel() + 2 ,yylineno);

                                                    $$ = nextQuadLabel();
                                                    //tobepatched
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }else
                                                {
                                                    handleTriada(&exprtmp);

                                                    emit(if_eq , nullptr , exprtmp , new expr(constbool_e,true) , nextQuadLabel() + 2 , yylineno);

                                                    $$=nextQuadLabel();
                                                    //tobepatched
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }
                                            }
          ;

elseprefix: ELSE                            {
                                                $$ = nextQuadLabel();
                                                //tobepatched
                                                emit( jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                            }
            ;

ifstmt: ifprefix stmt                       {
                                                patchLabel($1,nextQuadLabel());

                                                fprintf(syntax_file, "ifstmt: [ If ( Exprension ) Statement ] \n");
                                            }
        ;

ifstmt: ifprefix stmt elseprefix stmt       {
                                                patchLabel($1 , $3+1);
                                                patchLabel($3 , nextQuadLabel());

                                                fprintf(syntax_file, "ifstmt: [ If ( Exprension ) Statement ELSE Statement ] \n");
                                            }
        ;

whilestart: WHILE                           {
                                              $$ = nextQuadLabel();
                                            }
            ;

whilecond: '(' expr ')'                     {
                                                expr* exprtmp = $2;

                                                br_cont_counter_allowed++;

                                                break_list.push(std::list<unsigned int>());
                                                continue_list.push(std::list<unsigned int>());

                                                if($2->type != bool_e)
                                                {
                                                    emit(if_eq , nullptr , exprtmp , new expr(constbool_e,true) , nextQuadLabel() + 2 , yylineno);

                                                    $$ = nextQuadLabel();
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }else
                                                {
                                                    handleTriada(&exprtmp);

                                                    emit(if_eq , nullptr , exprtmp , new expr(constbool_e,true) , nextQuadLabel() + 2 , yylineno);

                                                    $$ = nextQuadLabel();
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }
                                            }
            ;

whilestmt:  whilestart whilecond stmt       {
                                              br_cont_counter_allowed--;

                                              emit(jump , nullptr , nullptr , nullptr , $1 , yylineno);
                                              patchLabel($2,nextQuadLabel());

                                              if(!(break_list.top().empty()))
                                              {
                                                for(auto x:break_list.top())
                                                {
                                                    patchLabel(x,nextQuadLabel());
                                                }
                                              }

                                              if(!(continue_list.top().empty()))
                                              {
                                                for(auto x:continue_list.top())
                                                {
                                                    patchLabel(x,$1);
                                                }
                                              }

                                              break_list.pop();
                                              continue_list.pop();

                                              fprintf(syntax_file, "whilestmt: [ While ( Exprension ) Statement ] \n");
                                            }
            ;

N: %empty                                   {
                                                $$ = nextQuadLabel();
                                                emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                            }
    ;

fortest: %empty                             {
                                                $$ = nextQuadLabel();
                                            }
        ;

forprefix: FOR
           '('                              {br_cont_counter_allowed++;}
           elist';'
           fortest
           expr';'                          {
                                                expr* exprtmp = $7;

                                                break_list.push(std::list<unsigned int>());
                                                continue_list.push(std::list<unsigned int>());

                                                if($7->type != bool_e)
                                                {
                                                    $$ = new forloop($6 , nextQuadLabel());

                                                    emit(if_eq , nullptr , $7 , new expr(constbool_e , true) , 0 , yylineno);
                                                }
                                                else
                                                {
                                                    handleTriada(&exprtmp);

                                                    $$ = new forloop($6 , nextQuadLabel());
                                                    emit(if_eq , nullptr , exprtmp , new expr(constbool_e , true) , 0 , yylineno);
                                                }
                                            }
            ;

forstmt: forprefix N elist ')' N stmt N     {
                                                br_cont_counter_allowed--;

                                                patchLabel($1->enter , $5 + 1);
                                                patchLabel($2 , nextQuadLabel());
                                                patchLabel($5 , $1->test);
                                                patchLabel($7 , $2 + 1);

                                                if(!(break_list.top().empty()))
                                                {
                                                    for(auto x:break_list.top())
                                                    {
                                                        patchLabel(x,nextQuadLabel());
                                                    }
                                                }

                                                if(!(continue_list.top().empty()))
                                                {
                                                    for(auto x:continue_list.top())
                                                    {
                                                        patchLabel(x,$2 + 1);
                                                    }
                                                }

                                                break_list.pop();
                                                continue_list.pop();
                                            }
        ;

returnstmt: RETURN expr ';'                 {
                                                expr* exprtmp = $2;

                                                if(functions_stack.empty()){
                                                    ERROR(" [return] must be in a function ",yylineno);
                                                    exit(0);
                                                }
                                                else
                                                {
                                                    if($2->type != bool_e)
                                                    {
                                                        emit(ret , $2 , nullptr , nullptr , 0 , yylineno);
                                                        return_list.top().push_back(nextQuadLabel());
                                                        emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                    }
                                                    else
                                                    {
                                                        handleTriada(&exprtmp);

                                                        emit(ret, exprtmp , nullptr , nullptr , nextQuadLabel() , yylineno);
                                                        return_list.top().push_back(nextQuadLabel());
                                                        emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                    }
                                                }

                                                fprintf(syntax_file , "returnstmt: [ Return [Exprension]; ] \n");
                                            }
             ;

returnstmt: RETURN ';'                      {
                                                if(functions_stack.empty()){
                                                    ERROR(" [return] must be in a function ",yylineno);
                                                    exit(0);
                                                }
                                                else
                                                {
                                                    emit(ret , nullptr , nullptr , nullptr , nextQuadLabel() , yylineno);
                                                    return_list.top().push_back(nextQuadLabel());
                                                    emit(jump , nullptr , nullptr , nullptr , 0 , yylineno);
                                                }

                                                fprintf(syntax_file ,"returnstmt: [ Return [Exprension]; ] \n");
                                            }
             ;

%%

int yyerror(char const* yaccProvidedMessage)
{
    std::cout << yaccProvidedMessage << std::endl;
}

int main (int argc , char** argv)
{
    if(argc == 1)
    {
        yyin = stdin;
        yyout = stdout;
    }
    else if(argc == 2)
    {
        if(!(yyin = fopen(argv[1] , "r")))
        {
            fprintf(stderr , "[ERROR] : Cannot read file [%s] \n", argv[1]);
            return 1;
        }

        yyout = stdout;
    }
    else if(argc == 3)
    {
        if(!(yyin = fopen(argv[1] , "r")))
        {
            fprintf(stderr , "[ERROR] : Cannot read file [%s] \n", argv[1]);
            return 1;
        }

        if(!(yyout = fopen(argv[2] , "w")))
        {
            fprintf(stderr , "[ERROR] : Cannot read file [%s] \n" , argv[2]);
            return 1;
        }
    }

    symbolTable->insert(new SymbolTableEntry("print" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("input" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("objectmemberkeys" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("objecttotalmembers" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("objectcopy" , LIB_FUNCTION , libraryfunc_s,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("totalarguments" , LIB_FUNCTION , libraryfunc_s,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("argument" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("typeof" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("strtonum" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("sqrt" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("cos" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));
    symbolTable->insert(new SymbolTableEntry("sin" , LIB_FUNCTION ,libraryfunc_s ,0 , 0 , 1));

    syntax_file = fopen("syntax.txt","w");

    yyparse();

    symbolTable->print();

    printQuads();

    generateInstructions();

    printInstructionsToText();
    printInstructionsToBinary();

    return 0;
}

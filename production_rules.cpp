#include "production_rules.h"
#include "error_output.hpp"
#include <map>
#include <assert.h>
#include "llvm_gen.hpp"
#include "bp.hpp"
#include "plog/include/plog/Log.h"
#include "plog/include/plog/Initializers/RollingFileInitializer.h"



llvmGen& llvm_inst = llvmGen::instance();

size_t loop_counter = 0;

extern Dict types_dict;
extern vector<SymbolTable> tables_stack;
extern vector<int> offsets_stack;
extern int yylineno;
extern std::map<type_t, string> CFanToLlvmTypesMap;

void startCompiler()
{
    std::remove("log.txt");
    plog::init(plog::debug, "log.txt");
    PLOGI << "Start compiler";
    openScope();
    llvm_inst.genInitialFuncs();
}

void endCompiler()
{
    #ifndef SEMANTIC
    PLOGI << "Print global buffer";
    CodeBuffer::instance().printGlobalBuffer();
    PLOGI << "Print code buffer";
    CodeBuffer::instance().printCodeBuffer();
    #endif
    closeScope();
    PLOGI << "End compiler";
}

/*================ Production rules implementation ===================*/

// FuncDecl -> RetType ID LP Formals RP LB Statements RB
FuncDecl::FuncDecl(RetType* ret_type, Node* id, Formals* formals)
{
    ASSERT_3ARGS(ret_type, id, formals);
    vector<type_t> args;

    // Semantic analysis
    for (const auto& elem : formals->m_formal_decls){
        auto type = elem->m_type;
        args.push_back(type);
    }
    if (findIdentifier(id->lexeme)){
        ERROR(output::errorDef(id->lineno, id->lexeme));
    }
    assert(tables_stack.size());
    tables_stack.back().addFuncEntry(id->lexeme, ret_type->m_type, args);
    openScope();
    for(int i = 1; i <= formals->m_formal_decls.size(); i++){
        const string& name = formals->m_formal_decls[i-1]->m_name;
        type_t type = formals->m_formal_decls[i-1]->m_type;
        int offset = -i;
        tables_stack.back().addArgEntry(name, type, offset);
    }

    // llvm generation
    llvm_inst.genFuncDecl(ret_type->m_type, id->lexeme, args);
    llvm_inst.incIdentation();
    //Allocaing room on stack for local variables
    llvm_inst.llvmEmit("%frame_ptr = alloca i32, i32 " + std::to_string(llvm_inst.maxNumOfVars), "Allocating local variables");
    llvm_inst.llvmEmit("");
}

// RetType -> Type
RetType::RetType(Type* type){
    ASSERT_ARG(type);
    m_type = type->m_type;
}

// RetType -> VOID
RetType::RetType(Node* type){
    ASSERT_ARG(type);
    m_type = VOID_T;
}

// Type -> INT/BYTE/BOOL
Type::Type(Node* type){
    ASSERT_ARG(type);
    PLOGD<<"type->token_type: "<< type->token_type;
    PLOGD<<"types_dict[type->token_type]: "<< types_dict[type->token_type];
    m_type = types_dict[type->token_type];
}

// FormalDecl -> Type ID
FormalDecl::FormalDecl(Type* type, Node* id){
    ASSERT_2ARGS(type, id);
    m_name = id->lexeme;
    m_type = type->m_type;
    lineno = id -> lineno;
} 

// FormalsList -> FormalDecl
FormalsList::FormalsList(FormalDecl* formal_decl)
{
    ASSERT_ARG(formal_decl);
    m_formal_decls.push_back(formal_decl);
}

// FormalsList -> FormalDecl COMMA FormalsList
FormalsList::FormalsList(FormalDecl* formal_decl, FormalsList* formal_decls)
{   
    ASSERT_2ARGS(formal_decl, formal_decls);
    for(int i = 0; i < formal_decls->m_formal_decls.size(); i++){
        if(formal_decls->m_formal_decls[i]->m_name == formal_decl->m_name){
            ERROR(output::errorDef(formal_decl->lineno, formal_decl->m_name));
        }
    }
    m_formal_decls.push_back(formal_decl);
    auto& vec = formal_decls->m_formal_decls;
    m_formal_decls.insert( m_formal_decls.end(), vec.begin(), vec.end());
}

// Formals -> FormalsList
Formals::Formals(FormalsList* formal_list) : m_formal_decls(formal_list->m_formal_decls){}

// ExpList -> Exp
ExpList::ExpList(Exp* exp)
{
    ASSERT_ARG(exp);
    lineno = exp->lineno;
    m_exp_list.push_back(exp);
}

// ExpList -> Exp CommaMarker ExpList 
ExpList::ExpList(Exp* exp, CommaMarker* comma_marker, ExpList* exp_list)
{
    ASSERT_3ARGS(exp, comma_marker, exp_list);
    lineno = exp_list->lineno;

    exp_list->m_exp_list.front()->m_label = comma_marker->m_label;

    m_exp_list = exp_list->m_exp_list;
    m_exp_list.insert(m_exp_list.begin(), exp);

}

// Call -> ID LPAREN ExpList RPAREN
Call::Call(Node* func_id, ExpList* exp_list)
{
    ASSERT_2ARGS(func_id, exp_list);

    //Semantic analysis
    lineno = exp_list->lineno;
    const SymbolTableEntry* id_p = findIdentifier(func_id->lexeme);
    //id doesnt exist or it's not a function
    if(!id_p || id_p->m_symbol_type != FUNC){
        ERROR(output::errorUndefFunc(yylineno, func_id->lexeme));
    }
    m_type = id_p->m_ret_type;

    bool is_error = false;
    if(exp_list->m_exp_list.size() != id_p->m_args_types.size()){
        is_error = true;
    }
    vector<string> f_args_types_string;
    for (int i = 0; i < id_p->m_args_types.size(); i++){
        type_t curr_type = id_p->m_args_types[i]; 
        f_args_types_string.push_back(types_dict[curr_type]);
        type_t src_type = INT_T;
        if(exp_list->m_exp_list.size() > i){
            src_type = exp_list->m_exp_list[i]->m_type;
        }
        type_t dst_type = id_p->m_args_types[i];
        if(!automaticCastValidity(dst_type, src_type)){
            is_error = true;
        }
    }
    if(is_error){
        ERROR(output::errorPrototypeMismatch(yylineno, func_id->lexeme, f_args_types_string));
    }

    //llvm generation

    vector<string> reg_args;
    vector<pair<int, BranchLabelIndex>> prev_loc_br;
    for (auto exp : exp_list->m_exp_list){
        CodeBuffer::instance().bpatch(prev_loc_br, exp->m_label);
        prev_loc_br.clear();
        if(exp->m_type == BOOL_T){
            reg_args.push_back(llvm_inst.genBoolExpVal(exp->m_true_list, exp->m_false_list));
            pair<int, BranchLabelIndex> item;
            llvm_inst.genUncondBranch(item);
            prev_loc_br = CodeBuffer::merge(prev_loc_br, CodeBuffer::makelist(item));
        }
        else{
            reg_args.push_back(exp->m_reg);
        }
    }
    if (!prev_loc_br.empty()){
        CodeBuffer::instance().bpatch(prev_loc_br, CodeBuffer::instance().genLabel());
    }
    m_ret_reg = llvm_inst.genCallFunc(func_id->lexeme, reg_args);
}

// Call -> ID LP RP
Call::Call(Node* func_id)
{
    ASSERT_ARG(func_id);
    
    //Semantic analysis
    lineno = func_id->lineno;
    const SymbolTableEntry* id_p = findIdentifier(func_id->lexeme);
    //id doesnt exist or it's not a function
    if(!id_p || id_p->m_symbol_type != FUNC){
        ERROR(output::errorUndefFunc(yylineno, func_id->lexeme));
    }
    m_type = id_p->m_ret_type;

    //llvm generation
    m_ret_reg = llvm_inst.genCallFunc(func_id->lexeme);
}

// Statements -> Statement
Statements::Statements(Statement* statement)
{
    ASSERT_ARG(statement);
    m_statement_list.clear();
    m_statement_list.push_back(statement);

    //llvm generaion
    m_next_list = statement->m_next_list;
    m_break_list = statement->m_break_list;
    m_continue_list = statement->m_continue_list;
}

// Statements -> Statements NextInstMarker Statement
Statements::Statements(Statements* statements, NextInstMarker* nextinst_marker, Statement* statement)
{
    ASSERT_3ARGS(statements, nextinst_marker, statement);
    statement->m_label = nextinst_marker->m_label;
    m_statement_list.push_back(statement);

    //Backpatching nextlist of statements with NextInstMarker's new label before the next statement.
    CodeBuffer::instance().bpatch(statements->m_next_list, nextinst_marker->m_label);

    m_next_list = CodeBuffer::merge(m_next_list, statement->m_next_list);
    m_break_list = CodeBuffer::merge(statements->m_break_list, statement->m_break_list);
    m_continue_list = CodeBuffer::merge(statements->m_continue_list, statement->m_continue_list);
}

// Statement -> LBRACE Statements RBRACE
Statement::Statement(Statements* statements){
    //llvm generation
    m_statement_type = BLOCK_STATEMENT;
    m_next_list = statements->m_next_list;
    m_break_list = statements->m_break_list;
    m_continue_list = statements->m_continue_list;
}

// Statement -> Type ID SC 
Statement::Statement(Type* type, Node* id)
{
    ASSERT_2ARGS(type, id);
    // Semantic analysis
    if(findIdentifier(id->lexeme)){
        ERROR(output::errorDef(id->lineno,id->lexeme));
    }
    assert(tables_stack.size());
    tables_stack.back().addVarEntry(id->lexeme, type->m_type);

    //llvm generation
    m_statement_type = DECALARATION_STATEMENT;
    llvm_inst.genStoreValInVar(id->lexeme, "", /*initial*/ true);
}

// Statement -> Type ID ASSIGN Exp SC 
Statement::Statement(Type* type, Node* id, Exp* exp)
{
    ASSERT_3ARGS(type, id, exp);
    if(!automaticCastValidity(type->m_type, exp->m_type)){
        PLOGD << id->lexeme << " type: " << types_dict[type->m_type] <<
                    ", " << exp->lexeme << " type: " << std::to_string(exp->m_type);
        ERROR(output::errorMismatch(yylineno));
    }
    if(findIdentifier(id->lexeme)){
        ERROR(output::errorDef(id->lineno,id->lexeme));
    }
    assert(tables_stack.size());
    tables_stack.back().addVarEntry(id->lexeme, type->m_type);

    //llvm generation    
    m_statement_type = ASSIGNMENT_STATEMENT;
    if (exp->m_type == BOOL_T){
        llvm_inst.llvmEmit("", "Computing boolean value of " + id->lexeme);
        exp->m_reg = llvm_inst.genBoolExpVal(exp->m_true_list, exp->m_false_list);
    }
    llvm_inst.genStoreValInVar(id->lexeme, llvm_inst.genCasting(exp->m_reg, exp->m_type, type->m_type) );
}

// Statement -> AUTO ID ASSIGN Exp SC
Statement::Statement(Node* auto_token, Node* id, Exp* exp)
{
    ASSERT_3ARGS(auto_token, id, exp);
    if(findIdentifier(id->lexeme)){
        ERROR(output::errorDef(yylineno,id->lexeme));
    }
    if(exp->m_type == VOID_T || exp->m_type == STRING_T){
        ERROR(output::errorMismatch(yylineno));
    }
    assert(tables_stack.size());
    tables_stack.back().addVarEntry(id->lexeme, exp->m_type);

    //llvm generation
    m_statement_type = ASSIGNMENT_STATEMENT;
    if (exp->m_type == BOOL_T){
        exp->m_reg = llvm_inst.genBoolExpVal(exp->m_true_list, exp->m_false_list);
    }
    llvm_inst.genStoreValInVar(id->lexeme, exp->m_reg);
}

// Statement -> ID ASSIGN Exp SC
Statement::Statement(Node* id, Exp* exp)
{
    ASSERT_2ARGS(id, exp);
    auto var = findIdentifier(id->lexeme);

    if(!var || var->m_symbol_type == symbol_type_t::FUNC){
        ERROR(output::errorUndef(yylineno, id->lexeme));
    }
    assert(var->m_symbol_type != symbol_type_t::FUNC);
    if (!automaticCastValidity(var->m_type, exp->m_type)){
            ERROR(output::errorMismatch(yylineno));
    }

    //llvm generation
    m_statement_type = ASSIGNMENT_STATEMENT;
    if (exp->m_type == BOOL_T){
        exp->m_reg = llvm_inst.genBoolExpVal(exp->m_true_list, exp->m_false_list);
    }
    llvm_inst.genStoreValInVar(id->lexeme, llvm_inst.genCasting(exp->m_reg, exp->m_type, var->m_type) );

}

// Statement -> Call SC
Statement::Statement(Call* call)
{   
    ASSERT_ARG(call);
    // llvm generation
    m_statement_type = CALL_STATEMENT;
}

//Statement -> IF LPAREN Exp RPAREN IfWhileMarker Statement
Statement::Statement(Exp* bool_exp, IfWhileMarker* if_while_marker, Statement* statement){
    ASSERT_3ARGS(bool_exp, if_while_marker, statement);
    // llvm generation
    m_statement_type = IF_STATEMENT;
    // if(!(statement->m_statement_type == BLOCK_STATEMENT ||
    //      statement->m_statement_type == RETURN_STATEMENT ||
    //      statement->m_statement_type == IF_STATEMENT ||
    //      statement->m_statement_type == WHILE_STATEMENT ||
    //      statement->m_statement_type == BREAK_STATEMENT ||
    //      statement->m_statement_type == CONTINUE_STATEMENT)){
    //     pair<int, BranchLabelIndex> next_list_item;
    //     llvm_inst.genUncondBranch(next_list_item);
    //     m_next_list = CodeBuffer::merge(m_next_list, CodeBuffer::makelist(next_list_item));
    // }
    CodeBuffer::instance().bpatch(bool_exp->m_true_list, if_while_marker->m_label);
    m_next_list = CodeBuffer::merge(m_next_list, bool_exp->m_false_list);
    m_next_list = CodeBuffer::merge(m_next_list, statement->m_next_list);
    // m_next_list = CodeBuffer::merge(m_next_list, statement->m_break_list);
    m_break_list = statement->m_break_list;
    m_continue_list = statement->m_continue_list;
}

// Statement -> WHILE LP WhileMarker Exp RP IfWhileMarker Statement 
Statement::Statement(WhileMarker* while_marker, Exp* bool_exp, IfWhileMarker* if_while_marker, Statement* statement){
    ASSERT_4ARGS(while_marker, bool_exp, if_while_marker, statement);
    // llvm generation
    m_statement_type = WHILE_STATEMENT;
    // if(!(statement->m_statement_type == BLOCK_STATEMENT ||
    //      statement->m_statement_type == RETURN_STATEMENT ||
    //      statement->m_statement_type == IF_STATEMENT ||
    //      statement->m_statement_type == WHILE_STATEMENT ||
    //      statement->m_statement_type == BREAK_STATEMENT ||
        //  statement->m_statement_type == CONTINUE_STATEMENT)){
        PLOGI << "statement type is " << statement->m_statement_type;
        pair<int, BranchLabelIndex> next_list_item;
        llvm_inst.genUncondBranch(next_list_item, "jump to while condition");
        statement->m_next_list = CodeBuffer::merge(statement->m_next_list, CodeBuffer::makelist(next_list_item));
    // }
    CodeBuffer::instance().bpatch(statement->m_continue_list, while_marker->m_label);
    CodeBuffer::instance().bpatch(bool_exp->m_true_list, if_while_marker->m_label);
    CodeBuffer::instance().bpatch(statement->m_next_list, while_marker->m_label);
    m_next_list = CodeBuffer::merge(m_next_list, bool_exp->m_false_list);
    m_next_list = CodeBuffer::merge(m_next_list, statement->m_break_list);
    m_break_list.clear();
    m_continue_list.clear();

}

// Statement -> IF LP Exp RP IfWhileMarker Statement Else ElseMarker Statement
Statement::Statement(Exp* boo_exp, IfWhileMarker* if_marker, Statement* if_statement, ElseMarker* else_marker, Statement* else_statement){
    ASSERT_5ARGS(boo_exp, if_marker, if_statement, else_marker, else_statement);
    // llvm generation
    m_statement_type = IFELSE_STATEMENT;
    pair<int, BranchLabelIndex> next_list_item;
    llvm_inst.genUncondBranch(next_list_item);
    if_statement->m_next_list = CodeBuffer::merge(if_statement->m_next_list, else_marker->m_next_list);
    else_statement->m_next_list = CodeBuffer::merge(else_statement->m_next_list, CodeBuffer::makelist(next_list_item));
    CodeBuffer::instance().bpatch(boo_exp->m_true_list, if_marker->m_label);
    CodeBuffer::instance().bpatch(boo_exp->m_false_list, else_marker->m_label);
    m_next_list = CodeBuffer::merge(m_next_list, if_statement->m_next_list);
    m_next_list = CodeBuffer::merge(m_next_list, else_statement->m_next_list);
    m_break_list = CodeBuffer::merge(if_statement->m_break_list, else_statement->m_break_list);
    m_continue_list = CodeBuffer::merge(if_statement->m_continue_list, else_statement->m_continue_list);
}

//Statement -> RETURN SC | BREAK SC | CONTINUE SC
Statement::Statement(Node* node){
    ASSERT_ARG(node);
    if(node->token_type == "RETURN"){
        m_statement_type = RETURN_STATEMENT;
        SymbolTableEntry curr_func = tables_stack[0].m_entries.back();
        if(curr_func.m_ret_type != VOID_T){
            ERROR(output::errorMismatch(yylineno));
        }

        //llvm generation
        llvm_inst.llvmEmit("ret void");
    }
    else if(node->token_type == "BREAK"){
        m_statement_type = BREAK_STATEMENT;
        if (loop_counter == 0){
            ERROR(output::errorUnexpectedBreak(yylineno));
        }

        // llvm generation
        llvm_inst.llvmEmit("", "Jumping to statement after while(found break)");
        pair<int, BranchLabelIndex> break_list_item;
        llvm_inst.genUncondBranch(break_list_item);
        m_break_list = CodeBuffer::makelist(break_list_item);

    }else{
        assert (node->token_type == "CONTINUE");
        m_statement_type = CONTINUE_STATEMENT;
        if (loop_counter == 0){
            ERROR(output::errorUnexpectedContinue(yylineno));
        }

        // llvm generation
        llvm_inst.llvmEmit("", "Jumping to while condition(found continue)");
        pair<int, BranchLabelIndex> cont_list_item;
        llvm_inst.genUncondBranch(cont_list_item);
        m_continue_list = CodeBuffer::makelist(cont_list_item);
    }  
}

//Statement -> RETURN Exp SC
Statement::Statement(Exp* exp){
    ASSERT_ARG(exp);
    m_statement_type = RETURN_STATEMENT;
    SymbolTableEntry curr_func = tables_stack[0].m_entries.back();
    const SymbolTableEntry* exp_p = findIdentifier(exp->lexeme); 
    if(exp_p && exp_p->m_symbol_type == FUNC){
        ERROR(output::errorUndef(yylineno, curr_func.m_name));
    }
    if(!automaticCastValidity(curr_func.m_ret_type, exp->m_type)){
        ERROR(output::errorMismatch(yylineno));
    }
    

    //llvm generation
    if (exp->m_type == BOOL_T){
        exp->m_reg = llvm_inst.genBoolExpVal(exp->m_true_list, exp->m_false_list);
    }

    string to_emit = "ret " + CFanToLlvmTypesMap[exp->m_type] + " " + 
            llvm_inst.genCasting(exp->m_reg, exp->m_type, curr_func.m_ret_type);
    llvm_inst.llvmEmit(to_emit);
}

// Exp -> Call
Exp::Exp(Call* call){
    ASSERT_ARG(call);
    lineno = call->lineno;
    m_type = call->m_type;
    
    //llvm generation
    m_reg = call->m_ret_reg;
    if(m_type == BOOL_T){
        pair<int,BranchLabelIndex> true_list_item, false_list_item;
        llvm_inst.genCondBranch(m_reg, true_list_item, false_list_item);
        m_true_list = CodeBuffer::makelist(true_list_item);
        m_false_list = CodeBuffer::makelist(false_list_item);  
    }
    
}

// Exp -> LP Exp RP
Exp::Exp(Node* LP, Exp* other_exp, Node* RP){
    ASSERT_ARG(other_exp);
    lineno = other_exp->lineno;
    m_type = other_exp->m_type;

    //llvm generation
    m_reg = other_exp->m_reg;
    m_false_list = other_exp->m_false_list;
    m_true_list = other_exp->m_true_list;
}

// Bool_Exp -> Exp
Exp::Exp(Exp* bool_exp){
    ASSERT_ARG(bool_exp);
    if (bool_exp->m_type != BOOL_T){
        ERROR(output::errorMismatch(bool_exp->lineno));
    }

    m_type = BOOL_T;
    //llvm generation
    m_reg = bool_exp->m_reg;
    m_true_list = bool_exp->m_true_list;
    m_false_list = bool_exp->m_false_list;  
    // PLOGD << "bool_exp->m_true_list.size(): " << bool_exp->m_true_list.size();
    // PLOGD << "bool_exp->m_false_list.size(): " << bool_exp->m_false_list.size();
    // llvm_inst.genCondBranch(bool_exp->m_reg, m_true_list[0], m_false_list[0]);
    
}

// Exp -> Exp * Exp, * in {AND, OR}
Exp::Exp(Exp* exp1, Node* node, NextInstMarker* M, Exp* exp2)
{
    ASSERT_4ARGS(exp1, node, M, exp2);
    lineno = exp2->lineno;
    if(node->token_type == "OR" || node->token_type == "AND"){
        m_type = BOOL_T;
        if(exp1->m_type != BOOL_T || exp2->m_type != BOOL_T){
            ERROR(output::errorMismatch(yylineno));            
        }
        
        //llvm generation
        if(node->token_type == "OR"){
            CodeBuffer::instance().bpatch(exp1->m_false_list, M->m_label);
            m_true_list = CodeBuffer::merge(exp1->m_true_list, exp2->m_true_list);
            m_false_list = exp2->m_false_list;
        }
        else{//"AND" case
            CodeBuffer::instance().bpatch(exp1->m_true_list, M->m_label);
            m_true_list = exp2->m_true_list;
            m_false_list = CodeBuffer::merge(exp1->m_false_list, exp2->m_false_list);
        }    
    }
    
} 

// Exp -> Exp * Exp, * in {BINOP_PLUSMINUS, BINOP_MULDIV, RELOP_EQ, RELOP_SIZE}
Exp::Exp(Exp* exp1, Node* node, Exp* exp2)
{
    ASSERT_3ARGS(exp1, node, exp2);
    lineno = exp2->lineno;
    if(node->token_type == "RELOP_EQ" || node->token_type == "RELOP_SIZE"){
        //semantic analysis
        m_type = BOOL_T;
        if(exp1->m_type != INT_T && exp1->m_type != BYTE_T){
            ERROR(output::errorMismatch(yylineno));            
        }
        if(exp2->m_type != INT_T && exp2->m_type != BYTE_T){
            ERROR(output::errorMismatch(yylineno));            
        }

        if (exp1->m_type == INT_T && exp2->m_type == BYTE_T){
            exp2->m_reg = llvm_inst.genCasting(exp2->m_reg, BYTE_T, INT_T);
            exp2->m_type = INT_T;
        }
        if (exp1->m_type == BYTE_T && exp2->m_type == INT_T){
            exp1->m_reg = llvm_inst.genCasting(exp1->m_reg, BYTE_T, INT_T);
            exp1->m_type = INT_T;
        }
        
        //llvm generation
        m_reg = llvm_inst.genCompare(exp1->m_reg, node->lexeme, exp2->m_reg, exp1->m_type);
        pair<int,BranchLabelIndex> true_list_item, false_list_item;
        //cond branch should be here, for boolean expressions without IF as well in order to 
        //support compound boolean expressions with AND and OR.
        llvm_inst.genCondBranch(m_reg, true_list_item, false_list_item);
        m_true_list = CodeBuffer::makelist(true_list_item);
        m_false_list = CodeBuffer::makelist(false_list_item);

    }
    else if(node->token_type == "BINOP_PLUSMINUS" || node->token_type == "BINOP_MULDIV"){
        if(exp1->m_type != BYTE_T && exp1->m_type != INT_T){
            ERROR(output::errorMismatch(yylineno));            
        }
        if(exp2->m_type != BYTE_T && exp2->m_type != INT_T){
            ERROR(output::errorMismatch(yylineno));            
        }
        if(exp1->m_type == BYTE_T && exp2->m_type == BYTE_T){
            m_type = BYTE_T;
        }
        else{
            m_type = INT_T;
        }

        //llvm generation 
        if (exp1->m_type == INT_T && exp2->m_type == BYTE_T){
            exp2->m_reg = llvm_inst.genCasting(exp2->m_reg, BYTE_T, INT_T);
            exp2->m_type = INT_T;
        }
        if (exp1->m_type == BYTE_T && exp2->m_type == INT_T){
            exp1->m_reg = llvm_inst.genCasting(exp1->m_reg, BYTE_T, INT_T);
            exp1->m_type = INT_T;
        }
        m_reg = llvm_inst.genBinop(exp1->m_reg, node->lexeme, exp2->m_reg, exp1->m_type);
    }
} 


// Exp -> *, * in {ID, NUM, STRING, TRUE, FALSE}
Exp::Exp(Node* node){
    PLOGI << "Exp -> *, * in {ID, NUM, STRING, TRUE, FALSE}: " + node->lexeme;
    ASSERT_ARG(node);
    lineno = node->lineno;
    if(node->token_type == "ID"){
        const SymbolTableEntry* id_p = findIdentifier(node->lexeme); 
        if(id_p != nullptr){
            // assert(id_p->m_symbol_type != FUNC);
            this->lexeme = id_p->m_name;
            m_type = id_p->m_type;
            m_reg = llvm_inst.genGetVar(node->lexeme);
            PLOGI << "Register of " << node->lexeme << " is: " << m_reg;
            if(m_type == BOOL_T){
                pair<int,BranchLabelIndex> true_list_item, false_list_item;
                llvm_inst.genCondBranch(m_reg, true_list_item, false_list_item);
                m_true_list = CodeBuffer::makelist(true_list_item);
                m_false_list = CodeBuffer::makelist(false_list_item);
            }
            return;
        }
        else{   
            ERROR(output::errorUndef(yylineno, node->lexeme));      
        }
    }
    else if(node->token_type == "NUM"){
        m_type = INT_T;
        m_reg = llvm_inst.setReg(node->lexeme, INT_T);
    }
    else if(node->token_type == "STRING"){
        m_type = STRING_T;
        string g_str_reg = llvm_inst.genStringReg(node->lexeme);
        string str_len = to_string(node->lexeme.length()-1);
        
        string emit_type = "[" + str_len + " x i8]";
        m_reg = llvm_inst.genGetElementPtr(emit_type, g_str_reg, 0, true);
    }
    else {
        assert(node->token_type == "TRUE" || node->token_type == "FALSE");
        m_type = BOOL_T;
        string boolVal = (node->token_type == "TRUE") ? "1" : "0";
        m_reg = llvm_inst.setReg(boolVal, BOOL_T);
        pair<int,BranchLabelIndex> true_list_item, false_list_item;
        llvm_inst.genCondBranch(m_reg, true_list_item, false_list_item);
        m_true_list = CodeBuffer::makelist(true_list_item);
        m_false_list = CodeBuffer::makelist(false_list_item);
    }
} 

// Exp → NUM B
Exp::Exp(Node* node, Node* B)
{
    ASSERT_2ARGS(node, B);
    lineno = node->lineno;
    m_type = BYTE_T;
    if(stoi(node->lexeme) > MAX_BYTE_SIZE){
        ERROR(output::errorByteTooLarge(yylineno, node->lexeme));
    }
    m_reg = llvm_inst.setReg(node->lexeme, BYTE_T);
}

// Exp -> LP Type RP Exp
Exp::Exp(Type* type, Exp* exp){
    ASSERT_2ARGS(type, exp);
    lineno = exp->lineno;
    if (!explicitCastValidity(type->m_type, exp->m_type)){
        ERROR(output::errorMismatch(yylineno));
    }
    m_type = type->m_type;

    //llvm generation

    m_reg = llvm_inst.genCasting(exp->m_reg, exp->m_type, type->m_type);
    m_true_list = exp->m_true_list;
    m_false_list = exp->m_false_list;
}

// Exp → NOT Exp
Exp::Exp(Node* NOT, Exp* exp){
    ASSERT_2ARGS(NOT, exp);
    lineno = exp->lineno;
    if (exp->m_type != BOOL_T){
        ERROR(output::errorMismatch(yylineno));
    }
    m_type = exp->m_type;

    //llvm generation

    m_true_list = exp->m_false_list;
    m_false_list = exp->m_true_list;
}

// Markers
ElseMarker::ElseMarker() : Marker(){
    pair<int, BranchLabelIndex> list_item; 
    llvm_inst.genUncondBranch(list_item);
    m_label = CodeBuffer::instance().genLabel();
    m_next_list = CodeBuffer::makelist(list_item);
}

WhileMarker::WhileMarker() : Marker(){
    pair<int, BranchLabelIndex> list_item; 
    llvm_inst.genUncondBranch(list_item, "Dummy branch to support while");
    m_label = CodeBuffer::instance().genLabel();
    CodeBuffer::instance().bpatch(CodeBuffer::makelist(list_item), m_label);
}

CommaMarker::CommaMarker() : Marker(){
    pair<int, BranchLabelIndex> list_item; 
    llvm_inst.genUncondBranch(list_item, "jump next exp in expList");
    m_label = CodeBuffer::instance().genLabel();
    CodeBuffer::instance().bpatch(CodeBuffer::makelist(list_item), m_label);    
}

NextInstMarker::NextInstMarker()
{
    Marker();
    pair<int,BranchLabelIndex> next_list_item;
    llvm_inst.genUncondBranch(next_list_item, "defalut jump at each statement's end to the next one");
    m_next_list = CodeBuffer::makelist(next_list_item);
    m_label = CodeBuffer::instance().genLabel();
    CodeBuffer::instance().bpatch(m_next_list, m_label);
}

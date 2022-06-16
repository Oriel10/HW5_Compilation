#include "production_rules.h"
#include "error_output.hpp"
#include <map>
#include <assert.h>
#include "llvm_gen.hpp"
#include "bp.hpp"
#include "plog/include/plog/Log.h"
#include "plog/include/plog/Initializers/RollingFileInitializer.h"



llvmGen& llvm_inst = llvmGen::instance();  //TODO: check if instance is needed 

size_t loop_counter = 0;

extern Dict types_dict;
extern vector<SymbolTable> tables_stack;
extern vector<int> offsets_stack;
extern int yylineno;

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
    PLOGI << "Print global buffer";
    CodeBuffer::instance().printGlobalBuffer();
    PLOGI << "Print code buffer";
    CodeBuffer::instance().printCodeBuffer();
    closeScope();
    PLOGI << "End compiler";
}

/*================ Classes implementation ===================*/

// FuncDecl -> RetType ID LP Formals RP LB Statements RB
FuncDecl::FuncDecl(RetType* ret_type, Node* id, Formals* formals)
{
    ASSERT_3ARGS(ret_type, id, formals);
    vector<type_t> args;

    // Semantic analasis
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
    llvm_inst.llvmEmit("%frame_ptr = alloca i32, i32 " + std::to_string(llvm_inst.maxNumOfVars));
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
    m_type = types_dict[type->token_type];
}

FormalDecl::FormalDecl(Type* type, Node* id){
    ASSERT_2ARGS(type, id);
    m_name = id->lexeme;
    m_type = type->m_type;
} 

FormalsList::FormalsList(FormalDecl* formal_decl)
{
    ASSERT_ARG(formal_decl);
    m_formal_decls.push_back(formal_decl);
}

FormalsList::FormalsList(FormalDecl* formal_decl, FormalsList* formal_decls)
{   
    ASSERT_2ARGS(formal_decl, formal_decls);
    m_formal_decls.push_back(formal_decl);
    auto& vec = formal_decls->m_formal_decls;
    m_formal_decls.insert( m_formal_decls.end(), vec.begin(), vec.end());
}

Formals::Formals(FormalsList* formal_list) : m_formal_decls(formal_list->m_formal_decls){}

// Call -> ID LPAREN ExpList RPAREN
Call::Call(Node* func_id, ExpList* exp_list){
    ASSERT_2ARGS(func_id, exp_list);
    lineno = exp_list->lineno;
    const SymbolTableEntry* id_p = findIdentifier(func_id->lexeme);
    if(id_p->m_symbol_type != symbol_type_t::FUNC){
        ERROR(output::errorUndefFunc(yylineno, id_p->m_name));
    }
    //id doesnt exist or it's not a function
    if(!id_p || id_p->m_symbol_type != FUNC){
        ERROR(output::errorUndefFunc(yylineno, func_id->lexeme));
    }
    m_type = id_p->m_ret_type;

    bool is_error = false;
    if(exp_list->m_exp_list_types.size() != id_p->m_args_types.size()){
        is_error = true;
    }
    vector<string> f_args_types_string;
    for (int i = 0; i < id_p->m_args_types.size(); i++){
        type_t curr_type = id_p->m_args_types[i]; 
        f_args_types_string.push_back(types_dict[curr_type]);
        type_t src_type = INT_T;
        if(exp_list->m_exp_list_types.size() > i){
            src_type = exp_list->m_exp_list_types[i];
        }
        type_t dst_type = id_p->m_args_types[i];
        if(!automaticCastValidity(dst_type, src_type)){
            is_error = true;
        }
    }
    if(is_error){
        ERROR(output::errorPrototypeMismatch(yylineno, func_id->lexeme, f_args_types_string));
    }
}

// Call -> ID LP RP
Call::Call(Node* func_id){
    ASSERT_ARG(func_id);
    lineno = func_id->lineno;
    const SymbolTableEntry* id_p = findIdentifier(func_id->lexeme);
    //id doesnt exist or it's not a function
    if(!id_p || id_p->m_symbol_type != FUNC){
        ERROR(output::errorUndefFunc(yylineno, func_id->lexeme));
    }
    m_type = id_p->m_ret_type;
}

// Statement - >Type ID SC 
Statement::Statement(Type* type, Node* id)
{
    ASSERT_2ARGS(type, id);
    // Semantic Analisis
    if(findIdentifier(id->lexeme)){
        ERROR(output::errorDef(id->lineno,id->lexeme));
    }
    assert(tables_stack.size());
    tables_stack.back().addVarEntry(id->lexeme, type->m_type);

    //llvm generation
    llvm_inst.genAllocVar();
}

// Statement -> Type ID ASSIGN Exp SC 
Statement::Statement(Type* type, Node* id, Exp* exp)
{
    ASSERT_3ARGS(type, id, exp);
    if(!automaticCastValidity(type->m_type, exp->m_type)){
        ERROR(output::errorMismatch(yylineno));
    }
    if(findIdentifier(id->lexeme)){
        ERROR(output::errorDef(id->lineno,id->lexeme));
    }
    assert(tables_stack.size());
    tables_stack.back().addVarEntry(id->lexeme, type->m_type);

    //llvm generation
    llvm_inst.genStoreValInVar(id->lexeme, exp->m_reg);
}

// Statement -> AUTO ID ASSIGN Exp SC
Statement::Statement(Node* auto_token, Node* id, Exp* exp)
{
    ASSERT_3ARGS(auto_token, id, exp);
    if(findIdentifier(id->lexeme)){
        ERROR(output::errorDef(yylineno,id->lexeme));
    }
    if(exp->m_type == VOID_T){
        ERROR(output::errorMismatch(yylineno));
    }
    assert(tables_stack.size());
    tables_stack.back().addVarEntry(id->lexeme, exp->m_type);

    //llvm generation
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
    if (!automaticCastValidity(exp->m_type, var->m_type)){
        ERROR(output::errorMismatch(yylineno));
    }

    //llvm generation
    llvm_inst.genStoreValInVar(id->lexeme, exp->m_reg);
}

// Statement -> Call SC
Statement::Statement(Call* call)
{
    ASSERT_ARG(call);
    // nothing to do
}

//Statement -> IF LPAREN Exp RPAREN Statement
//Statement -> WHILE LPAREN Exp RPAREN Statement
Statement::Statement(Node* node, Exp* exp, Statement* statement){
    //statement arg is just to make this c'tor unique
    ASSERT_3ARGS(node, exp, statement);
    if(exp->m_type != BOOL_T){
        ERROR(output::errorMismatch(exp->lineno));
    }
}

// Statement -> IF LP Exp RP Statement Else Statement
Statement::Statement(Node* node_if, Exp* exp, Node* node_else){
    ASSERT_3ARGS(node_if, exp, node_else);
    if(exp->m_type != BOOL_T){
        ERROR(output::errorMismatch(exp->lineno));
    }
}

//Statement -> RETURN SC | BREAK SC | CONTINUE SC
Statement::Statement(Node* node){
    ASSERT_ARG(node);
    if(node->token_type == "RETURN"){
        SymbolTableEntry curr_func = tables_stack[0].m_entries.back();
        if(curr_func.m_ret_type != VOID_T){
            ERROR(output::errorMismatch(yylineno));
        }       
    }
    else if(node->token_type == "BREAK"){
        if (loop_counter == 0){
            ERROR(output::errorUnexpectedBreak(yylineno));
        }
    }else{
        if (loop_counter == 0){
            ERROR(output::errorUnexpectedContinue(yylineno));
        }
    }
    
}

//Statement -> RETURN Exp SC
Statement::Statement(Exp* exp){
    ASSERT_ARG(exp);
    SymbolTableEntry curr_func = tables_stack[0].m_entries.back();
    const SymbolTableEntry* exp_p = findIdentifier(exp->lexeme); 
    if(exp_p && exp_p->m_symbol_type == FUNC){
        ERROR(output::errorUndef(yylineno, curr_func.m_name));
    }
    if(!automaticCastValidity(curr_func.m_ret_type, exp->m_type)){
        ERROR(output::errorMismatch(yylineno));
    }
}


// 𝐸𝑥𝑝 → 𝐶𝑎𝑙𝑙
Exp::Exp(Call* call){
    ASSERT_ARG(call);
    lineno = call->lineno;
    m_type = call->m_type; 
}

// 𝐸𝑥𝑝𝐿𝑖𝑠𝑡 → 𝐸𝑥𝑝
ExpList::ExpList(Exp* exp)
{
    ASSERT_ARG(exp);
    lineno = exp->lineno;
    m_exp_list_types.push_back(exp->m_type);
}

// 𝐸𝑥𝑝𝐿𝑖𝑠𝑡 → 𝐸𝑥𝑝 𝐶𝑂𝑀𝑀𝐴 𝐸𝑥𝑝𝐿𝑖𝑠𝑡
ExpList::ExpList(Exp* exp, ExpList* exp_list)
{
    ASSERT_2ARGS(exp, exp_list);
    lineno = exp_list->lineno;
    m_exp_list_types.push_back(exp->m_type);
    auto& vec = exp_list->m_exp_list_types;
    m_exp_list_types.insert( m_exp_list_types.end(), vec.begin(), vec.end());
}

// Exp -> LP Exp RP
Exp::Exp(Exp* other_exp){
    ASSERT_ARG(other_exp);
    lineno = other_exp->lineno;
    m_type = other_exp->m_type;
    m_reg = other_exp->m_reg;
}

// Exp -> Exp * Exp, * in {BINOP_PLUSMINUS, BINOP_MULDIV, AND, OR, RELOP_EQ, RELOP_SIZE}
Exp::Exp(Exp* exp1, Node* node, Exp* exp2)
{
    ASSERT_3ARGS(exp1, node, exp2);
    lineno = exp2->lineno;
    if(node->token_type == "OR" || node->token_type == "AND"){
        m_type = BOOL_T;
        if(exp1->m_type != BOOL_T || exp2->m_type != BOOL_T){
            ERROR(output::errorMismatch(yylineno));            
        }
    }
    else if(node->token_type == "RELOP_EQ" || node->token_type == "RELOP_SIZE"){
        m_type = BOOL_T;
        if(exp1->m_type != INT_T && exp1->m_type != BYTE_T){
            ERROR(output::errorMismatch(yylineno));            
        }
        if(exp2->m_type != INT_T && exp2->m_type != BYTE_T){
            ERROR(output::errorMismatch(yylineno));            
        }
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
            m_reg = llvm_inst.genBinop(exp1->m_reg, node->lexeme, exp2->m_reg, BYTE_T);
        }
        else{
            m_type = INT_T;
            m_reg = llvm_inst.genBinop(exp1->m_reg, node->lexeme, exp2->m_reg, INT_T);
        }

    }
} 


// Exp -> *, * in {ID, NUM, STRING, TRUE, FALSE}
Exp::Exp(Node* node){
    PLOGI << "Exp -> *, * in {ID, NUM, STRING, TRUE, FALSE}";
    ASSERT_ARG(node);
    lineno = node->lineno;
    if(node->token_type == "ID"){
        const SymbolTableEntry* id_p = findIdentifier(node->lexeme); 
        if(id_p != nullptr){
            // assert(id_p->m_symbol_type != FUNC);
            this->lexeme = id_p->m_name;
            m_type = id_p->m_type;
            return;
        }    
        ERROR(output::errorUndef(yylineno, node->lexeme));      
    }
    else if(node->token_type == "NUM"){
        m_type = INT_T;
        m_reg = llvm_inst.setReg(node->lexeme, INT_T);
    }
    else if(node->token_type == "STRING"){
        m_type = STRING_T;
        string g_str_reg = llvm_inst.genStringReg(node->lexeme);
        string str_len = to_string(node->lexeme.length()-1);
        m_reg = llvm_inst.getFreshRegister();
        
        string emit_type = "[" + str_len + " x i8]";

        string str_emit = m_reg + " = getelementptr [" + str_len + " x i8], [" + str_len + " x i8]* " + g_str_reg + ", i32 0, i32 0";
        llvm_inst.llvmEmit(str_emit);
    }
    else if(node->token_type == "TRUE"){
        m_type = BOOL_T;
        m_reg = llvm_inst.setReg("1", BOOL_T);
    }
    else{
        assert(node->token_type == "FALSE");
        m_type = BOOL_T;
        m_reg = llvm_inst.setReg("0", BOOL_T);
    }
} 

// 𝐸𝑥𝑝 → 𝑁𝑈𝑀 B
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

// 𝐸𝑥𝑝 → 𝐿𝑃𝐴𝑅𝐸𝑁 𝑇𝑦𝑝𝑒 𝑅𝑃𝐴𝑅𝐸𝑁 𝐸𝑥p
Exp::Exp(Type* type, Exp* exp){
    ASSERT_2ARGS(type, exp);
    lineno = exp->lineno;
    if (!explicitCastValidity(type->m_type, exp->m_type)){
        ERROR(output::errorMismatch(yylineno));
    }
    exp->m_type = type->m_type; 
}

// 𝐸𝑥𝑝 → 𝑁𝑂𝑇 𝐸𝑥p
Exp::Exp(Node* NOT, Exp* exp){
    ASSERT_2ARGS(NOT, exp);
    lineno = exp->lineno;
    if (exp->m_type != BOOL_T){
        ERROR(output::errorMismatch(yylineno));
    }
    m_type = exp->m_type;
}
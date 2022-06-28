#include "semantic_analizer.h"
#include "hw3_output.hpp"
#include <map>
#include <assert.h>
#include <string.h>


extern char* yytext;
extern int yylineno;

Dict types_dict;

//global stacks
vector<SymbolTable> tables_stack;
vector<int> offsets_stack;

void SymbolTableEntry::print() const
{
    string str_type;
    if (m_symbol_type == FUNC){
        vector<string> arg_str_types;
        for (auto type : m_args_types){
            arg_str_types.push_back(types_dict[type]);
        }
        const auto& ret_str_type = types_dict[m_ret_type];
        str_type = output::makeFunctionType(ret_str_type, arg_str_types);
    }
    else{
        str_type = types_dict[m_type];
    }
    output::printID(m_name, m_offset, str_type);
}

void SymbolTable::print() const
{
    for (const auto& entry : m_entries){
        entry.print();
    }
}

void SymbolTable::addFuncEntry(const string& name, type_t ret_type, vector<type_t>& arg_types)
{
    m_entries.push_back(SymbolTableEntry(name, ret_type, arg_types, symbol_type_t::FUNC));
}

void SymbolTable::addVarEntry(const string& name, type_t type)
{
    assert(offsets_stack.size());
    m_entries.push_back(SymbolTableEntry(name, type, offsets_stack.back(), symbol_type_t::VAR));
    offsets_stack.back()++;
}

void SymbolTable::addArgEntry(const string& name, type_t type, int offset)
{
    m_entries.push_back(SymbolTableEntry(name, type, offset, symbol_type_t::ARG));    
}

void openScope()
{   
    
    //Semantic analysis
    //start of the program
    if(tables_stack.empty()){ 
        SymbolTable global;
        vector<type_t> print_args;
        print_args.push_back(STRING_T);
        vector<type_t> printi_args;
        printi_args.push_back(INT_T);
        global.addFuncEntry("print", VOID_T, print_args);
        global.addFuncEntry("printi", VOID_T, printi_args);
        tables_stack.push_back(global);
        offsets_stack.push_back(0);
        return;
    }
    tables_stack.push_back(SymbolTable());
    assert(offsets_stack.size());
    offsets_stack.push_back(offsets_stack.back());

    //llvm generation
}

void closeScope()
{   
    
    //global scope is closed and main is missing
    const SymbolTableEntry* main_entry = findIdentifier("main");
    
    //Program reduced
    if(tables_stack.size() == 1){
        if(strcmp(yytext,"")!=0){
            return;
        }
        if((!main_entry || main_entry->m_ret_type != VOID_T || main_entry->m_args_types.size() > 0 )){
            ERROR(output::errorMainMissing());    
        }
    }
    assert(tables_stack.size());
    assert(offsets_stack.size());
    const auto& table = tables_stack.back();
    
    #ifdef SEMANTIC
    output::endScope();
    table.print();
    #endif

    tables_stack.pop_back();
    offsets_stack.pop_back();
}

const SymbolTableEntry* findIdentifier(const string& name)
{
    for(auto itr = tables_stack.crbegin(); itr != tables_stack.crend(); itr++ ){
        auto& curr_table = *itr;
        for (auto& entry : curr_table.m_entries){
            if (entry.m_name == name){
                return &entry;
            }
        }
    }
    return nullptr;
}

bool explicitCastValidity(type_t dst_type , type_t src_type)
{
    if ((dst_type == src_type) ||
        (dst_type == INT_T && src_type == BYTE_T) ||
         (src_type == INT_T && dst_type == BYTE_T) ){
        return true;
    }
    return false;
}

bool automaticCastValidity(type_t dst_type , type_t src_type)
{
    if ( dst_type == src_type || (dst_type == INT_T && src_type == BYTE_T) ){
        return true;
    }
    return false;
}

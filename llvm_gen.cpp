#include "llvm_gen.hpp"
#include <experimental/iterator>
#include <iterator>
#include <sstream>
#include "plog/include/plog/Log.h"


llvmGen::llvmGen(): m_reg_num(0)
{
    m_cb = &(CodeBuffer::instance());
}

//TODO: Create instance
// llvmGen &llvmGen::instance() {
// 	static llvmGen inst;//only instance
// 	return inst;
// }

std::string llvmGen::genRegister(){
    std::string res_reg = "%var";
    res_reg += std::to_string(m_reg_num++);
    return res_reg;
}

void llvmGen::genFuncDecl(type_t retType, const std::string& funcName, std::vector<type_t> argsTypes) const
{
    PLOGI << "Generaing function declaration: \"" + funcName + "\"";

    std::string ret_type = "";
    switch (retType)
    {
    case type_t::VOID_T:
        ret_type = "void";
        break;
    default:
        //TODO: add iXX by type  (int / byte / bool)
        //TODO: figure out how to add pointer type
        ret_type = "i32";
        break;
    }

    std::vector<std::string> types;
    for (auto& type : argsTypes){
        //TODO: add iXX by type
        types.push_back(std::string("i32"));
    }

    std::ostringstream sperated_args_list;
    std::copy(types.begin(),
              types.end(),
              std::experimental::make_ostream_joiner(sperated_args_list,", "));
    
    m_cb->emitGlobal("declare " + ret_type + " @"  + funcName + "(" + sperated_args_list.str() + ")");
}

void llvmGen::genInitialFuncs() const
{
    PLOGI << "Generating intial functions";
    m_cb->emitGlobal("declare i32 @printf(i8*, ...)");
    genFuncDecl(type_t::VOID_T, "exit", std::vector<type_t>{type_t::INT_T});
    m_cb->emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    m_cb->emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    
    
}


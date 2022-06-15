#include "llvm_gen.hpp"
#include <experimental/iterator>
#include <iterator>
#include <sstream>
#include "plog/include/plog/Log.h"
#include <map>

std::map<type_t, std::string>
 CFanToLlvmTypesMap = {
    {type_t::INT_T, "i32"},
    {type_t::BYTE_T, "i8" },
    {type_t::BOOL_T, "i1"},
    {type_t::VOID_T, "void"}
};



llvmGen::llvmGen(): m_reg_num(0), m_indentation(0)
{
    m_cb = &(CodeBuffer::instance());
}

//TODO: Create instance
// llvmGen &llvmGen::instance() {
// 	static llvmGen inst;//only instance
// 	return inst;
// }

std::string llvmGen::getIdentation()
{
    std::string res = "";
    for (size_t i=0; i < m_indentation; i++){
        res += "\t";
    }
    return res;
}

std::string llvmGen::getFreshRegister(){
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
    
    m_cb->emit("define " + ret_type + " @"  + funcName + "(" + sperated_args_list.str() + ") {");
}

void llvmGen::genInitialFuncs() const
{
    PLOGI << "Generating intial functions";
    m_cb->emitGlobal("declare i32 @printf(i8*, ...)");
    m_cb->emitGlobal("declare void @exit(i32)");
    m_cb->emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    m_cb->emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    m_cb->emit("");
    
    m_cb->emit("define void @printi(i32) {");
    m_cb->emit("%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
    m_cb->emit("call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
    m_cb->emit("ret void");
    m_cb->emit("}");
    m_cb->emit("");

    m_cb->emit("define void @print(i8*) {");
    m_cb->emit("%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
    m_cb->emit("call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
    m_cb->emit("ret void");
    m_cb->emit("}");
}

void llvmGen::genAllocVar(std::string varName)
{
    PLOGI << "Generating alloca commad";
    m_cb->emit("%" + varName + " = alloca i32");
}

void llvmGen::genStoreValInVar(std::string varName, size_t value)
{
    PLOGI << "Generating store command";
    m_cb->emit("store i32 " +std::to_string(value) + ", i32* %" + varName);
}
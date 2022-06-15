#include "llvm_gen.hpp"
#include "semantic_analizer.h"
#include <experimental/iterator>
#include <iterator>
#include <sstream>
#include "plog/include/plog/Log.h"
#include <map>

extern stack<SymbolTable> tables_stack;
 

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
llvmGen &llvmGen::instance() {
	static llvmGen inst;//only instance
	return inst;
}

std::string llvmGen::getIdentation() const
{
    PLOGI << "identation: " + std::to_string(m_indentation);
    std::string res = "";
    for (size_t i=0; i < m_indentation; i++){
        res += "    ";
    }
    return res;
}

int llvmGen::llvmEmit(const string& str) const
{
    PLOGI << "m_indentation: "<< m_indentation;
    return m_cb->emit(getIdentation() + str);
}

std::string llvmGen::getFreshRegister(){
    std::string res_reg = "%var";
    res_reg += std::to_string(m_reg_num++);
    return res_reg;
}

//%var15 = add i32 0, value

void llvmGen::genFuncDecl(type_t retType, const std::string& funcName, std::vector<type_t> argsTypes) const
{
    PLOGI << "Generaing function declaration: \"" + funcName + "\"";

    const std::string& ret_type = CFanToLlvmTypesMap.at(retType);

    std::vector<std::string> types;
    for (auto& type : argsTypes){
        types.push_back(CFanToLlvmTypesMap.at(type));
    }

    std::ostringstream sperated_args_list;
    std::copy(types.begin(),
              types.end(),
              std::experimental::make_ostream_joiner(sperated_args_list,", "));
    
    llvmEmit("define " + ret_type + " @"  + funcName + "(" + sperated_args_list.str() + ") {");
}


void llvmGen::genInitialFuncs() const
{
    PLOGI << "Generating intial functions";

    m_cb->emitGlobal("declare i32 @printf(i8*, ...)");
    m_cb->emitGlobal("declare void @exit(i32)");
    m_cb->emitGlobal("declare i8* @llvm.frameaddress(i32 <level>)");
    m_cb->emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    m_cb->emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    llvmEmit("");
    
    //hard coded printi implementation
    llvmEmit("define void @printi(i32) {");
    llvmEmit("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
    llvmEmit("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
    llvmEmit("    ret void");
    llvmEmit("}");
    llvmEmit("");

    //hard coded print implementation
    llvmEmit("define void @print(i8*) {");
    llvmEmit("    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
    llvmEmit("    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
    llvmEmit("    ret void");
    llvmEmit("}");
    llvmEmit("");
}

void llvmGen::genAllocVar()
{
    PLOGI << "Generating alloca commad";
    llvmEmit(getFreshRegister() + " = alloca i32");
}

void llvmGen::genStoreValInVar(std::string varName, size_t value)
{
    PLOGI << "Generating store command";
    llvmEmit("store i32 " +std::to_string(value) + ", i32* %" + varName);
}

void llvmGen::incIdentation(){
    m_indentation++;
}
void llvmGen::decIdentation(){
    m_indentation--;
}
void llvmGen::zeroIdentation(){
    m_indentation = 0;
}

void llvmGen::closeFunc(){
    zeroIdentation();
    llvmEmit("}");
    llvmEmit("");
}

//TODO: finish
// string llvmGen::genGetVar(string varName)
// {
//     auto& curr_entry = tables_stack.back();
//     int offset = curr_entry.varOffsetByName(varName);
//     llvmEmit("");
// }
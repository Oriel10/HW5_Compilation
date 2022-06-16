#include "llvm_gen.hpp"
#include "semantic_analizer.h"
#include <experimental/iterator>
#include <iterator>
#include <sstream>
#include "plog/include/plog/Log.h"
#include <map>
#include <utility>

extern stack<SymbolTable> tables_stack;
 

std::map<type_t, string>
 CFanToLlvmTypesMap = {
    {type_t::INT_T, "i32"},
    {type_t::BYTE_T, "i8" },
    {type_t::BOOL_T, "i1"},
    {type_t::VOID_T, "void"},
    {type_t::STRING_T, "i8*"}
};

std::map<pair<string, type_t>, string>
 CFanToLlvmOPsMap = {
    {pair<string, type_t>("*", INT_T), "mul i32"},
    {pair<string, type_t>("/", INT_T), "sdiv i32"}, //TODO: when is it udiv?
    {pair<string, type_t>("+", INT_T), "add i32"},
    {pair<string, type_t>("-", INT_T), "sub i32"},
    {pair<string, type_t>("*", BYTE_T), "mul i8"},
    {pair<string, type_t>("/", BYTE_T), "sdiv i8"}, 
    {pair<string, type_t>("+", BYTE_T), "add i8"},
    {pair<string, type_t>("-", BYTE_T), "sub i8"},
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

string llvmGen::setReg(string init_val, type_t val_type){
    assert(val_type != STRING_T);
    string llvm_type = CFanToLlvmTypesMap[val_type];
    string reg = getFreshRegister();
    string assignment = reg + " = add " + llvm_type +" " + init_val + ", 0"; 
    llvmEmit(assignment);
    PLOGI << assignment;
    return reg;
}

string llvmGen::genStringReg(string str){
    string g_reg = getFreshRegister(true);
    int str_len = str.length();
    string res = g_reg + " = constant [" + to_string(str_len) + " x i8], c\"" + str + "\\00\"\n";
    m_cb->emitGlobal(res);
    //maybe emit getelementptr here?
    return g_reg;
}

string llvmGen::genBinop(string reg1, string op, string reg2, type_t op_type){
    string res_reg = getFreshRegister();
    // %vari = op type %reg1, %reg2 
    //TODO: handle dev by 0 - create a label to deal with it?
    string to_emit = res_reg + " = " + CFanToLlvmOPsMap[pair<string, type_t>(op, op_type)] + " " + reg1 + ", " + reg2; 
    llvmEmit(to_emit);
    return res_reg;
}

int llvmGen::llvmEmit(const string& str) const
{
    PLOGI << "m_indentation: "<< m_indentation;
    return m_cb->emit(getIdentation() + str);
}

std::string llvmGen::getFreshRegister(bool is_global){

    std::string res_reg = is_global ? "@var" : "%var";
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
    m_cb->emitGlobal("");
    
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

string llvmGen::genAllocVar()
{
    PLOGI << "Generating alloca commad";
    string res_reg = getFreshRegister(); 
    llvmEmit(res_reg + " = alloca i32");
    return res_reg;
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
    llvmEmit("}\n");
    // llvmEmit("");
}

//TODO: finish
// string llvmGen::genGetVar(string varName)
// {
//     auto& curr_entry = tables_stack.back();
//     int offset = curr_entry.varOffsetByName(varName);
//     llvmEmit("");
// }
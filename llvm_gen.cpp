#include "llvm_gen.hpp"
#include "semantic_analizer.h"
#include <experimental/iterator>
#include <iterator>
#include <sstream>
#include "plog/include/plog/Log.h"
#include <map>
#include <utility>

extern vector<SymbolTable> tables_stack;
 

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
    {pair<string, type_t>("/", INT_T), "sdiv i32"}, 
    {pair<string, type_t>("+", INT_T), "add i32"},
    {pair<string, type_t>("-", INT_T), "sub i32"},
    {pair<string, type_t>("*", BYTE_T), "mul i8"},
    {pair<string, type_t>("/", BYTE_T), "udiv i8"}, 
    {pair<string, type_t>("+", BYTE_T), "add i8"},
    {pair<string, type_t>("-", BYTE_T), "sub i8"},
};


llvmGen::llvmGen(): m_reg_num(0), m_indentation(0)
{
    m_cb = &(CodeBuffer::instance());
}


llvmGen &llvmGen::instance() {
	static llvmGen inst;//only instance
	return inst;
}

std::string llvmGen::getIdentation() const
{
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
    string res = g_reg + " = constant [" + to_string(str_len-1) + " x i8] c\"" + str.substr(1,str.length()-2) + "\\00\"\n";
    m_cb->emitGlobal(res);
    //maybe emit getelementptr here?
    return g_reg;
}

string llvmGen::genBinop(string reg1, string op, string reg2, type_t op_type){
    string res_reg = getFreshRegister();
    // %vari = op type %reg1, %reg2 
    //TODO: handle dev by 0 - create a label to deal with it?
    //%vari = op type %var1, %var2
    string to_emit = res_reg + " = " + CFanToLlvmOPsMap[pair<string, type_t>(op, op_type)] + " " + reg1 + ", " + reg2; 
    llvmEmit(to_emit);
    return res_reg;
}

int llvmGen::llvmEmit(const string& str) const
{
    PLOGI << "emit the command: "<< str;
    return m_cb->emit(getIdentation() + str);
}

std::string llvmGen::getFreshRegister(bool is_global){
    std::string res_reg = is_global ? "@var" : "%var";
    res_reg += std::to_string(m_reg_num++);
    return res_reg;
}

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

string llvmGen::genGetElementPtr(string type, string ptr_reg, unsigned int offset, bool is_aggregate)
{   string res_reg = getFreshRegister();
    //%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0
    string to_emit;

    if(!is_aggregate){//stack usage
        assert(type == "i32");
        to_emit = res_reg + " = getelementptr i32, i32* " + ptr_reg + ", i32 " + to_string(offset);
    }
    else{//string usage
        to_emit = res_reg + " = getelementptr " + type +", " + type + "*" + ptr_reg +  "i32 0, i32 " + to_string(offset);
    }
    llvmEmit(to_emit);
    return res_reg;
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

string llvmGen::genAllocVar()
{
    PLOGI << "Generating alloca commad";
    const auto& var = getFreshRegister();
    llvmEmit(var + " = alloca i32");
    genStore("i32", "0", var);
    // llvmEmit("store i32 0, i32* " + var);
    return var;
}

string llvmGen::genGetVar(const string& varName)
{
    PLOGI << "Generating genGetVar commands";

    auto& curr_table = tables_stack.back();
    int offset = curr_table.GetVarOffsetByName(varName);
    bool is_arg = offset < 0;
    if(is_arg){
        offset = -offset;
        return "%" + to_string(--offset);
    }
    llvmEmit("");
    //%vari = getelementptr i32, i32* %frame_ptr, i32 offset
    string src_ptr = genGetElementPtr("i32" , "%frame_ptr", offset);
    // llvmEmit(ptr + " = getelementptr i32, i32* %frame_ptr, i32 " + std::to_string(offset));
    auto var_value = getFreshRegister();
    //%varj = load i32, i32* %vari
    // llvmEmit(var_value + " = load i32, i32* " + src_ptr);
    genLoad(var_value, "i32", src_ptr);
    
    type_t varType = curr_table.GetVarTypeByName(varName);
    if (varType == type_t::INT_T){
        return var_value;
    }
    else{
        string casted_reg = getFreshRegister();
        llvmEmit(casted_reg + "trunc i32 " + var_value + " to " + CFanToLlvmTypesMap[varType]);
        return casted_reg;
    }
}

void llvmGen::genLoad(string dst_reg, string type, string src_ptr){
    llvmEmit(dst_reg + " = load " + type + ", " + type + "* " + src_ptr);
}

void llvmGen::genStore(string type, string src_val, string dst_ptr){
    llvmEmit("store " + type + " " + src_val + ", " + type + "* " + dst_ptr);
}

//TODO: support diffrenet types
//TODO: check offest
void llvmGen::genStoreValInVar(string varName, string src_reg)
{
    PLOGI << "Generating store command";
    PLOGI << varName << ", " << src_reg;
    PLOGI << "tables_stack size: " << tables_stack.size();

    auto& curr_table = tables_stack.back();
    int offset = curr_table.GetVarOffsetByName(varName);

     PLOGI << "After GetVarOffsetByName func";
    // auto ptr = getFreshRegister();
    string dst_ptr = genGetElementPtr("i32" , "%frame_ptr", offset);
    // llvmEmit(ptr + " = getelementptr i32, i32* %frame_ptr, i32 " + std::to_string(offset));
    // llvmEmit("store i32 " + src_reg + ", i32* " + dst_ptr);
    genStore("i32", src_reg, dst_ptr);
}
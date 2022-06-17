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

string llvmGen::setReg(const string& init_val, type_t val_type){
    assert(val_type != STRING_T);
    string llvm_type = CFanToLlvmTypesMap[val_type];
    string reg = getFreshRegister();
    string assignment = reg + " = add " + llvm_type +" " + init_val + ", 0"; 
    llvmEmit(assignment);
    PLOGI << assignment;
    return reg;
}

string llvmGen::genStringReg(const string& str){
    string g_reg = getFreshRegister(true);
    int str_len = str.length();
    string res = g_reg + " = constant [" + to_string(str_len-1) + " x i8] c\"" + str.substr(1,str.length()-2) + "\\00\"\n";
    m_cb->emitGlobal(res);
    //maybe emit getelementptr here?
    return g_reg;
}

string llvmGen::genBinop(const string& reg1, const string& op, const string& reg2, type_t op_type){
    string res_reg = getFreshRegister();
    // %vari = op type %reg1, %reg2 
    //TODO: handle dev by 0 - create a label to deal with it?
    //%vari = op type %var1, %var2
    string to_emit = res_reg + " = " + CFanToLlvmOPsMap[pair<string, type_t>(op, op_type)] + " " + reg1 + ", " + reg2; 
    llvmEmit(to_emit);
    return res_reg;
}

int llvmGen::llvmEmit(const string& str, const string& comment) const
{
    PLOGI << "emit the command: "<< str;
    return m_cb->emit(getIdentation() + str + comment);
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

string llvmGen::genGetElementPtr(const string& type, const string& ptr_reg, unsigned int offset, bool is_aggregate)
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
    return var;
}

string llvmGen::genGetVar(const string& varName)
{
    PLOGI << "Generating genGetVar commands";

    auto& curr_table = tables_stack.back();
    int offset = curr_table.getVarOffsetByName(varName);
    bool is_arg = offset < 0;
    if(is_arg){
        offset = -offset;
        return "%" + to_string(--offset);
    }
    llvmEmit("");
    //%vari = getelementptr i32, i32* %frame_ptr, i32 offset
    string src_ptr = genGetElementPtr("i32" , "%frame_ptr", offset);
    auto var_value = getFreshRegister();
    //%varj = load i32, i32* %vari
    genLoad(var_value, "i32", src_ptr);
    
    type_t varType = curr_table.getVarTypeByName(varName);
    if (varType == type_t::INT_T){
        return var_value;
    }
    else{
        // std::TODO: check string type
        string casted_reg = getFreshRegister();
        llvmEmit(casted_reg + " = trunc i32 " + var_value + " to " + CFanToLlvmTypesMap[varType]);
        return casted_reg;
    }
}

void llvmGen::genLoad(const string& dst_reg, const string& type, const string& src_ptr){
    llvmEmit(dst_reg + " = load " + type + ", " + type + "* " + src_ptr);
}

void llvmGen::genStore(const string& type, const string& src_val, const string& dst_ptr){
    llvmEmit("store " + type + " " + src_val + ", " + type + "* " + dst_ptr);
}


// int x = 5b;
//TODO: support diffrenet types
void llvmGen::genStoreValInVar(const string& varName /*i32*/, const string& src_reg /*i8*/)
{
    PLOGI << "Generating store command with: " << varName << ", " << src_reg;

    auto& curr_table = tables_stack.back();
    int offset = curr_table.getVarOffsetByName(varName);
    type_t var_type = curr_table.getVarTypeByName(varName);

    string dst_ptr = genGetElementPtr("i32" , "%frame_ptr", offset);

    genStore("i32", genCasting(src_reg, var_type, type_t::INT_T), dst_ptr);

}

string llvmGen::genCallFunc(const string& funcName, vector<string> args_regs)
{
    auto& curr_table = tables_stack.front(); //global table
    auto types_pair = curr_table.getFuncRetTypeAndArgsTypesByName(funcName);
    type_t ret_type = types_pair.first;
    auto& args_types = types_pair.second;
    for (size_t i=0; i < args_regs.size(); i++){
        args_regs[i].insert(0, CFanToLlvmTypesMap.at(args_types[i]) + " ");
    }

    std::ostringstream sperated_args_list;
    std::copy(args_regs.begin(),
              args_regs.end(),
              std::experimental::make_ostream_joiner(sperated_args_list,", "));
    
    if (ret_type == type_t::VOID_T){
        //%vari = call i32 @test(i32 2)
        llvmEmit("call " + CFanToLlvmTypesMap.at(ret_type) +
                    " @" + funcName + "(" + sperated_args_list.str() + ")");  
        return ""; //Souldn't be used         
    }
    else{
        string ret_val_reg = getFreshRegister();
        //call i32 @test(i32 2)
        llvmEmit(ret_val_reg + " = call " + CFanToLlvmTypesMap.at(ret_type) +
                    " @" + funcName + "(" + sperated_args_list.str() + ")");
        return ret_val_reg;
    }
}

string llvmGen::genCasting(const string& reg, type_t src_type, type_t dst_type)
{
        if (src_type == dst_type){
            return reg;
        }

        auto casted_reg = getFreshRegister();

        if(src_type == type_t::INT_T && dst_type == type_t::BYTE_T){
            llvmEmit(casted_reg + " = trunc i32 " + reg + " to i8");
        }
        else if(dst_type == type_t::INT_T && src_type == type_t::BYTE_T){
            llvmEmit(casted_reg + " = zext i8 " + reg + " to i32");
        }
        else{
            PLOGF << "Casting is available only with int and byte types.";
            return "%failedCasting";  //Souldn't be used 
        }

        return casted_reg;
    
}

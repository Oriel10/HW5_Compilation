#include "llvm_gen.hpp"
#include "semantic_analizer.h"
#include <experimental/iterator>
#include <iterator>
#include <sstream>
#include "plog/include/plog/Log.h"
#include <map>
#include <utility>

extern vector<SymbolTable> tables_stack;
extern Dict types_dict;
 

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
    {{"==", INT_T}, "eq i32"},
    {{"==", BYTE_T}, "eq i8"},
    {{"!=", INT_T}, "ne i32"},
    {{"!=", BYTE_T}, "ne i8"},
    {{">", INT_T}, "sgt i32"},
    {{">", BYTE_T}, "ugt i8"},
    {{">=", INT_T}, "sge i32"},
    {{">=", BYTE_T}, "uge i8"},
    {{"<", INT_T}, "slt i32"},
    {{"<", BYTE_T}, "ult i8"},
    {{"<=", INT_T}, "sle i32"},
    {{"<=", BYTE_T}, "ule i8"},

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
    PLOGI << "Emitting command: "<< str;
    string comment_prefix = comment.empty() ? "" : ";";
    string tab = str.empty() ? "" : "      ";
    return m_cb->emit(getIdentation() + str + tab + comment_prefix + comment);
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
    PLOGI << to_emit;
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
    if(!tables_stack.back().is_return_appeared){
        SymbolTableEntry& curr_func = tables_stack[0].m_entries.back();
        if (curr_func.m_ret_type == type_t::VOID_T){
            llvmEmit("ret void");
        }
        else{
            llvmEmit("ret " + CFanToLlvmTypesMap[curr_func.m_ret_type] + " 0");
        }
    }
    zeroIdentation();
    llvmEmit("}\n");
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

    auto var_entry = findIdentifier(varName);
    int offset = var_entry->m_offset;
    bool is_arg = offset < 0;
    if(is_arg){
        offset = -offset;
        return "%" + to_string(--offset);
    }
    llvmEmit("", "Getting var " + varName);
    //%vari = getelementptr i32, i32* %frame_ptr, i32 offset
    string src_ptr = genGetElementPtr("i32" , "%frame_ptr", offset);
    auto var_value = getFreshRegister();
    //%varj = load i32, i32* %vari
    genLoad(var_value, "i32", src_ptr);
    
    type_t varType = var_entry->m_type;
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


void llvmGen::genStoreValInVar(const string& varName , const string& src_reg, bool initial)
{
    PLOGI << "Generating store command with: " << varName << ", " << src_reg;

    auto var_entry = findIdentifier(varName);
    int offset = var_entry->m_offset;
    type_t var_type = var_entry->m_type;

    if(initial){
        llvmEmit("", "Initializing var " + varName + " to 0");
    }
    else{
        llvmEmit("", "Storing value in var " + varName);
    }
    string dst_ptr = genGetElementPtr("i32" , "%frame_ptr", offset);
    if (initial){
        genStore("i32", "0", dst_ptr);
    }
    else{
        genStore("i32", genCasting(src_reg, var_type, type_t::INT_T), dst_ptr);
    }

}

string llvmGen::genCallFunc(const string& funcName, vector<string> args_regs)
{
    PLOGI << "Generating call function command of: " << funcName;

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
            PLOGF << "Casting is available only with int and byte types."<<"(src_t: " << types_dict[src_type]<<", dst_t: " << types_dict[dst_type]<<")";
            return "%failedCasting";  //Souldn't be used 
        }

        return casted_reg;
}


string llvmGen::genCompare(const string& reg1, const string& rel_op, const string& reg2, type_t op_type)
{
    string res_reg = getFreshRegister();
    //%vari = icmp relop %varm, %varn
    llvmEmit(res_reg + " = icmp " + CFanToLlvmOPsMap[{rel_op, op_type}] + " " + reg1 + ", " + reg2);
    return res_reg;// i1 register
}

void llvmGen::genCondBranch(const string& bool_reg, pair<int,BranchLabelIndex>& true_list_item,
                                           pair<int,BranchLabelIndex>& false_list_item)
{
    //br i1 %bool_reg, label @, label @
    int loc = llvmEmit("br i1 " + bool_reg + ", label @, label @");
    true_list_item = {loc, BranchLabelIndex::FIRST};
    false_list_item = {loc, BranchLabelIndex::SECOND};
    return; 
}

void llvmGen::genUncondBranch(pair<int,BranchLabelIndex>& list_item)
{
    //br label @
     int loc = llvmEmit("br label @");
    list_item = {loc, BranchLabelIndex::FIRST};
    return;   
}

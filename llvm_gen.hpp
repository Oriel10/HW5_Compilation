#ifndef _LLVM_GEN_
#define _LLVM_GEN_

#include <string>
#include "production_rules.h"
#include "bp.hpp"

class llvmGen
{ 
    size_t m_reg_num;
    size_t m_indentation;
    CodeBuffer* m_cb;

public: 

    llvmGen();
    static llvmGen& instance();

    const size_t maxNumOfVars = 50;

    int llvmEmit(const string& str) const;
    void closeFunc();
    
    // Handle identation
    void incIdentation();
    void decIdentation();
    void zeroIdentation();
    string getIdentation() const;
    
    string getFreshRegister(bool is_global = false);
    string setReg(string init_val, type_t val_type);

    string genStringReg(string str);
    string genBinop(string reg1, string op, string reg2, type_t op_type);
    void genInitialFuncs() const;
    void genFuncDecl(type_t RetType, const string& funcName, vector<type_t> argsTypes) const;
    
    //usage: %res_reg = getelementptr type, type* ptr_reg, i32 0, i32 idx
    string genGetElementPtr(string type, string ptr_reg, unsigned int idx, bool is_aggregate = false);
    string genAllocVar();
    string genGetVar(const string& varName);
    void genLoad(string dst_reg, string type, string src_ptr);
    void genStore(string type, string src_reg, string dst_ptr);
    void genStoreValInVar(string varName, string reg);
};


#endif
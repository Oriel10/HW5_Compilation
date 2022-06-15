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

    string getIdentation() const;
    int llvmEmit(const string& str) const;
    string getFreshRegister();
    void genInitialFuncs() const;
    void genFuncDecl(type_t RetType, const string& funcName,
                 vector<type_t> argsTypes) const;
    void genAllocVar();
    void genStoreValInVar(string varName, size_t value);
    void incIdentation();
    void decIdentation();
    void zeroIdentation();
    void closeFunc();
};


#endif
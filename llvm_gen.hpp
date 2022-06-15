#ifndef _LLVM_GEN_
#define _LLVM_GEN_

#include <string>
#include "compiler_impl.h"
#include "bp.hpp"

class llvmGen
{ 
    size_t m_reg_num;
    size_t m_indentation;
    CodeBuffer* m_cb;

public: 

    llvmGen();
    // static llvmGen& instance();27

    std::string getIdentation();
    void updateIdentation(bool increase = true);

    std::string getFreshRegister();
    void genInitialFuncs() const;
    void genFuncDecl(type_t RetType, const std::string& funcName,
                 std::vector<type_t> argsTypes) const; //TODO: add somehow pointers to args.
    void genAllocVar(std::string varName);
    void genStoreValInVar(std::string varName, size_t value);
};


#endif
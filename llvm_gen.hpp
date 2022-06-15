#ifndef _LLVM_GEN_
#define _LLVM_GEN_

#include <string>
#include "compiler_impl.h"
#include "bp.hpp"

class llvmGen
{ 
    size_t m_reg_num;
    CodeBuffer* m_cb;

public: 

    llvmGen();
    // static llvmGen& instance();
    std::string genRegister();
    void genInitialFuncs() const;
    void genFuncDecl(type_t RetType, const std::string& funcName,
                 std::vector<type_t> argsTypes) const; //TODO: add somehow pointers to args.
};


#endif
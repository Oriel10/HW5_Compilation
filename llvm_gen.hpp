#ifndef _LLVM_GEN_
#define _LLVM_GEN_

#include <string>
#include "compiler_impl.h"
#include "bp.hpp"

class llvmGen
{ 
    size_t m_reg_num;
    // CodeBuffer& m_cb_inst;

public: 

    llvmGen() = default;
    // static llvmGen& instance();
    std::string genRegister();
    void genInitialFuncs();
    std::string genFuncDecl(type_t RetType, std::string funcName, std::vector<type_t> argsTypes);
};


#endif
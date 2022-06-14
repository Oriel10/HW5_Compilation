#include "llvm_gen.hpp"
#include <experimental/iterator>
#include <iterator>

// llvmGen::llvmGen(): m_reg_num(0)
// {
//     m_cb_inst = &codeBuffer::instance();
// }

// llvmGen &llvmGen::instance() {
// 	static llvmGen inst;//only instance
// 	return inst;
// }

std::string llvmGen::genRegister(){
    std::string res_reg = "%var";
    res_reg += std::to_string(m_reg_num++);
    return res_reg;
}

std::string llvmGen::genFuncDecl(type_t RetType, std::string funcName, std::vector<type_t> argsTypes)
{
    std::string command = "declare";

    //TODO: add iXX by type  (int / byte / bool / void)
    command += "i32 @";

    command += funcName + "(";

    // std::ostringstream s;
    // std::copy(argsTypes.begin(),
    //           argsTypes.end(),
    //           std::experimental::make_ostream_joiner(std::cout, ", "));

    for (auto& type : argsTypes){
        //TODO: add iXX by type
        command += "i32, ";
    }

}

void llvmGen::genInitialFuncs(){
    auto& inst = CodeBuffer::instance();

    inst.emitGlobal("declare i32 @printf(i8*, ...)\ndeclare void @exit(i32)");
    inst.emitGlobal("declare void @exit(i32)");
    inst.emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    inst.emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    
    
}


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

    int llvmEmit(const string& str, const string& comment = "") const;
    void closeFunc(Statements* statements);
    
    // Handle identation
    void incIdentation();
    void decIdentation();
    void zeroIdentation();
    string getIdentation() const;
    
    string getFreshRegister(bool is_global = false);
    string setReg(const string& init_val, type_t val_type);

    string genStringReg(const string& str);
    string genBinop(const string& reg1, const string& op, const string& reg2, type_t op_type);
    void genInitialFuncs() const;
    void genFuncDecl(type_t RetType, const string& funcName, vector<type_t> argsTypes) const;
    
    //usage: %res_reg = getelementptr type, type* ptr_reg, i32 0, i32 idx
    string genGetElementPtr(const string& type, const string& ptr_reg, unsigned int idx, bool is_aggregate = false);
    string genAllocVar();
    string genGetVar(const string& varName);
    void genLoad(const string& dst_reg, const string& type, const string& src_ptr);
    void genStore(const string& type, const string& src_reg, const string& dst_ptr);
    void genStoreValInVar(const string& varName, const string& reg,  bool initial=false);
    string genCallFunc(const string& funcName, vector<string> args = vector<string>{}); 
    string genCasting(const string& reg, type_t src_type, type_t dst_type);
    string genCompare(const string& reg1, const string& rel_op, const string& reg2, type_t op_type);
    void genCondBranch(const string& bool_reg, pair<int,BranchLabelIndex>& true_list_item,
                                           pair<int,BranchLabelIndex>& false_list_item);
    void genUncondBranch(pair<int,BranchLabelIndex>& list_item,  string comment = "");
    
    /** genBoolExpVal
     * Based on trueList and falseList of boolean expression, the function
     * generates false label and true label and then, using phi llvm instruction,
     * generates new reg with the boolean vales and return it;
     *  
    */
    string genBoolExpVal(const vector<pair<int,BranchLabelIndex>>& true_list, const vector<pair<int,BranchLabelIndex>>& false_list);

};


#endif
#ifndef _PRODUCTION_RULES_
#define _PRODUCTION_RULES_

#include "semantic_analizer.h"
#include "bp.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <string.h>


#define ASSERT_ARG(arg) assert(arg!=nullptr)
#define ASSERT_2ARGS(arg1, arg2) assert(arg1!=nullptr); assert(arg2!=nullptr)
#define ASSERT_3ARGS(arg1, arg2, arg3) ASSERT_2ARGS(arg1, arg2); assert(arg3!=nullptr)
#define ASSERT_4ARGS(arg1, arg2, arg3, arg4) ASSERT_3ARGS(arg1, arg2, arg3); assert(arg4!=nullptr)
#define ASSERT_5ARGS(arg1, arg2, arg3, arg4, arg5) ASSERT_4ARGS(arg1, arg2, arg3, arg4); assert(arg5!=nullptr)

#define MAX_BYTE_SIZE (255)

using namespace std;

void startCompiler();
void endCompiler();

/*================ production rules ===================*/

struct Program;
struct Funcs;
struct FuncDecl;
struct RetType;
struct Formals;
struct FormalsList;
struct FormalDecl;
struct Statements;
struct Statement;
struct NextInstMarker;
struct IfWhileMarker;
struct ElseMarker;
struct Call;
struct ExpList;
struct Type;
struct Exp;

struct Node{
    string lexeme;
    string token_type;
    int lineno;
    Node() = default;
    Node(string lexeme, string token_type, int lineno) : lexeme(lexeme), token_type(token_type), lineno(lineno){}
    virtual ~Node() {};
};

struct Program : public Node{
    Program(Funcs*); //Program -> Funcs
    ~Program() = default;
};

struct Funcs : public Node{
    Funcs() = default; // Funcs -> e
    Funcs(FuncDecl*, Funcs*); // Funcs -> FuncDecl Funcs
    ~Funcs() = default;
};

struct FuncDecl : public Node{
    FuncDecl(RetType*, Node* ID, Formals*); // FuncDecl -> RetType ID LP Formals RP Statements RB
    ~FuncDecl() = default;
};

struct RetType : public Node{
    RetType(Type*); // RetType -> Type
    RetType(Node*); // RetType -> VOID
    ~RetType() = default;
    type_t m_type;
};

struct Formals : public Node{

    Formals() = default; // Formals -> e
    Formals(FormalsList*); // Formals -> FormalsList
    ~Formals() = default;
    vector<FormalDecl*> m_formal_decls;
};

struct FormalsList : public Node{
    FormalsList(FormalDecl*); // FormalsList -> FormalDecl 
    FormalsList(FormalDecl*, FormalsList*); // FormalsList -> FormalDecl COMMA FormalsList
    ~FormalsList() = default;
    vector<FormalDecl*> m_formal_decls;
     
};

struct FormalDecl : public Node{
    FormalDecl(Type*, Node* ID); // FormalDecl -> Type ID
    ~FormalDecl() = default;
    string m_name;
    type_t m_type;
    int lineno;
};

struct NextInstMarker : public Node{
    string m_label = "";
    NextInstMarker(string);
};

struct Statements : public Node{
    vector<Statement*> m_statement_list;
    vector<pair<int,BranchLabelIndex>> m_next_list;
    Statements() = default;
    Statements(Statement*); // Statements -> Statement
    Statements(Statements*, NextInstMarker*, Statement*); // Statements -> Statements M Statement
    ~Statements() = default;
};

struct Statement : public Node{
    string m_label = ""; //start label for the statement 
    vector<pair<int,BranchLabelIndex>> m_next_list;
    bool m_is_return = false;
    Statement() = default;
    Statement(Statements*); // Statement -> LB Statements RB
    Statement(Type*, Node*); // Statement - >Type ID SC 
    Statement(Type*, Node*, Exp*); // Statement -> Type ID ASSIGN Exp SC 
    Statement(Node*, Node*, Exp*); // Statement -> AUTO ID ASSIGN Exp SC
    Statement(Node*, Exp*); // Statement -> ID ASSIGN Exp SC
    Statement(Call*); // Statement -> Call SC
    Statement(Node*); // //Statement -> RETURN SC | BREAK SC | CONTINUE SC
    Statement(Exp*); // Statement -> Return Exp SC
    Statement(Node*, Exp*, IfWhileMarker*, Statement*); // Statement -> IF_WHILE LP Exp RP IfWhileMarker Statement
    Statement(Exp*, IfWhileMarker*, Statement*, ElseMarker*, Statement*); // Statement -> IF LP Exp RP IfWhileMarker Statement Else ElseMarker Statement
    // Statement(Node* WHILE, Exp*, Statement*);
    // Statement(Node* BREAK);
    // Statement(Node* CONT);

    ~Statement() = default;
};

struct IfWhileMarker : public Node{
    string m_label = "";
    IfWhileMarker(string);
};

struct ElseMarker  : public Node{
    string m_label = "";
    ElseMarker(string);
};



struct Call : public Node{
    type_t m_type;
    string m_ret_reg;
    Call(Node* ID, ExpList*); // Call -> ID LP ExpList RP
    Call(Node* ID); // Call -> ID LP RP
    ~Call() = default;
};

struct ExpList : public Node{
    vector<type_t> m_exp_list_types;
    vector<string> m_exp_list_regs;
    ExpList(Exp*); // 𝐸𝑥𝑝𝐿𝑖𝑠𝑡 → 𝐸𝑥p
    ExpList(Exp*, ExpList*); // 𝐸𝑥𝑝𝐿𝑖𝑠𝑡 → 𝐸𝑥𝑝 𝐶𝑂𝑀𝑀𝐴 𝐸𝑥𝑝𝐿𝑖𝑠 
    ~ExpList() = default;
};

struct Type : public Node{
    Type(Node* type); // Type -> INT/BYTE/BOOL
    ~Type() = default;
    type_t m_type;
};

struct Exp : public Node{
    type_t m_type;
    string m_reg;
    vector<pair<int,BranchLabelIndex>> m_true_list;
    vector<pair<int,BranchLabelIndex>> m_false_list;
    Exp(Node*, Exp*, Node*); // Exp -> LP Exp RP
    Exp(Exp*, Node* , Exp*); // Exp -> Exp * Exp, * in {BINOP_PLUSMINUS, BINOP_MULDIV, AND, OR, RELOP_EQ, RELOP_SIZE}
    Exp(Node*); // Exp -> *, * in {ID, NUM, STRING, TRUE, FALSE}
    Exp(Node*, Node*); // Exp -> NUM B
    Exp(Call*); // Exp -> Call
    // Exp(Node* ID); // Exp -> ID
    // Exp(Node* STRING);
    // Exp(Node* TRUE);
    // Exp(Node* FALSE);
    Exp(Node* NOT, Exp*); // Exp -> NOT Exp
    // Exp(Exp*, Node* AND, Exp*);
    // Exp(Exp*, Node* OR, Exp*);
    // Exp(Exp*, Node* RELOP, Exp*);
    Exp(Type*, Exp*); // Exp -> LP Type RP Exp
    Exp(Exp*); // Bool_Exp -> Exp

    ~Exp() = default;
};



#endif //PRODUCTION_RULESS
#ifndef SEMANTIC_ANALIZER
#define SEMANTIC_ANALIZER

#include <vector>
#include <string>
#include <map>

using namespace std;

enum type_t {INT_T, BOOL_T, BYTE_T, STRING_T, VOID_T};
enum symbol_type_t {VAR, FUNC, ARG};

struct Dict{
    map<type_t, string> types_dict;
    map<string, type_t> type_string_to_enum;
    Dict(){
        types_dict = {
            {INT_T, "INT"},
            {BYTE_T, "BYTE"},
            {BOOL_T, "BOOL"},
            {VOID_T, "VOID"},
            {STRING_T, "STRING"}
        };
        type_string_to_enum = {
            {"INT", INT_T},
            {"BYTE", BYTE_T},
            {"BOOL", BOOL_T},
            {"VOID", VOID_T},
            {"STRING" , STRING_T}
        };
    }
    string operator[](type_t type_e){
        return types_dict[type_e];
    }
    type_t operator[](string type_s){
        return type_string_to_enum[type_s];
    }
};

struct SymbolTableEntry{
    string m_name;
    type_t m_type;
    int m_offset;
    type_t m_ret_type;
    symbol_type_t m_symbol_type;
    vector<type_t> m_args_types;
    SymbolTableEntry(const string& name, type_t type, int offset, symbol_type_t symbol_type) 
        : m_name(name), m_type(type), m_offset(offset), m_symbol_type(symbol_type){}
    SymbolTableEntry(const string& name, type_t ret_type, vector<type_t>& args_types, symbol_type_t symbol_type) 
        : m_name(name), m_ret_type(ret_type), m_offset(0), m_args_types(args_types), m_symbol_type(symbol_type) {}
    void print() const;
};

struct SymbolTable{
    vector<SymbolTableEntry> m_entries;
    SymbolTable(){}
    SymbolTable(vector<SymbolTableEntry> entries) : m_entries(entries) {}
    bool is_return_appeared = false;
    void print() const;
    void addFuncEntry (const string& name, type_t ret_type, vector<type_t>& arg_types);
    void addVarEntry (const string& name, type_t type);
    void addArgEntry (const string& name, type_t type, int offset);
    int getVarOffsetByName(const string& varName);
    type_t getVarTypeByName(const string& varName);
    pair<type_t, vector<type_t>> getFuncRetTypeAndArgsTypesByName(const string& funcName); 
}; 

void openScope();

void closeScope();

const SymbolTableEntry* findIdentifier(const string& name);
bool explicitCastValidity(type_t dst_type , type_t src_type);
bool automaticCastValidity(type_t dst_type , type_t src_type);


#endif //SEMANTIC_ANALIZER
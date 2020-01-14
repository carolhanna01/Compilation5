#ifndef SCOPES_H_INCLUDED
#define SCOPES_H_INCLUDED 

//
// Created by Carol Hanna on 15/12/2019.
//

#include <iostream>
#include <stack>
#include <vector>
#include "hw3_output.hpp"
#include "bp.hpp"

using namespace std;
using namespace output;

#define INT_SIZE 1
#define BOOL_SIZE 1
#define YYSTYPE scopeTables::variableEntry*

extern int yylineno;
typedef pair<int,BranchLabelIndex> bp_pair;

namespace scopeTables{
    template <typename t>
        class entry {
            public:
            string name;
            t type;
            int offset;
            int value;
            string reg;
            string label;
            string op;
            vector<bp_pair> trueList;
            vector<bp_pair> falseList;
            entry(string name="", t type= t(), int offset=0, int value=0, string op= ""):name(name), type(type), offset(offset), value(value), op(op),
                        reg(""), label(""){
                this->trueList = vector<bp_pair>();
                this->falseList = vector<bp_pair>();

            };
        };

        class functionType {
        public:
            string returnType;
            vector<string> paramTypes;

            functionType(string returnType= "", vector<string> paramTypes= vector<string>()):
            returnType(returnType), paramTypes(paramTypes){};

            void addParam(string paramType) {
                paramTypes.push_back(paramType);
            }
        };

        class enumType {
        public:
            vector<string> values;

            enumType(vector<string> values = vector<string>()) : values(values){};
        };


    typedef entry<string> variableEntry;
    typedef entry<functionType> functionEntry;
    typedef entry<enumType> enumEntry;
    typedef entry<string> operationEntry;

    class Scope{
            public:

            vector<variableEntry> variables;
            vector<functionEntry > functions;
            vector<enumEntry> enums;
            bool insideLoop;
            functionEntry* currentFunc;
            Scope* parent;


            // Constructor
            Scope(){
            variables = vector<variableEntry>();
            functions = vector<functionEntry>();
            insideLoop = false;
            currentFunc = new functionEntry();
            }

            variableEntry* getVariable(const string &name) {
                // cout << "SEARCHING FOR " << name << endl;
                for (variableEntry& v : variables) {
                    // cout << "COMPARING " << name << " WITH " << v.name << endl; 
                    if (!name.compare(v.name)) {
                        return &v;
                    }
                }
                return nullptr;
            }

            functionEntry* getFunction(const string &name) {
                for (functionEntry& f : functions) {
                    if (name == f.name) {
                        return &f;
                    }
                }
                return nullptr;
            }

            enumEntry* getEnum(const string &name) {
                for (enumEntry& e : enums) {
                    if (name == e.name) {
                        return &e;
                    }
                }
                return nullptr;
            }

            enumEntry* getEnumWithEnumeration(const string& enumeration) {
                for (enumEntry& e : enums) {
                    for (string enum_value : e.type.values) { 
                        if (enum_value == enumeration) {
                            return &e;
                        }
                    }
                }
                return nullptr;
            }

            void insertVariable(variableEntry v) {
                if (getVariable(v.name) != nullptr ) {
                    errorDef(yylineno, v.name);
                    exit(0);
                }
                variables.push_back(variableEntry(v.name, v.type, v.offset));
            }
            void insertFunction(functionEntry f) {
                if (getVariable(f.name) != nullptr) {
                    errorDef(yylineno, f.name);
                    exit(0);
                }
                functions.push_back(f);
            }
            void insertEnum(enumEntry e) {
                if (getEnum(e.name) != nullptr) {
                    errorDef(yylineno, e.name);
                    exit(0);
                }
                enums.push_back(e);
            }



            void printVariables(){
                for(variableEntry v: variables)
                    printID(v.name, v.offset, v.type);
            }
            void printFunctions(){
                for(functionEntry f : functions)
                    printID(f.name, 0, makeFunctionType(f.type.returnType, f.type.paramTypes));
            }
            void printEnums(){
                for(enumEntry e : enums)
                    printEnumType(e.name.substr(5), e.type.values);
            }


        };
}

#endif

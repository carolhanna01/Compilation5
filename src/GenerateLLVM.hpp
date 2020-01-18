#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include "hw3_output.hpp"
#include "scopes.h"
#include "bp.hpp"

using namespace std;
using namespace output;
typedef pair<int,BranchLabelIndex> bp_pair;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Wrapping bp header functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int emit(const string &s){
    return CodeBuffer::instance().emit(s);
}

void emitGlobal(const std::string& s){
    CodeBuffer::instance().emitGlobal(s);
}

void bpatch(const vector<bp_pair>& l, const string &label) {
    CodeBuffer::instance().bpatch(l, label);
}


string genLabel() {
    return CodeBuffer::instance().genLabel();
}

vector<bp_pair> makeList(bp_pair item) {
    return CodeBuffer::instance().makelist(item);
}

vector<bp_pair> merge(const vector<bp_pair> &l1,const vector<bp_pair> &l2) {
    return CodeBuffer::instance().merge(l1, l2);
}

void printCodeBuffer() {
    return CodeBuffer::instance().printCodeBuffer();
};

void printGlobalBuffer(){
    return CodeBuffer::instance().printGlobalBuffer();
}
/*                                         ~~~~~~~~~                                        */

string toString(int num) {
    return to_string(num);
}

string freshReg(){
    static int regCount = -1;
    regCount++;
    return "%r" + toString(regCount);
}

string freshString(){
    static int strCount = -1;
    strCount++;
    return "@.Str" + toString(strCount);
}



string getArithmeticOp(string op, bool is_signed){

    if (op == "+") return "add ";
    if (op == "-") return "sub ";
    if (op == "*") return "mul ";
    if (op == "/") return is_signed ? "sdiv " : "udiv ";
    return "";
}

string getRelopOp(string op) {
    if (op == ">")  return "icmp sgt i32 ";
    if (op == ">=") return "icmp sge i32 ";
    if (op == "<")  return "icmp slt i32 ";
    if (op == "<=") return "icmp sle i32 ";
    if (op == "==") return "icmp eq i32 ";
    if (op == "!=") return "icmp ne i32 ";
    return "";
}

void exitProgram(){
 emit("call void @exit(i32 1)");
}


int emitCondition(string r1, string op, string r2) {
    string resReg = freshReg();
    emit(resReg + " = " + getRelopOp(r1) + r1 + ", " + r2);
    return emit("br i1 " +  resReg + ", label @, label @");
}

int emitUnconditional() {
    return emit("br label @");
}

class GenerateLLVM{
private:
    stack<string> stackBases;

    string prepareArgsForCall(vector<string> paramRegs, string name) {
        if (paramRegs.size() == 0) {
            return string("");
        }

        string orderedRegs = "";
        if (name == "print"){
            for (auto reg : paramRegs) {
                orderedRegs += "i8* " + reg + ", ";
            }
        }else {
            for (auto reg : paramRegs) {
                orderedRegs += "i32 " + reg + ", ";
            }
        }
        orderedRegs.pop_back();
        orderedRegs.pop_back();
        return orderedRegs;
    }

    string prepareArgsForDec(int numArgs) {
        if (numArgs == 0) {
            return string("");
        }

        string args = "";
        for (int i = 0; i < numArgs; i++) {
            args += "i32, ";
        }
        args.pop_back();
        args.pop_back();
        return args;
    }

    void addExitPrintFunctions(){

        emit("declare i32 @printf(i8*, ...)");
        emit("declare void @exit(i32)");
        emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
        emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");

        emit("define void @printi(i32) {");
        emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4x i8]* @.int_specifier, i32 0, i32 0), i32 %0)");
        emit("ret void");
        emit("}");
        emit("define void @print(i8*) {");
        emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4x i8]* @.str_specifier, i32 0, i32 0), i8* %0)");
        emit("ret void");
        emit("}");
    }

public:
    GenerateLLVM() {
      //  emitGlobal("division_by_zero_error: call void (i32) @print(i32 'Error division by zero')"); //todo: is this how the print function works?
        addExitPrintFunctions();
    }

    string binop(string r1, string r2, string op, bool isSigned) {
        if (op == "/") {
            int errorLabel = emitCondition("0", "!=", r2);
            exitProgram();
            bpatch(makeList(bp_pair(errorLabel, FIRST)), genLabel());
        }
        string resultReg = freshReg();
        emit(resultReg + " = " + getArithmeticOp(op, isSigned) + " " + r1 + ", " + r2);

        if (!isSigned) //todo - zext
            emit("and i32 " + r1 + ", 255");

        return r1;
    }


    /*
        @args   - int numOfElements - The number of i32 elements to allocate on the stack
        @ret    - string
    */
    string prepStack() {
        string base = freshReg();
        emit(base + " = alloca [50 x i32]");
        stackBases.push(base);
        return base;
    }

    /*
        Returns a register containing the address of an element with the requested offset in the stack.
    */
    string stackGet(int offset) {
        string ptr = freshReg();
        string element = freshReg();
        emit(ptr + " = getelementptr [50 x i32], [50 x i32]* " + stackBases.top() + ", i32 0, i32 " + toString(offset));
        emit(element + " = load i32, i32* " + ptr);
        return element;
    }

    /*
        Set a value in stack by a given register
        Returns register with the address of the element
    */
    void stackSet(int offset, string reg) {
        string ptr = freshReg();
        emit(ptr + " = getelementptr [50 x i32], [50 x i32]* " + stackBases.top() + ", i32 0, i32 " + toString(offset));
        emit("store i32 " + reg + ", i32* " + ptr);    
    }

    /*
        Set a value in stack by a given value
    */
    void stackSetByVal(int offset, int value) {
        stackSet(offset, toString(value));     //TRICK TO SET AN ACTUAL VALUE, SENT AS A REG
    }

    void newVariable(int offset, string reg = "") {
        if(reg == "")
            stackSetByVal(offset, 0);
        else
            stackSet(offset, reg);
    };

    /*
        Helpful guide for how backpatching for booleans should look like:

            {boolean assesment code BRANCHED to missing labels}
            trueLabel:
                {true code} // Assign true to register/memory
                br after
            falseLabel:
                {false code} // Assign false to register/memory
                // 'br after' is not needed because it will actually go to after... 
            after:
                {continuation...}

        We must makes sure the trueLabel and falseLabel are backpached for the boolean assesment code.
    */
    string setBool(vector<bp_pair> &trueList, vector<bp_pair> &falseList, bool intoStack = false, int offset = 0) { //todo: previously defined..?
        string trueLabel = genLabel();
        bpatch(trueList, trueLabel);

        string reg = freshReg();
        if (intoStack) {
            stackSetByVal(offset, 1);
        } else {
            assignToReg(reg, 1);
        }

        int after_address = emitUnconditional();
        string falseLabel = genLabel();
        bpatch(falseList, falseLabel);

        if (intoStack) {
            stackSetByVal(offset, 0);
        }
        else {
            assignToReg(reg, 1);
        }
        string afterLabel = genLabel();
        bpatch(makeList(bp_pair(after_address, FIRST)),afterLabel);
        return reg;
    };
    
    string number(int value) {
        string reg = freshReg();
        assignToReg(reg, value);
        return reg;
    }

    string addGlobalString(string s){
        string newS= freshString();
        string stringSize= toString(s.length()+2);
        emitGlobal(newS + " = constant [" + stringSize + "x i8] c\"" + s + "\\0A\\00\"");
        string reg = freshReg();
        emit(reg + " = getelementptr [" + stringSize + "x i8] , ["+ stringSize + "x i8] * " + newS + ", i32 0, i32 0");
        return reg;
    }

    void assignToReg(string reg, int value) {
        emit(reg + " = add i32 0, " + toString(value));
    }

    void startFuncDef(string name, int numArgs, string type= "") {
        string args = prepareArgsForDec(numArgs);
        if(type == "VOID"){
            emit("define void @" + name + "(" + args + ") {" );
        }
        else {
            emit("define i32 @" + name + "(" + args + ") {" );
        }
        prepStack();
    }

    void endFuncDef(string returnReg = "") {
        if (returnReg == "") {
            emit("ret void");
        }
        else {
            emit("ret i32 " + returnReg);
        }
        emit("}");
    }

    string callFunc(string name, string returnType, vector<string> paramRegs = vector<string>()) {
        string args = prepareArgsForCall(paramRegs, name);
        string output = string("");

        if (returnType == "VOID") {
            emit("call void @" + name + "(" + args + ")");
        }else {
            output = freshReg();
            emit(output + " = call i32"  + " @" + name + "(" + args + ")");
        }
        return output;
    }



};
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

vector<bp_pair> makelist(bp_pair item) {
    return CodeBuffer::instance().makelist(item);
}

vector<bp_pair> merge(const vector<bp_pair> &l1,const vector<bp_pair> &l2) {
    return CodeBuffer::instance().merge(l1, l2);
}
/*                                         ~~~~~~~~~                                        */

string toString(int num) {
        stringstream ss;
        ss << num;
        return ss.str();
}

string freshReg(){
    static int regCount=-1;
    regCount++;
    return "%r" + toString(regCount);
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

int emitCondition(string r1, string op, string r2) {
    string resReg = freshReg();
    emit(resReg + " = " + getRelopOp(r1) + r1 + ", " + r2);
    return emit("br i1 " +  resReg + ", label @, label @");
}

int emitUnconditional() {
    return emit("br label @");
}

class GenerateLLVM{
public:
    GenerateLLVM(){
        emitGlobal("division_by_zero_error: call void (i32) @print(i32 'Error division by zero')"); //todo: is this how the print function works?
    }

    string binop(string r1, string r2, string op, bool isSigned){
        if(op == "/"){
            int errorLabel = emitCondition("0", "!=",  r2 );
            /* todo: work on this when you implement the stack and understand it
            emit("la " + r1 + ", division_by_zero");
            push(r1);
            emit("jal print");
            */

            exitProgram();
            bpatch(makelist(make_pair(errorLabel, FIRST)),genLabel());
        }
        string resultReg= freshReg();
        emit(resultReg + " = " + getArithmeticOp(op, isSigned) + " " + r1 + ", " + r2);

        if(!isSigned)
            emit("and i32 " + r1 + ", 255");

        return r1;
    }
};
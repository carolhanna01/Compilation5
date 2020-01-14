#include <iostream>
#include <vector>
#include <stack>
#include "hw3_output.hpp"
#include "scopes.h"
#include "bp.hpp"

using namespace std;
using namespace output;

int freshReg(){
    static int regCount=-1;
    regCount++;
    return "r" + regCount;
}


string arithmeticOp(string op, bool is_signed){

    if (op == "+") return "add ";
    if (op == "-") return "sub ";
    if (op == "*") return "mul ";
    if (op == "/") return is_signed ? "sdiv " : "udiv ";
    return "";
}

string getRelopOp(string op) {
    if (op == ">")  return "icmp sgt ";
    if (op == ">=") return "icmp sge ";
    if (op == "<")  return "icmp slt ";
    if (op == "<=") return "icmp sle ";
    if (op == "==") return "icmp eq ";
    if (op == "!=") return "icmp ne ";
    return "";
}

void exitProgram(){
 emit("call void @exit(i32 1)")
}

void addExitPrintFunctions(){

    emit("declare i32 @printf(i8*, ...)");
    emit("declare void @exit(i32)");
    emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");

    emit("define void @printi(i32) {")
    emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4x i8]* @.int_specifier, i32 0, i32 0), i32 %0)")
    emit("ret void")
    emit("}")
    emit("define void @print(i8*) {")
    emit("call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4x i8]* @.str_specifier, i32 0, i32 0), i8* %0)")
    emit("ret void")
    emit("}")

}

class GenerateLLVM{
public:
    int binop(int r1, int r2, string op, string type, bool is_signed){
        if(op == "/"){
            //add treatment for division by zero
        }
        int resultReg= freshReg();
        emit(resultReg + " = " arithmeticOp(op, is_signed) + " " + r1 + ", " + r2);

        /* todo: dealing with unsigned
        if(!is_signed)
            emit("and " + r1 + ", 255");
        */
        return r1;
    }
};
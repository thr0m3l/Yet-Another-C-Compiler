#ifndef PARSERUTILS
#define PARSERUTILS

#include "./SymbolTable/SymbolTable.h"
#include "./SymbolTable/SymbolInfo.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;
ofstream log, error;

extern int line_count;
int error_count = 0;

string currentFunction = "";

SymbolTable st(&log);

struct param { 
    public:
    string type;
    string name;
    param(string name, string type){
        this->name = name;
        this->type = type;
    }
    
};

vector<param> paramList;
vector<string> argTypeList;

string type = "";



void printRule(string rule)
{
    log << "Line no: " << line_count <<" " << rule << "\n"  << endl;
}

void printSymbol(SymbolInfo *sym)
{
    log << sym->getName()<<endl<<endl;
}

void printLog(string msg)
{
    log << msg << endl;
}

void printError(string msg)
{
    error << "Error at line no: " << line_count << " - "<<msg << endl;
    log << "Error at line no: " << line_count << " - "<<msg << endl;
    error_count++;

}

void yyerror(const char *s)
{
    printError(s);

}

void printWarning(string msg)
{
    error << "Warning at line no: " << line_count <<" " << msg << endl;
}

bool insertSymbol(SymbolInfo *sym)
{
    return st.insertSymbol(sym);
}

void enterScope(){
    st.enterScope();
    // cout<<currentFunction<<endl;
    if (currentFunction != ""){
        for(param p : paramList){
            SymbolInfo *sym = new SymbolInfo(p.name, "ID");
            sym->setIdType("VARIABLE");
            for (auto & c: p.type) c = toupper(c);
            sym->setVarType(p.type);
            insertSymbol(sym);
        }
        paramList.clear();
    }
}

void exitScope()
{
    currentFunction = "";
    st.exitScope();
    st.printAll();
}

SymbolInfo* nullSym()
{
    SymbolInfo *ns = new SymbolInfo("NULL", "NULL");
    ns->setVarType("INT");
    ns->setIdType("VARIABLE");
    ns->intData.push_back(0);
    ns->floatData.push_back(0);

    return ns;
}

SymbolInfo* getVariable(SymbolInfo* sym)
{
    
    SymbolInfo *var = st.lookup(sym->getName());

    if (var == NULL){
        printError(sym->getName() + " is not declared");
    } else if (!var->isVariable())
    {
        if(var->isArray()){
            printError(var->getName() + " is an array and must be accessed with an index");
        } else {
            printError(var->getName() + " is not a variable");
        }
    } 
    else {
        return var;
    }
    return nullSym();
}


void addFuncDef (SymbolInfo *funcVal, SymbolInfo *returnType)
{

    SymbolInfo *func = st.lookup(funcVal->getName());

    if (func != NULL){
        if (func->getIdType() != "FUNCTION" && func->isFuncDefined()){
            printError(funcVal->getName() + " is already defined");
            paramList.clear();
            return;
        } else if (returnType->getName() != func->getReturnType()){
            printError(funcVal->getName() + " return type doesn't match");
            paramList.clear();
            return;
        } else if (func->getIdType() != "FUNCTION"){
            printError(funcVal->getName() + " is not a function");
            paramList.clear();
            return;

        } else if (!func->isFuncDefined() ){
            if (paramList.size() != func->paramList.size()){
                printError(funcVal->getName() + " Number of parameters in definition is not equal to the number of paramters in prototype");
                paramList.clear();
                return;
            } else {
                for(int i = 0; i < paramList.size(); ++i){
                    if (paramList[i].type != func->paramList[i].type){
                        printError(funcVal->getName() + "Function parameter mismatch");
                        paramList.clear();
                        return;
                    } else {
                        func->paramList[i].name = paramList[i].name;
                    }
                }
            }
        
        }
    }

    else {
        for(param p : paramList){
            funcVal->addParam(p.name, p.type);
        }
        funcVal->setIdType("FUNCTION");
        funcVal->setReturnType(returnType->getName());
        funcVal->setFuncDefined(true);
        st.insertSymbol(funcVal);
    }
    
    //Set the current function name
    currentFunction = funcVal->getName();
}


void addFuncDecl (SymbolInfo *funcVal, SymbolInfo *returnType)
{
    SymbolInfo *func = st.lookup(funcVal->getName());

    if (func != NULL && func->getIdType() != "FUNCTION" && !func->isFuncDefined()){
        printError(funcVal->getName() + " is already declared");
        paramList.clear();
        return;
    } else {
        SymbolInfo *func = new SymbolInfo(funcVal->getName(), "ID");
        func->setIdType("FUNCTION");
        func->setReturnType(returnType->getName());
        func->setVarType(returnType->getName());
        
        for(param p : paramList){
            func->addParam(p.name, p.type);
        }

        st.insertSymbol(func);
    }
}

SymbolInfo *insertVar(SymbolInfo *var)
{   
    if (st.lookup(var->getName())){
        printError(var->getName() + " is already declared");
    
    } else if (type == "VOID") {
        printError(var->getName() + " is of void type");

    } else {
        var->setVarType(type);
        var->setIdType("VARIABLE");
        st.insertSymbol(var);
        return var;
    }
    return nullSym();
}

void addParam (string name, string type)
{
    param p(name, type);
    paramList.push_back(p);
}

void insertArray(SymbolInfo *id, SymbolInfo *size)
{
    if (!insertSymbol(id)){
        printError(id->getName() + " is already declared");
        return;
    } else if (type == "VOID"){
        printError(id->getName() + " is of void type");
    }
    id->setIdType("ARRAY");
    id->setArrSize(stoi(size->getName()));
    id->setVarType(type);

    for(int i = 0; i < id->getArrSize(); ++i){
        id->intData.push_back(0);
        id->floatData.push_back(0);
    }

}

SymbolInfo* getConstVal(SymbolInfo *val, string varType)
{   

    
    val->setIdType("VARIABLE");
    val->setVarType(varType);

    if (varType == "FLOAT")
    {
        val->setFloatValue(stof(val->getName()));
    } else if (varType == "INT")
    {
        val->setIntValue(stoi(val->getName()));
    }
    
    return val;
}

SymbolInfo* getArrayIndexVar(SymbolInfo *arr, SymbolInfo *index)
{
    SymbolInfo *arrIdxVar = st.lookup(arr->getName());
    SymbolInfo *var;
    if (arrIdxVar == NULL)
    {
        printError(arr->getName() + " is not declared");
        return nullSym();
    }
    else
    {
        if (!arrIdxVar->isArray())
        {
            printError(arrIdxVar->getName() + " is not an array");
            return nullSym();
        }
        else if (index->getVarType() != "INT")
        {
            printError(arrIdxVar->getName() + " array index must be an integer");
            return nullSym();
        }
         else {
             var = new SymbolInfo(*arrIdxVar);
             var->setArrIndex(index->getIntValue());
             var->setName(arr->getName()+"["+to_string(index->getIntValue())+"]");
             var->setReal(arrIdxVar);
        }

    }
    return var;
}

SymbolInfo* handle_assign(SymbolInfo *sym1, SymbolInfo *sym2)
{
    SymbolInfo *result;

    if(sym1->getVarType() == "VOID" || sym2->getVarType() == "VOID")
    {
        printError("Operand of void type");
        return nullSym();
    }

    else if (sym2->getVarType() == "INT")
    {
        if (sym1->getVarType() == "FLOAT") 
        {
            printWarning("Assigning integer to a float variable");
            sym1->setFloatValue(sym2->getIntValue());
        } 
        else if (sym1->getVarType() == "INT")
        {
            sym1->setIntValue(sym2->getIntValue());
        }
        
        
        
    } 
    else if (sym2->getVarType() == "FLOAT")
    {
        if (sym1->getVarType() == "FLOAT") 
        {
            sym1->setFloatValue(sym2->getFloatValue());
        } 
        else if (sym1->getVarType() == "INT")
        {
            printWarning("Assigning float to an integer variable");
            sym1->setIntValue(sym2->getFloatValue());
        }
        
        
    }
    result = new SymbolInfo(*sym1);
    result->setName(sym1->getName() + "=" + sym2->getName());
    result->setIdType("VARIABLE");
    // cout<<result->getName()<<" -- "<<sym1->getIntValue()<<" "<<sym2->getFloatValue()<<endl;
    return result;
    
}

SymbolInfo* handleADDOP(SymbolInfo* sym1, SymbolInfo* op, SymbolInfo* sym2)
{
    if(sym1->getVarType() == "VOID" || sym2->getVarType() == "VOID")
    {
        printError("Operand of void type");
        return nullSym();
    }

    SymbolInfo *result = new SymbolInfo("", "");
    if (sym1->getVarType() == "FLOAT" || sym2->getVarType() == "FLOAT" )
    {
        result->setVarType("FLOAT");
    }
    else 
    {
        result->setVarType("INT");
    }
    result->setIdType("VARIABLE");


    string ADDOP = op->getName();
    if (ADDOP == "+"){
        if (sym1->isVariable() || sym1->isArray()){
            if (sym2->isVariable() || sym2->isArray()){
                if (sym1->getVarType() == "FLOAT"){
                    if (sym2->getVarType() == "INT"){
                        result->setFloatValue(sym1->getFloatValue()+sym2->getIntValue());
                    } else {
                        result->setFloatValue(sym1->getFloatValue()+sym2->getFloatValue());
                    }                    
                } else if (sym1->getVarType() == "INT"){
                    if (sym2->getVarType() == "INT"){
                        result->setIntValue(sym1->getIntValue()+sym2->getIntValue());
                        // cout<<result->getIntValue()<<" "<<result->getVarType()<<endl;
                    } else {
                        result->setFloatValue(sym1->getIntValue()+sym2->getFloatValue());
                    }
                }
            } else if (sym2->isArray()){

            }
        }
    } 
    else if (ADDOP == "-")
    {
        if (sym1->isVariable() || sym1->isArray()){
            if (sym2->isVariable() || sym2->isArray()){
                if (sym1->getVarType() == "FLOAT"){
                    if (sym2->getVarType() == "INT"){
                        result->setFloatValue(sym1->getFloatValue()-sym2->getIntValue());
                    } else {
                        result->setFloatValue(sym1->getFloatValue()-sym2->getFloatValue());
                    }                    
                } else if (sym1->getVarType() == "INT"){
                    if (sym2->getVarType() == "INT"){
                        result->setIntValue(sym1->getIntValue()-sym2->getIntValue());
                    } else {
                        result->setFloatValue(sym1->getIntValue()-sym2->getFloatValue());
                    }
                }
            }
        }
    }
    result->setName(sym1->getName() + ADDOP + sym2->getName());
    return result;
}

SymbolInfo* handle_RELOP (SymbolInfo *sym1, SymbolInfo *op, SymbolInfo *sym2)
{
    if (sym1->getVarType() == "VOID" || sym2->getVarType() == "VOID"){
        printError("Void type expressions are not comparable");
        return nullSym();
    }

    if (sym1->getVarType() != sym2->getVarType()){
        printWarning("Comparison between different types");
    }

    string relop = op->getName();



    SymbolInfo *result = new SymbolInfo("","");
    result->setVarType("INT");
    result->setIdType("VARIABLE");

    int leftInt = 0, rightInt = 0;
    float leftFloat = 0.0, rightFloat = 0.0;
    int resultValue = 0;

    if (sym1->getVarType() == "INT" && sym2->getVarType() == "INT"){
        leftInt = sym1->getIntValue();
        rightInt = sym2->getIntValue();
        
        if (relop == "=="){
            resultValue = leftInt == rightInt;

        } else if (relop == "!="){
            resultValue = leftInt != rightInt;

        } else if (relop == "<="){
            resultValue = leftInt <= rightInt;

        } else if (relop == ">="){
            resultValue = leftInt >= rightInt;
        } else if (relop == ">"){
            resultValue = leftInt > rightInt;
        } else if (relop == "<"){
            resultValue = leftInt < rightInt;
        } 
    } else if (sym1->getVarType() == "INT" && sym2->getVarType() == "FLOAT"){
        leftInt = sym1->getIntValue();
        rightFloat = sym2->getFloatValue();
        if (relop == "=="){
            resultValue = leftInt == rightFloat;

        } else if (relop == "!="){
            resultValue = leftInt != rightFloat;

        } else if (relop == "<="){
            resultValue = leftInt <= rightFloat;

        } else if (relop == ">="){
            resultValue = leftInt >= rightFloat;
        } else if (relop == ">"){
            resultValue = leftInt > rightFloat;
        } else if (relop == "<"){
            resultValue = leftInt < rightFloat;
        } 
    } else if (sym1->getVarType() == "FLOAT" && sym2->getVarType() == "INT"){
        leftFloat = sym1->getFloatValue();
        rightInt = sym2->getIntValue();
        if (relop == "=="){
            resultValue = leftFloat == rightInt;

        } else if (relop == "!="){
            resultValue = leftFloat != rightInt;

        } else if (relop == "<="){
            resultValue = leftFloat <= rightInt;

        } else if (relop == ">="){
            resultValue = leftFloat >= rightInt;
        } else if (relop == ">"){
            resultValue = leftFloat > rightInt;
        } else if (relop == "<"){
            resultValue = leftFloat < rightInt;
        } 
    } else if (sym1->getVarType() == "FLOAT" && sym2->getVarType() == "FLOAT"){
        leftFloat = sym1->getFloatValue();
        rightFloat = sym2->getFloatValue();
        if (relop == "=="){
            resultValue = leftFloat == rightFloat;

        } else if (relop == "!="){
            resultValue = leftFloat != rightFloat;

        } else if (relop == "<="){
            resultValue = leftFloat <= rightFloat;

        } else if (relop == ">="){
            resultValue = leftFloat >= rightFloat;
        } else if (relop == ">"){
            resultValue = leftFloat > rightFloat;
        } else if (relop == "<"){
            resultValue = leftFloat < rightFloat;
        } 
    }
    result->setIntValue(resultValue);
    result->setName(sym1->getName() + relop + sym2->getName());
    // cout<<resultValue<<endl;
    return result; 
}

SymbolInfo* handle_MULOP (SymbolInfo *sym1, SymbolInfo *op, SymbolInfo *sym2)
{
    if (sym1->getVarType() == "VOID" || sym2->getVarType() == "VOID"){
        printError("Void type operand");
        return nullSym();
    }

    string mulOp = op->getName();

    if (mulOp == "%" && (sym1->getVarType() == "FLOAT" || sym2->getVarType() == "FLOAT")){
        printError("Float operand for mod operation");
        return nullSym();
    }

    SymbolInfo *result = new SymbolInfo("", "VARIABLE");

    if (sym1->getVarType() == "FLOAT" || sym2->getVarType() == "FLOAT"){
        result->setVarType("FLOAT");
    } else {
        result->setVarType("INT");
    }
    result->setIdType("VARIABLE");

    if (mulOp == "%"){
        result->setIntValue(sym1->getIntValue() % sym2->getIntValue());
    } else if (mulOp == "*"){
        if (sym1->getVarType() == "INT" && sym2->getVarType() == "INT"){
            result->setIntValue(sym1->getIntValue() * sym2->getIntValue());
        } else if (sym1->getVarType() == "INT" && sym2->getVarType() == "FLOAT") {
            result->setFloatValue(sym1->getIntValue() * sym2->getFloatValue());
        } else if (sym1->getVarType() == "FLOAT" && sym2->getVarType() == "INT") {
            result->setFloatValue(sym1->getFloatValue() * sym2->getIntValue());
        } else if (sym1->getVarType() == "FLOAT" && sym2->getVarType() == "FLOAT") {
            result->setFloatValue(sym1->getFloatValue() * sym2->getFloatValue());
        }
    } else if (mulOp == "/"){
        if (sym2->getIntValue() == 0 || sym2->getFloatValue() == 0.0){
            printError("Divide by zero");
            return nullSym();
        }
        if (sym1->getVarType() == "INT" && sym2->getVarType() == "INT"){
            result->setIntValue(sym1->getIntValue() / sym2->getIntValue());
        } else if (sym1->getVarType() == "INT" && sym2->getVarType() == "FLOAT") {
            result->setFloatValue((sym1->getIntValue()*1.0) / sym2->getFloatValue());
        } else if (sym1->getVarType() == "FLOAT" && sym2->getVarType() == "INT") {
            result->setFloatValue(sym1->getFloatValue() / (sym2->getIntValue()*1.0));
        } else if (sym1->getVarType() == "FLOAT" && sym2->getVarType() == "FLOAT") {
            result->setFloatValue(sym1->getFloatValue() / sym2->getFloatValue());
        }
    }
    result->setName(sym1->getName()+mulOp+sym2->getName());
    return result;
}

SymbolInfo* handle_INCOP(SymbolInfo *sym1)
{
    SymbolInfo *temp = new SymbolInfo("1", "CONST_INT");
    SymbolInfo *one = getConstVal(temp, "INT");
    SymbolInfo *addOp = new SymbolInfo("+", "");
    SymbolInfo *result = handle_assign(sym1, handleADDOP(sym1, addOp, one));
    result->setName(sym1->getName() + "++");
    return result;
}

SymbolInfo* handle_DECOP(SymbolInfo *sym1)
{
    SymbolInfo *temp = new SymbolInfo("1", "CONST_INT");
    SymbolInfo *one = getConstVal(temp, "INT");
    SymbolInfo *addOp = new SymbolInfo("-", "");
    SymbolInfo *result = handle_assign(sym1, handleADDOP(sym1, addOp, one));
    result->setName(sym1->getName() + "--");
    return result;
}

SymbolInfo* handle_LOGICOP(SymbolInfo *sym1, SymbolInfo *op, SymbolInfo *sym2)
{
    bool resultValue = false;
    SymbolInfo *result = new SymbolInfo("","");
    if (sym1->getVarType() == "VOID" || sym2->getVarType() == "VOID"){
        printError("Void type operand");
        return nullSym();
    }

    if (sym1->getVarType() != sym2->getVarType()){
        printWarning("Comparison between two different types");
    }

    string logicOp = op->getName();
    result->setIdType("VARIABLE");
    result->setVarType("INT");

    float leftFloat = 0, rightFloat = 0;
    int leftInt = 0, rightInt = 0;

    if (sym1->getVarType() == "INT"){
        leftInt = sym1->getIntValue();
    } else {
        leftFloat = sym1->getFloatValue();
    }

    if (sym2->getVarType() == "INT"){
        rightInt = sym2->getIntValue();
    } else {
        rightFloat = sym2->getFloatValue();
    }

    if (sym1->getVarType() == "INT" && sym2->getVarType() == "INT"){
        if (logicOp == "||") resultValue = leftInt || rightInt;
        else resultValue = leftInt && rightInt;
    } else if (sym1->getVarType() == "FLOAT" && sym2->getVarType() == "INT"){
        if (logicOp == "||") resultValue = leftFloat || rightInt;
        else resultValue = leftFloat && rightInt;
    } else if (sym1->getVarType() == "INT" && sym2->getVarType() == "FLOAT"){
        if (logicOp == "||") resultValue = leftInt || rightFloat;
        else resultValue = leftInt && rightFloat;
    } else if (sym1->getVarType() == "FLOAT" && sym2->getVarType() == "FLOAT"){
        if (logicOp == "||") resultValue = leftFloat || rightFloat;
        else resultValue = leftFloat && rightFloat;
    }

    result->setIntValue(resultValue);
    result->setName(sym1->getName()+logicOp+sym2->getName());
    return result;
}

SymbolInfo *handle_function(SymbolInfo *funcVal, SymbolInfo *argList){
    SymbolInfo *func = st.lookup(funcVal->getName());

    if (func == NULL){
        printError("Function " + funcVal->getName() + " is not declared or defined");
        argTypeList.clear();
    } else if (!func->isFuncDefined()){
        printError("Function " + funcVal->getName() + " is not defined");
        argTypeList.clear();
    } else if (!func->isFunction()){
        printError(funcVal->getName() + " is not a function");
        argTypeList.clear();
    } else if (argTypeList.size() != func->paramList.size()) {
        printError(funcVal->getName() + " argument number mismatch");
        argTypeList.clear();
    } else {
        for(int i = 0; i < func->paramList.size(); i++){
            if (func->paramList[i].type != argTypeList[i]){
                printError(funcVal->getName() + " argument type mismatch for" + func->paramList[i].name);
                argTypeList.clear();
                return nullSym();
            }
        }
    }

    SymbolInfo *sym = new SymbolInfo(funcVal->getName() + "(" + argList->getName() + ")", "NON_TERMINAL");
    return sym;
    
}

void handle_print(SymbolInfo *sym){

    sym = getVariable(sym);

    if (sym->getIdType() != "VARIABLE") {
        printError(sym->getName() + " is not a variable");
        return;
    } 
    if (sym->getVarType() == "VOID"){
        printError(sym->getName() + " is of void type");
        return;
    }
    
    if (sym->getVarType() == "INT"){
        log << sym->getIntValue()<<endl;
    } else {
        log << sym->getFloatValue()<<endl;
    }
}

SymbolInfo *handle_unary_ADDOP(SymbolInfo *op, SymbolInfo *sym){
    string opr = op->getName();

    if (sym->getIdType() != "VARIABLE"){
        printError(sym->getName() + " is not an expression");
        return nullSym();
    } 
    if (sym->getVarType() == "VOID"){
        printError(sym->getName() + " is of void type");
        return nullSym();
    }

    if (opr == "-"){
        if (sym->getVarType() == "INT"){
            sym->setIntValue(-sym->getIntValue());
        } else {
            sym->setFloatValue(-sym->getFloatValue());
        }
        sym->setName(sym->getName());
    } else {
        sym->setName(sym->getName());
    } 
    
    return sym;
}

SymbolInfo *handle_NOT(SymbolInfo *sym){
    if (sym->getIdType() != "VARIABLE"){
        printError(sym->getName() + " is not an expression");
        return nullSym();
    } 
    if (sym->getVarType()== "VOID"){
        printError(sym->getName() + " is of void type");
        return nullSym();
    }

    if (sym->getVarType() == "INT"){
        if (sym->getIntValue() != 0){
            sym->setIntValue(0);
        } else {
            sym->setIntValue(1);
        }
    } else {
        if (sym->getFloatValue() != 0.0){
            sym->setFloatValue(0.0);
        } else {
            sym->setFloatValue(1.0);
        }
    }

    sym->setName(sym->getName());

    return sym;
}
#endif
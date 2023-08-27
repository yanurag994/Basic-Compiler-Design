#pragma once
#include "./Lexer.hpp"
#include "./Scope.hpp"
#include <vector>
#include <iostream>
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>

class Parser
{
private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder;
    llvm::Module module;
    llvm::Type *getLLVMType(token_type type);
    token cur_tk;
    token prev_tk;
    bool global_flag = false;
    bool returned_flag_for_if = false;
    bool scan_assume(token_type);
    bool optional_scan_assume(token_type);
    bool scan_assume(token_type, token &, bool);
    bool optional_scan_assume(token_type, token &, bool);
    bool resync(token_type, bool);
    bool program_header();
    bool program_body();
    bool declaration();
    bool procedure_declaration(token &);
    bool procedure_header(token &);
    bool parameter_list(std::vector<token> &);
    bool procedure_body(token &);
    bool variable_declaration(token &);
    bool type_mark(token_type &);
    bool statement();
    bool procedure_call(token &, llvm::Value *&);
    bool assignment_statement(token &);
    bool destination(token &, llvm::Value *&);
    bool if_statement();
    bool loop_statement();
    bool return_statement();
    bool expression(token &, llvm::Value *&);
    bool cond_expression(llvm::Value *&);
    bool arithOp(token &, llvm::Value *&);
    bool relation(token &, llvm::Value *&);
    bool factor(token &, llvm::Value *&);
    bool argument_list(llvm::Function *calleeFunc, std::vector<llvm::Value *> &args);
    void printf();
    void scanf();
    void strcmp();
    void malloc();
    void putinteger();
    void putfloat();
    void putstring();
    void putbool();
    llvm::Value *getinteger();
    llvm::Value *getfloat();
    llvm::Value *getstring();
    llvm::Value *getbool();
    llvm::Value *sqrt();
    llvm::Function *mainFunc;
    std::error_code ec;
    llvm::raw_fd_ostream *dest;
    llvm::raw_fd_ostream *console;

public:
    Lexer lexer_handle;
    bool program();
    Symbols *symbols;
    Parser(const std::string &inputFileName, const std::string &outputFileName) : lexer_handle(inputFileName), builder(context), module(inputFileName, context)
    {
        symbols = new Symbols();
        cur_tk = lexer_handle.scan();
        dest = new llvm::raw_fd_ostream(outputFileName, ec);
        console = new llvm::raw_fd_ostream("-", ec);
    };
    Parser(const std::string &inputFileName) : Parser(inputFileName, "-") {}
    void initialize()
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        printf();
        scanf();
        malloc();
        strcmp();
        putinteger();
        putfloat();
        putstring();
        putbool();
        getinteger();
        getfloat();
        getbool();
        getstring();
        sqrt();
    };
    void execute()
    {
        module.print(*dest, nullptr);
        if (!llvm::verifyModule(module, console))
        {
            llvm::ExecutionEngine *engine = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(&module)).create();
            engine->runFunction(mainFunc, {});
            std::cout << "Completed Execution, Enter any number to exit" << std::endl;
            int k;
            std::cin >> k;
        }
        else
        {
            std::cout << "LLVM Module contains above errors, code will not execute further" << std::endl;
        }
    };
};

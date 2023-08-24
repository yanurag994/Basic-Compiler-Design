#include "./Parser.hpp"
#include <sstream>
#include <string>
#include <unistd.h>

bool Parser::scan_assume(token_type type, token &returned, bool definition = false)
{
  if (optional_scan_assume(type, returned, definition))
  {
    return true;
  }
  else
  {
    std::stringstream ss;
    ss << "At token " << cur_tk.type << ", expecting token " << type << std::endl;
    lexer_handle.reportError(ss.str());
    return false;
  }
}

bool Parser::optional_scan_assume(token_type type, token &returned, bool definition = false)
{
  if (cur_tk.type == type)
  {
    if (cur_tk.type == IDENTIFIER)
      returned = symbols->HashLookup(cur_tk, global_flag, definition);
    else
      returned = cur_tk;
    if (type != (token_type)'.')
    {
      cur_tk = lexer_handle.scan();
    }
    return true;
  }
  return false;
}

bool Parser::scan_assume(token_type type)
{
  if (optional_scan_assume(type))
  {
    return true;
  }
  else
  {
    std::stringstream ss;
    ss << "At token " << cur_tk.type << ", expecting token " << type << std::endl;
    lexer_handle.reportError(ss.str());
    return false;
  }
}

bool Parser::optional_scan_assume(token_type type)
{
  if (cur_tk.type == type)
  {
    if (type != (token_type)'.')
    {
      cur_tk = lexer_handle.scan();
    }
    return true;
  }
  return false;
}

bool Parser::resync(token_type type, bool ahead = false)
{
  while (cur_tk.type != type && cur_tk.type != T_EOF)
    cur_tk = lexer_handle.scan();
  if (cur_tk.type == type)
  {
    cur_tk = lexer_handle.scan();
    std::cout << "Skipping till token " << cur_tk.type << std::endl;
    return true;
  }
  std::cout << "Unable to continue parsing. Reached EOF while attempting to resync" << std::endl;
  return false;
}

bool Parser::program() // Complete
{
  return (program_header() && program_body() && scan_assume((token_type)'.')) ? true : false;
}

bool Parser::program_header() // Complete
{
  return (scan_assume(PROGRAM_RW) && scan_assume(IDENTIFIER) && scan_assume(IS_RW)) ? true : resync(IS_RW);
}

bool Parser::program_body() // Complete
{
  while (declaration())
    ;
  if (scan_assume(BEGIN_RW))
  {
    llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
    llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", &module);
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entryBlock);
    while (statement() && scan_assume((token_type)';'))
      ;
    if (scan_assume(END_RW) && scan_assume(PROGRAM_RW))
    {
      builder.CreateRetVoid();
      llvm::verifyFunction(*mainFunc);
      std::error_code ec;
      llvm::raw_fd_ostream dest("-", ec);
      module.print(dest, nullptr);
      return true;
    }
    else
      return false;
  }
  return resync(PROGRAM_RW);
}

bool Parser::declaration()
{
  global_flag = optional_scan_assume(GLOBAL_RW);
  token decl;
  if (optional_scan_assume(PROCEDURE_RW) && procedure_declaration(decl))
    return true;
  if (optional_scan_assume(VARIABLE_RW) && variable_declaration(decl) && scan_assume((token_type)';'))
  {
    llvm::Type *dataType = getLLVMType(decl.dataType);
    if (dataType)
    {
      llvm::Value *variable;
      if (decl.global_var)
        variable = new llvm::GlobalVariable(module, dataType, false, llvm::GlobalValue::ExternalLinkage, nullptr, decl.tokenMark.stringValue);
      else
        variable = builder.CreateAlloca(dataType, nullptr, decl.tokenMark.stringValue);
      decl.llvm_value = variable;
      symbols->Completetoken(decl);
      return true;
    }
  }
  return false;
}

bool Parser::type_mark(token_type &dType)
{
  token returned;
  if (optional_scan_assume(INTEGER_RW, returned) || optional_scan_assume(FLOAT_RW, returned) || optional_scan_assume(STRING_RW, returned) || optional_scan_assume(BOOLEAN_RW, returned))
  {
    dType = returned.type;
    return true;
  }
  else
  {
    lexer_handle.reportError("Expected a type identifier");
    return false;
  }
}

bool Parser::procedure_declaration(token &proc) // Complete
{
  return procedure_header(proc) && procedure_body(proc) ? true : resync(PROCEDURE_RW, true);
}

bool Parser::procedure_header(token &proc)
{
  if (scan_assume(IDENTIFIER, proc, true) && scan_assume(TYPE_SEPERATOR) && type_mark(proc.dataType))
  {
    symbols->enterScope();
    if (scan_assume((token_type)'(') && parameter_list(proc.argType) && scan_assume((token_type)')'))
    {
      symbols->CompleteDeclPrevtoken(proc); // For outer Scope
      symbols->Completetoken(proc);
      std::vector<llvm::Type *> llvmargs;
      for (auto &argInfo : proc.argType)
        llvmargs.push_back(getLLVMType(argInfo.dataType));
      llvm::FunctionType *funcType = llvm::FunctionType::get(getLLVMType(proc.dataType), llvmargs, false);
      llvm::Function *Func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, proc.tokenMark.stringValue, &module);
      llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", Func);
      builder.SetInsertPoint(entryBlock);
      auto argIt = Func->arg_begin();
      for (auto &argInfo : proc.argType)
      {
        llvm::Type *argLLVMType = getLLVMType(argInfo.dataType);
        llvm::Argument *arg = &*argIt;
        argIt->setName(argInfo.tokenMark.stringValue);
        llvm::AllocaInst *allocaInst = builder.CreateAlloca(argLLVMType);
        argInfo.llvm_value = allocaInst;
        symbols->Completetoken(argInfo);
        builder.CreateStore(arg, allocaInst);
        ++argIt;
      }
      return true;
    }
    return false;
  }
  else
  {
    return resync((token_type)')');
  }
}

bool Parser::procedure_body(token &proc)
{
  while (declaration())
    ;
  if (scan_assume(BEGIN_RW))
    ;
  while (statement() && scan_assume((token_type)';'))
    ;
  if (scan_assume(END_RW) && scan_assume(PROCEDURE_RW) && scan_assume((token_type)';'))
  {
    symbols->exitScope();
    return true;
  }
  lexer_handle.reportWarning("Resync not possible at this stage");
  return false;
}

bool Parser::parameter_list(std::vector<token> &argType)
{
  if (optional_scan_assume(VARIABLE_RW))
  {
    token arg;
    auto valid = parameter(arg);
    symbols->Completetoken(arg);
    argType.push_back(arg);
    if (optional_scan_assume((token_type)','))
      return parameter_list(argType);
    return true;
  }
  else
    return true;
}

bool Parser::parameter(token &param)
{
  auto temp = variable_declaration(param);
  return temp;
}

bool Parser::variable_declaration(token &var)
{
  if (scan_assume(IDENTIFIER, var, true) && scan_assume(TYPE_SEPERATOR) && type_mark(var.dataType))
  {
    token temp;
    if (optional_scan_assume((token_type)'[') && scan_assume(INTEGER_VAL, temp))
    {
      var.size = temp.tokenMark.intValue;
      if (!scan_assume((token_type)']'))
      {
        resync((token_type)']');
        return false;
      }
    }
    return true;
  }
  return false;
}

bool Parser::statement()
{
  if (optional_scan_assume(IF_RW) && if_statement())
    return true;
  if (optional_scan_assume(FOR_RW) && loop_statement())
    return true;
  if (optional_scan_assume(RETURN_RW) && return_statement())
    return true;
  token var;
  if (optional_scan_assume(IDENTIFIER, var) && assignment_statement(var))
    return true;
  return false;
}

bool Parser::if_statement()
{
  llvm::Value *result;
  if (scan_assume((token_type)'(') && cond_expression(result) && scan_assume((token_type)')') && scan_assume(THEN_RW))
  {
    llvm::Function *function = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(context, "if", function);
    llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(context, "else");
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context, "continue");
    builder.CreateCondBr(result, thenBlock, elseBlock);
    builder.SetInsertPoint(thenBlock);
    while (statement() && scan_assume((token_type)';'))
      ;
    builder.CreateBr(mergeBlock);
    function->getBasicBlockList().push_back(elseBlock);
    builder.SetInsertPoint(elseBlock);
    if (optional_scan_assume(ELSE_RW))
      while (statement() && scan_assume((token_type)';'))
        ;
    builder.CreateBr(mergeBlock);
    function->getBasicBlockList().push_back(mergeBlock);
    builder.SetInsertPoint(mergeBlock);
    if (scan_assume(END_RW) && scan_assume(IF_RW))
      return true;
  }
  return false;
}

bool Parser::assignment_statement(token &dest)
{
  if (destination(dest))
  {
    llvm::Value *LHS = dest.llvm_value;
    if (scan_assume(EQUAL_ASSIGN))
    {
      token result;
      llvm::Value *RHS = nullptr;
      if (expression(result, RHS))
      {
        builder.CreateStore(RHS, LHS);
        return true;
      }
    }
  }
  return false;
}

bool Parser::destination(token &var)
{
  if (optional_scan_assume((token_type)'['))
  {
    token result;
    llvm::Value *size;
    while (expression(result,  size))
      ;
    if (scan_assume((token_type)']'))
      return true;
  }
  return true;
}

bool Parser::loop_statement()
{
  return true;
  /*   token var, exp_result;
    std::stringstream exp;
    if (scan_assume((token_type)'(') && scan_assume(IDENTIFIER, var) && assignment_statement(var) && scan_assume((token_type)';') && expression(exp_result, exp) && scan_assume((token_type)')'))
    {
      while (statement() && scan_assume((token_type)';'))
        ;
      if (scan_assume(END_RW) && scan_assume(FOR_RW))
        return true;
    }
    return false; */
}

bool Parser::return_statement()
{
  token result;
  llvm::Value *returnValue;
  if (expression(result,  returnValue))
  {
    builder.CreateRet(returnValue);
    return true;
  }
  return false;
}

bool Parser::procedure_call(token &proc, llvm::Value *&returnValue)
{
  llvm::Function *calleeFunc = module.getFunction(proc.tokenMark.stringValue);
  if (!calleeFunc)
  {
    // Error: The function is not defined
    return false;
  }
  std::vector<llvm::Value *> arguments;
  if (optional_scan_assume((token_type)')'))
    return true;
  if (argument_list(calleeFunc, arguments))
  {
    if (scan_assume((token_type)')'))
    {
      returnValue = builder.CreateCall(calleeFunc, arguments);
      // Process the returnValue if needed
      return true;
    }
  }
  return false;
}

bool Parser::argument_list(llvm::Function *calleeFunc, std::vector<llvm::Value *> &args)
{
  token result;
  llvm::Value *argValue;
  if (expression(result, argValue))
  {
    args.push_back(argValue);
    if (optional_scan_assume((token_type)','))
    {
      return argument_list(calleeFunc, args);
    }
    return true;
  }
  return false;
}

bool Parser::cond_expression(llvm::Value *&result)
{
  token exp_result;
  if (expression(exp_result,  result))
  {
    return true;
  }
  else
    return false;
}

bool Parser::expression(token &exp_result,  llvm::Value *&value)
{
  optional_scan_assume((token_type)'!'); // Might be replaced if not is a keyword
  if (arithOp(exp_result, value))
  {
    if (optional_scan_assume((token_type)'|') || optional_scan_assume((token_type)'&'))
    {
      llvm::Value *secondValue = nullptr;
      bool isOr = exp_result.type == (token_type)'|';

      // Recursive call to handle the second part of the expression
      if (expression(exp_result, secondValue))
      {
        // Generate LLVM IR for logical OR or AND
        if (isOr)
        {
          value = builder.CreateOr(value, secondValue, "ortmp");
        }
        else
        {
          value = builder.CreateAnd(value, secondValue, "andtmp");
        }
        return true;
      }
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    return true;
  }
}

bool Parser::arithOp(token &var, llvm::Value *&value)
{
  token term_1, op;
  llvm::Value *value_1 = nullptr;
  // Parse the first term using relation
  if (relation(term_1, value_1))
  {
    // Handle optional arithmetic operations in a loop
    while (optional_scan_assume((token_type)'+', op) || optional_scan_assume((token_type)'-', op) ||
           optional_scan_assume((token_type)'*', op) || optional_scan_assume((token_type)'/', op))
    {
      token term_2;
      llvm::Value *value_2 = nullptr;

      // Parse the next term using relation
      if (relation(term_2,  value_2))
      {
        // Generate LLVM IR for the arithmetic operation based on the operator
        llvm::Instruction::BinaryOps binaryOp = llvm::Instruction::BinaryOpsEnd;
        if (op.type == (token_type)'+')
        {
          binaryOp = llvm::Instruction::Add;
        }
        else if (op.type == (token_type)'-')
        {
          binaryOp = llvm::Instruction::Sub;
        }
        else if (op.type == (token_type)'*')
        {
          binaryOp = llvm::Instruction::Mul;
        }
        else if (op.type == (token_type)'/')
        {
          binaryOp = llvm::Instruction::SDiv;
        }

        if (binaryOp != llvm::Instruction::BinaryOpsEnd)
        {
          // Generate LLVM IR for the arithmetic operation
          value_1 = builder.CreateBinOp(binaryOp, value_1, value_2, "");
        }
        else
        {
          // Handle unsupported operator
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    // Assign the final result to the output parameter
    var = term_1;
    value = value_1;

    return true;
  }

  return false;
}

bool Parser::relation(token &var, llvm::Value *&value)
{
  token var_1, var_2, op;
  llvm::Value *value_1 = nullptr, *value_2 = nullptr;
  if (factor(var_1, value_1))
  {
    if (optional_scan_assume((token_type)'<', op) || optional_scan_assume(LESS_EQUAL, op) || optional_scan_assume(GREATER_EQUAL, op) ||
        optional_scan_assume((token_type)'>', op) || optional_scan_assume(EQUALITY, op) || optional_scan_assume(NOT_EQUAL, op))
    {
      if (factor(var_2, value_2))
      {
        // Generate LLVM IR for the comparison based on the operator
        llvm::CmpInst::Predicate predicate = llvm::CmpInst::ICMP_EQ; // Default to equal comparison
        if (op.type == (token_type)'<')
        {
          predicate = llvm::CmpInst::ICMP_SLT;
        }
        else if (op.type == LESS_EQUAL)
        {
          predicate = llvm::CmpInst::ICMP_SLE;
        }
        else if (op.type == GREATER_EQUAL)
        {
          predicate = llvm::CmpInst::ICMP_SGE;
        }
        else if (op.type == (token_type)'>')
        {
          predicate = llvm::CmpInst::ICMP_SGT;
        }
        else if (op.type == EQUALITY)
        {
          predicate = llvm::CmpInst::ICMP_EQ;
        }
        else if (op.type == NOT_EQUAL)
        {
          predicate = llvm::CmpInst::ICMP_NE;
        }
        // Generate LLVM IR for the comparison
        value = builder.CreateICmp(predicate, value_1, value_2, "");

        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      // No relational operator, assign the value from var_1
      var = var_1;
      value = value_1;
      return true;
    }
  }
  else
  {
    return false;
  }
}

bool Parser::factor(token &var, llvm::Value *&value)
{

  if (optional_scan_assume(TRUE_RW, var) || optional_scan_assume(FALSE_RW, var))
  {
    value = builder.getInt32(var.type == TRUE_RW);
    return true;
  }
  else if (optional_scan_assume(STRING_VAL, var))
  {
    return true;
  }
  else
  {
    bool isNegative = optional_scan_assume((token_type)'-');
    if (optional_scan_assume(INTEGER_VAL, var))
    {
      value = builder.getInt32(var.tokenMark.intValue);
      if (isNegative)
        value = builder.CreateNeg(value, "negtmp");
      return true;
    }
    else if (optional_scan_assume(FLOAT_VAL, var))
    {
      value = llvm::ConstantFP::get(context, llvm::APFloat(var.tokenMark.doubleValue));
      if (isNegative)
        value = builder.CreateNeg(value, "negtmp");
      return true;
    }
    else if (optional_scan_assume(IDENTIFIER, var))
    {
      if (optional_scan_assume((token_type)'(') && procedure_call(var, value))
        return true;
      if (destination(var))
      {
        value = builder.CreateLoad(getLLVMType(var.dataType), var.llvm_value);
        return true;
      }
      return false;
    }
    return false;
  }
}

llvm::Type *Parser::getLLVMType(token_type type)
{
  switch (type)
  {
  case INTEGER_RW:
    return builder.getInt32Ty();
  case FLOAT_RW:
    return builder.getFloatTy();
  case STRING_RW:
    return builder.getInt64Ty();
  case BOOLEAN_RW:
    return builder.getInt1Ty();
  default:
    return nullptr; // Unknown type
  }
}
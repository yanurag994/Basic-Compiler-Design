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
    mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", &module);
    mainFunc->setDSOLocal(true);
    llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entryBlock);
    while (statement() && scan_assume((token_type)';'))
      ;
    if (scan_assume(END_RW) && scan_assume(PROGRAM_RW))
    {
      builder.CreateRetVoid();
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
      {
        llvm::Constant *zeroInitializer = llvm::Constant::getNullValue(dataType);
        llvm::GlobalVariable *temp = new llvm::GlobalVariable(module, dataType, false, llvm::GlobalValue::ExternalLinkage, zeroInitializer, decl.tokenMark.stringValue);
        temp->setDSOLocal(true);
        variable = temp;
      }
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
      Func->setDSOLocal(true);
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
    returned_flag_for_if = false;
    while (statement() && scan_assume((token_type)';'))
      ;
    if (returned_flag_for_if == false)
      builder.CreateBr(mergeBlock);
    returned_flag_for_if = false;
    function->getBasicBlockList().push_back(elseBlock);
    builder.SetInsertPoint(elseBlock);
    returned_flag_for_if = false;
    if (optional_scan_assume(ELSE_RW))
      while (statement() && scan_assume((token_type)';'))
        ;
    if (returned_flag_for_if == false)
      builder.CreateBr(mergeBlock);
    returned_flag_for_if = false;
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
    while (expression(result, size))
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
  if (expression(result, returnValue))
  {
    builder.CreateRet(returnValue);
    returned_flag_for_if = true;
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
    // return false;
  }
  std::vector<llvm::Value *> arguments;
  if (optional_scan_assume((token_type)')'))
  {
    returnValue = builder.CreateCall(calleeFunc, arguments);
    return true;
  }
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
  if (expression(exp_result, result))
  {
    return true;
  }
  else
    return false;
}

bool Parser::expression(token &exp_result, llvm::Value *&value)
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
      if (relation(term_2, value_2))
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
    if (optional_scan_assume(LESS_THAN, op) || optional_scan_assume(LESS_EQUAL, op) || optional_scan_assume(GREATER_EQUAL, op) ||
        optional_scan_assume(GREATER_THAN, op) || optional_scan_assume(EQUALITY, op) || optional_scan_assume(NOT_EQUAL, op))
    {
      if (factor(var_2, value_2))
      {
        // Generate LLVM IR for the comparison based on the operator
        llvm::CmpInst::Predicate predicate = llvm::CmpInst::ICMP_EQ; // Default to equal comparison
        if (op.type == LESS_THAN)
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
        else if (op.type == GREATER_THAN)
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
    llvm::Type *stringType = llvm::Type::getInt8PtrTy(context);
    llvm::Constant *stringLiteral = llvm::ConstantDataArray::getString(context, var.tokenMark.stringValue);
    llvm::GlobalVariable *stringVar = new llvm::GlobalVariable(module, stringLiteral->getType(), true, llvm::GlobalValue::PrivateLinkage, stringLiteral);
    llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    llvm::Value *indices[] = {zero, zero};
    llvm::Constant *stringPtr = llvm::ConstantExpr::getGetElementPtr(stringLiteral->getType(), stringVar, indices);
    value = builder.CreateBitCast(stringPtr, stringType);
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
      value = llvm::ConstantFP::get(builder.getFloatTy(), (float)var.tokenMark.doubleValue);
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
    return llvm::Type::getInt8PtrTy(context);
  case BOOLEAN_RW:
    return builder.getInt1Ty();
  default:
    return nullptr; // Unknown type
  }
}

void Parser::printf()
{
  llvm::FunctionType *printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context), true);
  llvm::Function *printfFunc = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", &module);
}

void Parser::scanf()
{
  llvm::FunctionType *scanfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context), true);
  llvm::Function *scanfFunc = llvm::Function::Create(scanfType, llvm::Function::ExternalLinkage, "scanf", &module);
}

void Parser::putinteger()
{
  llvm::Function *printfFunc = module.getFunction("printf");

  // Define the putinteger function
  llvm::FunctionType *putintegerType = llvm::FunctionType::get(llvm::Type::getInt1Ty(context), llvm::Type::getInt32Ty(context), false);
  llvm::Function *putintegerFunc = llvm::Function::Create(putintegerType, llvm::Function::ExternalLinkage, "putinteger", &module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", putintegerFunc);
  builder.SetInsertPoint(entryBlock);

  // Create format string constant
  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%d\n");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  // Call printf function
  llvm::Value *value = putintegerFunc->args().begin();
  llvm::Value *printfArgs[] = {formatPtr, value};
  builder.CreateCall(printfFunc, printfArgs);
  builder.CreateRet(builder.getInt1(1));
}

void Parser::putfloat()
{
  llvm::Function *printfFunc = module.getFunction("printf");

  // Define the putfloat function
  llvm::FunctionType *putfloatType = llvm::FunctionType::get(llvm::Type::getInt1Ty(context), llvm::Type::getFloatTy(context), false);
  llvm::Function *putfloatFunc = llvm::Function::Create(putfloatType, llvm::Function::ExternalLinkage, "putfloat", &module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", putfloatFunc);
  builder.SetInsertPoint(entryBlock);

  // Create format string constant
  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%f\n");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  // Call printf function with correct format specifier for float
  llvm::Value *value = putfloatFunc->args().begin();
  llvm::Type *doubleTy = llvm::Type::getDoubleTy(context);
  llvm::Value *convertedValue = builder.CreateFPExt(value, doubleTy, "convertedValue");
  llvm::Value *printfArgs[] = {formatPtr, convertedValue};
  builder.CreateCall(printfFunc, printfArgs);
  builder.CreateRet(builder.getInt1(1));
}

void Parser::putbool()
{
  llvm::Function *printfFunc = module.getFunction("printf");

  // Define the putbool function
  llvm::FunctionType *putboolType = llvm::FunctionType::get(llvm::Type::getInt1Ty(context), llvm::Type::getInt1Ty(context), false);
  llvm::Function *putboolFunc = llvm::Function::Create(putboolType, llvm::Function::ExternalLinkage, "putbool", &module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", putboolFunc);
  builder.SetInsertPoint(entryBlock);

  // Create format string constant
  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%d\n");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  // Call printf function
  llvm::Value *value = putboolFunc->args().begin();
  llvm::Value *printfArgs[] = {formatPtr, value};
  builder.CreateCall(printfFunc, printfArgs);
  builder.CreateRet(builder.getInt1(1));
}

void Parser::putstring()
{
  llvm::Function *printfFunc = module.getFunction("printf");

  // Define the putstring function
  llvm::FunctionType *putstringType = llvm::FunctionType::get(llvm::Type::getInt1Ty(context), llvm::Type::getInt8PtrTy(context), false);
  llvm::Function *putstringFunc = llvm::Function::Create(putstringType, llvm::Function::ExternalLinkage, "putstring", &module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", putstringFunc);
  builder.SetInsertPoint(entryBlock);

  // Create format string constant
  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%s\n");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  // Call printf function
  llvm::Value *value = putstringFunc->args().begin();
  llvm::Value *printfArgs[] = {formatPtr, value};
  builder.CreateCall(printfFunc, printfArgs);
  builder.CreateRet(builder.getInt1(1));
}

llvm::Value *Parser::getinteger()
{
  llvm::Function *scanfFunc = module.getFunction("scanf");

  // Define the getinteger function
  llvm::FunctionType *getintegerType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
  llvm::Function *getintegerFunc = llvm::Function::Create(getintegerType, llvm::Function::ExternalLinkage, "getinteger", &module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", getintegerFunc);
  builder.SetInsertPoint(entryBlock);

  // Create format string constant for scanf
  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%d");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  // Allocate memory for the integer input
  llvm::Value *alloca = builder.CreateAlloca(llvm::Type::getInt32Ty(context));

  // Call scanf function
  llvm::Value *scanfArgs[] = {formatPtr, alloca};
  builder.CreateCall(scanfFunc, scanfArgs);

  // Load and return the parsed integer
  llvm::Value *intValue = builder.CreateLoad(builder.getInt32Ty(), alloca);
  builder.CreateRet(intValue);
  return getintegerFunc;
}

llvm::Value *Parser::getfloat()
{
  llvm::Function *scanfFunc = module.getFunction("scanf");

  // Define the getfloat function
  llvm::FunctionType *getfloatType = llvm::FunctionType::get(llvm::Type::getFloatTy(context), false);
  llvm::Function *getfloatFunc = llvm::Function::Create(getfloatType, llvm::Function::ExternalLinkage, "getfloat", &module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", getfloatFunc);
  builder.SetInsertPoint(entryBlock);

  // Create format string constant for scanf
  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%f");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  // Allocate memory for the float input
  llvm::Value *alloca = builder.CreateAlloca(llvm::Type::getFloatTy(context));

  // Call scanf function
  llvm::Value *scanfArgs[] = {formatPtr, alloca};
  builder.CreateCall(scanfFunc, scanfArgs);

  // Load and return the parsed float
  llvm::Value *floatValue = builder.CreateLoad(builder.getFloatTy(), alloca);
  builder.CreateRet(floatValue);
  return getfloatFunc;
}

llvm::Value *Parser::getbool()
{
  llvm::Function *scanfFunc = module.getFunction("scanf");

  // Define the getbool function
  llvm::FunctionType *getboolType = llvm::FunctionType::get(llvm::Type::getInt1Ty(context), false);
  llvm::Function *getboolFunc = llvm::Function::Create(getboolType, llvm::Function::ExternalLinkage, "getbool", &module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", getboolFunc);
  builder.SetInsertPoint(entryBlock);

  // Create format string constant for scanf
  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%d");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  // Allocate memory for the bool input
  llvm::Value *alloca = builder.CreateAlloca(llvm::Type::getInt32Ty(context));
  // llvm::Value *alloca_2 = builder.getInt32(0);

  // Call scanf function
  llvm::Value *scanfArgs[] = {formatPtr, alloca};
  builder.CreateCall(scanfFunc, scanfArgs);
  llvm::Value *scanf_result = builder.CreateLoad(builder.getInt32Ty(), alloca);

  // Load and return the parsed bool
  // llvm::Value *result=builder.CreateICmp(llvm::CmpInst::ICMP_NE, alloca, alloca_2, "");
  llvm::Value *result = builder.CreateICmpNE(scanf_result, builder.getInt32(0), "");
  builder.CreateRet(result);
  return getboolFunc;
}

llvm::Value *Parser::sqrt()
{
  llvm::FunctionType *sqrtFuncType = llvm::FunctionType::get(llvm::Type::getFloatTy(context), llvm::Type::getInt32Ty(context), false);
  llvm::Function *sqrtFunc = llvm::Function::Create(sqrtFuncType, llvm::Function::ExternalLinkage, "sqrt", &module);

  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", sqrtFunc);
  builder.SetInsertPoint(entryBlock);

  // Get the argument value
  llvm::Value *value = sqrtFunc->args().begin();
  llvm::Value *floatValue = builder.CreateSIToFP(value, builder.getFloatTy(), "");

  // Call the sqrt intrinsic function
  llvm::Function *sqrtIntrinsic = llvm::Intrinsic::getDeclaration(&module, llvm::Intrinsic::sqrt, llvm::Type::getFloatTy(context));
  llvm::Value *sqrtResult = builder.CreateCall(sqrtIntrinsic, floatValue, "");

  builder.CreateRet(sqrtResult);
  return sqrtFunc;
}

/*llvm::Value *Parser::getstring()
{
  llvm::Function *scanfFunc = module.getFunction("scanf");

  // Define the getstring function
  llvm::FunctionType *getstringType = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(context), false);
  llvm::Function *getstringFunc = llvm::Function::Create(getstringType, llvm::Function::ExternalLinkage, "getstring", &module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", getstringFunc);
  builder.SetInsertPoint(entryBlock);

  // Create format string constant for scanf
  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%s");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  // Allocate memory for the string input
  llvm::Value *alloca = builder.CreateAlloca(llvm::Type::getInt8PtrTy(context));

  llvm::Type *stringType = llvm::Type::getInt8PtrTy(context);
  llvm::Constant *stringLiteral = llvm::ConstantDataArray::getString(context,"");
  llvm::Variable *stringVar = new llvm::Variable(module, stringLiteral->getType(), true, llvm::GlobalValue::PrivateLinkage, stringLiteral);

  // Call scanf function
  llvm::Value *scanfArgs[] = {formatPtr, alloca};
  builder.CreateCall(scanfFunc, scanfArgs);

  // Load and return the parsed string
  llvm::Value *stringValue = builder.CreateLoad(llvm::Type::getInt8PtrTy(context), alloca);
  builder.CreateRet(stringValue);
  return getstringFunc;
}*/
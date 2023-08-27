#include "./Parser.hpp"

void Parser::printf()
{
  llvm::FunctionType *printfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context), true);
  llvm::Function *printfFunc = llvm::Function::Create(printfType, llvm::Function::ExternalLinkage, "printf", &module);
  return;
}

void Parser::scanf()
{
  llvm::FunctionType *scanfType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context), true);
  llvm::Function *scanfFunc = llvm::Function::Create(scanfType, llvm::Function::ExternalLinkage, "scanf", &module);
  return;
}

void Parser::malloc()
{
  llvm::FunctionType *mallocType = llvm::FunctionType::get(llvm::PointerType::get(llvm::IntegerType::get(context, 8), 0), {llvm::IntegerType::get(context, 64)}, false);
  llvm::Function *mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", module);
  return;
}

void Parser::strcmp()
{
  llvm::Type *i8PtrType = llvm::Type::getInt8PtrTy(context);
  llvm::Type *strcmpArgTypes[] = {i8PtrType, i8PtrType};
  llvm::FunctionType *strcmpType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), strcmpArgTypes, false);
  llvm::Function *strcmpFunc = llvm::Function::Create(strcmpType, llvm::Function::ExternalLinkage, "strcmp", &module);
  return;
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
  return;
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
  llvm::Value *convertedValue = builder.CreateFPExt(value, doubleTy, "");
  llvm::Value *printfArgs[] = {formatPtr, convertedValue};
  builder.CreateCall(printfFunc, printfArgs);
  builder.CreateRet(builder.getInt1(1));
  return;
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
  return;
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
  return;
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

llvm::Value *Parser::getstring()
{
  llvm::Function *scanfFunc = module.getFunction("scanf");
  llvm::Function *mallocFunc = module.getFunction("malloc");
  llvm::FunctionType *scanfType = llvm::FunctionType::get(llvm::IntegerType::get(context, 32), llvm::PointerType::get(llvm::IntegerType::get(context, 8), 0), true);

  // Define the getstring function
  llvm::FunctionType *getstringType = llvm::FunctionType::get(llvm::PointerType::get(llvm::IntegerType::get(context, 8), 0), {}, false);
  llvm::Function *getstringFunc = llvm::Function::Create(getstringType, llvm::GlobalValue::LinkageTypes::ExternalLinkage, "getstring", module);
  llvm::BasicBlock *entryBlock = llvm::BasicBlock::Create(context, "", getstringFunc);
  builder.SetInsertPoint(entryBlock);

  // Initialize the variable and allocate memory using malloc
  llvm::AllocaInst *name = builder.CreateAlloca(llvm::Type::getInt8PtrTy(context), nullptr, "name");
  llvm::CallInst *mallocCall = builder.CreateCall(mallocFunc, llvm::ConstantInt::get(context, llvm::APInt(64, 256)), "malloc");
  builder.CreateStore(mallocCall, name);
  llvm::LoadInst *nameLoad = builder.CreateLoad(llvm::PointerType::get(llvm::IntegerType::get(context, 8), 0), name);

  llvm::Constant *formatStr = llvm::ConstantDataArray::getString(context, "%s");
  llvm::GlobalVariable *formatVar = new llvm::GlobalVariable(module, formatStr->getType(), true, llvm::GlobalValue::PrivateLinkage, formatStr);
  llvm::Value *zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
  llvm::Value *indices[] = {zero, zero};
  llvm::Constant *formatPtr = llvm::ConstantExpr::getGetElementPtr(formatStr->getType(), formatVar, indices);

  llvm::CallInst *scanfCall = builder.CreateCall(scanfFunc, {formatPtr, nameLoad});
  builder.CreateRet(nameLoad);
  return getstringFunc;
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
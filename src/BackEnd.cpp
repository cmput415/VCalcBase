#include <assert.h>

#include "BackEnd.h"

BackEnd::BackEnd() : loc(mlir::UnknownLoc::get(&context)) {
    // Load Dialects.
    context.loadDialect<mlir::LLVM::LLVMDialect>();
    context.loadDialect<mlir::arith::ArithDialect>();
    context.loadDialect<mlir::scf::SCFDialect>();
    context.loadDialect<mlir::cf::ControlFlowDialect>();
    context.loadDialect<mlir::memref::MemRefDialect>(); 

    // Initialize the MLIR context 
    builder = std::make_shared<mlir::OpBuilder>(&context);
    module = mlir::ModuleOp::create(builder->getUnknownLoc());
    builder->setInsertionPointToStart(module.getBody());

    // Some intial setup to get off the ground 
    setupPrintf();
    createGlobalString("%c\0", "charFormat");
    createGlobalString("%d\0", "intFormat");
}

int BackEnd::emitModule() {

    // Create a main function 
    mlir::Type intType = mlir::IntegerType::get(&context, 32);
    auto mainType = mlir::LLVM::LLVMFunctionType::get(intType, {}, false);
    mlir::LLVM::LLVMFuncOp mainFunc = mlir::LLVM::LLVMFuncOp::create(*builder, loc, "main", mainType);
    mlir::Block *entry = mainFunc.addEntryBlock(*builder);
    builder->setInsertionPointToStart(entry);

    // Get the integer format string we already created.   
    mlir::LLVM::GlobalOp formatString;
    if (!(formatString = module.lookupSymbol<mlir::LLVM::GlobalOp>("intFormat"))) {
        llvm::errs() << "missing format string!\n";
        return 1;
    }

    // Get the format string and print 415
    mlir::Value formatStringPtr = mlir::LLVM::AddressOfOp::create(*builder, loc, formatString); 
    mlir::Value intToPrint = mlir::LLVM::ConstantOp::create(*builder, loc, intType, 415); 
    mlir::ValueRange args = {formatStringPtr, intToPrint}; 
    mlir::LLVM::LLVMFuncOp printfFunc = module.lookupSymbol<mlir::LLVM::LLVMFuncOp>("printf"); 
    mlir::LLVM::CallOp::create(*builder, loc, printfFunc, args);

    // Return 0
    mlir::Value zero = mlir::LLVM::ConstantOp::create(*builder, loc, intType, builder->getIntegerAttr(intType, 0));
    mlir::LLVM::ReturnOp::create(*builder, builder->getUnknownLoc(), zero);    
    
    module.dump();

    if (mlir::failed(mlir::verify(module))) {
        module.emitError("module failed to verify");
        return 1;
    }
    return 0;
}

int BackEnd::lowerDialects() {
    // Set up the MLIR pass manager to iteratively lower all the Ops
    mlir::PassManager pm(&context);

    // Lower SCF to CF (ControlFlow)
    pm.addPass(mlir::createSCFToControlFlowPass());

    // Lower Arith to LLVM
    pm.addPass(mlir::createArithToLLVMConversionPass());

    // Lower MemRef to LLVM
    pm.addPass(mlir::createFinalizeMemRefToLLVMConversionPass());

    // Lower CF to LLVM
    pm.addPass(mlir::createConvertControlFlowToLLVMPass());

    // Finalize the conversion to LLVM dialect
    pm.addPass(mlir::createReconcileUnrealizedCastsPass());

    // Run the passes
    if (mlir::failed(pm.run(module))) {
        llvm::errs() << "Pass pipeline failed\n";
        return 1;
    }
    return 0;
}

void BackEnd::dumpLLVM(std::ostream &os) {  
    // The only remaining dialects in our module after the passes are builtin
    // and LLVM. Setup translation patterns to get them to LLVM IR.
    mlir::registerBuiltinDialectTranslation(context);
    mlir::registerLLVMDialectTranslation(context);
    llvm_module = mlir::translateModuleToLLVMIR(module, llvm_context);

    // Create llvm ostream and dump into the output file
    llvm::raw_os_ostream output(os);
    output << *llvm_module;
}

void BackEnd::setupPrintf() {
    // Create a function declaration for printf, the signature is:
    //   * `i32 (ptr, ...)`
    mlir::Type intType = mlir::IntegerType::get(&context, 32);
    auto ptrTy = mlir::LLVM::LLVMPointerType::get(&context);
    auto llvmFnType = mlir::LLVM::LLVMFunctionType::get(intType, ptrTy,
                                                        /*isVarArg=*/true);

    // Insert the printf function into the body of the parent module.
    mlir::LLVM::LLVMFuncOp::create(*builder, loc, "printf", llvmFnType);
}

void BackEnd::createGlobalString(const char *str, const char *stringName) {

    mlir::Type charType = mlir::IntegerType::get(&context, 8);

    // create string and string type
    auto mlirString = mlir::StringRef(str, strlen(str) + 1);
    auto mlirStringType = mlir::LLVM::LLVMArrayType::get(charType, mlirString.size());

    mlir::LLVM::GlobalOp::create(*builder, loc, mlirStringType, /*isConstant=*/true,
                            mlir::LLVM::Linkage::Internal, stringName,
                            builder->getStringAttr(mlirString), /*alignment=*/0);
    return;
}
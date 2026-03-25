#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <stdint.h>
#include <string>

using namespace llvm;

extern "C" int
main_ray_tr();

int
main()
{
    LLVMContext context;
    Module*     module = new Module( "ray_tracing_app_generated", context );
    IRBuilder<> builder( context );

    Function* injected_main =
        Function::Create( FunctionType::get( Type::getInt32Ty( context ), false ),
                          Function::ExternalLinkage,
                          "main_injected",
                          module );

    Function* orig_main = Function::Create(
        FunctionType::get( builder.getInt64Ty(), Type::getInt64Ty( context ), false ),
        Function::ExternalLinkage,
        "main",
        module );

    builder.SetInsertPoint( BasicBlock::Create( context, "entrypoint", injected_main ) );
    builder.CreateRet( builder.CreateCall( orig_main, orig_main ) );

    module->print( outs(), nullptr );
    outs() << '\n';
    bool verif = verifyFunction( *injected_main, &outs() );
    outs() << "[VERIFICATION] " << ( verif ? "FAIL\n\n" : "OK\n\n" );

    outs() << "[EE] Run\n";
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    ExecutionEngine* ee = EngineBuilder( std::unique_ptr<Module>( module ) ).create();
    ee->InstallLazyFunctionCreator( []( const std::string& fnName ) -> void* {
        if ( fnName == "main" )
        {
            return reinterpret_cast<void*>( main_ray_tr );
        }
        outs() << "[ExecutionEngine] Can't find function " << fnName
               << ". Catch the Segmentation fault:)\n";
        return nullptr;
    } );
    ee->finalizeObject();
    ArrayRef<GenericValue> noargs;
    GenericValue           v = ee->runFunction( injected_main, noargs );
    outs() << "[EE] Result: " << v.IntVal << '\n';

    return 0;
}

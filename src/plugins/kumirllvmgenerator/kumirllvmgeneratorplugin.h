#ifndef KUMIR_LLVM_GENERATOR_PLUGIN_H
#define KUMIR_LLVM_GENERATOR_PLUGIN_H


#include "extensionsystem/kplugin.h"
#include "interfaces/generatorinterface.h"
#include "dataformats/ast.h"
#include "dataformats/ast_variable.h"
#include "dataformats/ast_algorhitm.h"
#define __STDC_CONSTANT_MACROS
#define __STDC_LIMIT_MACROS
#include <llvm/DerivedTypes.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/User.h>
#include <llvm/Value.h>
#include <llvm/Constant.h>
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>

namespace KumirLLVMGenerator {

class NameProvider;

class KumirLLVMGeneratorPlugin
        : public ExtensionSystem::KPlugin
        , public Shared::GeneratorInterface
{
    Q_OBJECT
    Q_INTERFACES(Shared::GeneratorInterface)
public:
    KumirLLVMGeneratorPlugin();
    ~KumirLLVMGeneratorPlugin();

    QString initialize(const QStringList &arguments);
    QPair<QString,QString> generateExecuable(
            const AST::Data *tree,
            QIODevice *out,
            QStringList *usedDlls = 0);

private:
    void prepareContext(llvm::LLVMContext & context);
    llvm::Module *addKumirModule(llvm::LLVMContext & context,
                        const AST::Data * ast, const AST::Module * module);
    void addAlgorhitmBody(llvm::LLVMContext & context,
                          llvm::Module *lmodule,
                          const AST::Data * ast,
                          const AST::Module * module,
                          const AST::Algorhitm * alg);

    static QString modulesPath();
    llvm::Type * makeType(llvm::LLVMContext & context, AST::VariableBaseType, int dimension);
    NameProvider * nameProvider;
};


}


#endif
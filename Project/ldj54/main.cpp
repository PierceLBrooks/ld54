
// Author: Pierce Brooks

#include <iostream>
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "FXPlatform/Prolog/PrologQueryCompiler.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"

int main(int argc, char** argv)
{
    shared_ptr<HtnTermFactory> factory = shared_ptr<HtnTermFactory>(new HtnTermFactory());
    shared_ptr<HtnRuleSet> state = shared_ptr<HtnRuleSet>(new HtnRuleSet());
    PrologStandardCompiler compiler(factory.get(), state.get());
    if (compiler.Compile("\nwrite(\"Compile!\").\n"))
    {
        PrologStandardQueryCompiler queryCompiler(factory.get());
        HtnGoalResolver resolver;
        if (queryCompiler.Compile("write(\"Query!\")."))
        {
            shared_ptr<vector<UnifierType>> queryResult = resolver.ResolveAll(factory.get(), state.get(), queryCompiler.result());
            if (queryResult != nullptr)
            {
                std::cout << HtnGoalResolver::ToString(queryResult.get()).c_str() << std::endl;
            }
            else
            {
                std::cout << "Query failure!" << std::endl;
            }
        }
        else
        {
            std::cout << "Query compile failure!" << std::endl;
        }
        queryCompiler.Clear();
    }
    else
    {
        std::cout << "Compile failure!" << std::endl;
    }
    return 0;
}


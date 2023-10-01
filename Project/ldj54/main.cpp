
// Author: Pierce Brooks

#include <iostream>
#include "cowl.h"
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "FXPlatform/Prolog/PrologQueryCompiler.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"

// Custom context struct for the query.
typedef struct _CustomContext {
    CowlOntology* ontology;
    UOStream* stream;
} CustomContext;

// Iterator body, invoked for each class expression matching the query.
bool forEachClass(void* iterator, CowlAny* clazz) {
    if (cowl_cls_exp_get_type(clazz) != COWL_CET_CLASS) {
        return true;
    }

    // Log the IRI remainder.
    CustomContext* context = static_cast<CustomContext*>(iterator);
    cowl_write_string(context->stream, cowl_iri_get_rem(cowl_class_get_iri(static_cast<CowlClass*>(clazz))));
    cowl_write_static(context->stream, "\n");

    // Recurse.
    CowlIterator subiterator = { context, forEachClass };
    return cowl_ontology_iterate_sub_classes(context->ontology, static_cast<CowlClass*>(clazz), &subiterator, false);
}

int main(int argc, char** argv)
{
    cowl_init();
    CowlManager *manager = cowl_manager();
    CowlOntology *ontology = cowl_manager_read_path(manager, ustring_literal("./ldj54.owl"));
    cowl_release(manager);
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
    UOStream* stream = uostream_std();
    CowlClass* clazz = cowl_class_from_static("http://127.0.0.1:8080/ldj54#myFoo");
    CustomContext context = { ontology, stream };
    CowlIterator iterator = { &context, forEachClass };
    cowl_ontology_iterate_sub_classes(ontology, clazz, &iterator, false);
    cowl_release_all(clazz, ontology);
    return 0;
}


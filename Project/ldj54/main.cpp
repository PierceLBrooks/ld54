
// Author: Pierce Brooks

#include <iostream>
#include "cowl.h"
#include "cowl_sub_cls_axiom.h"
#include "FXPlatform/Prolog/PrologCompiler.h"
#include "FXPlatform/Prolog/PrologQueryCompiler.h"
#include "FXPlatform/Prolog/HtnRuleSet.h"
#include "FXPlatform/Prolog/HtnTermFactory.h"

// Custom context struct for the query.
typedef struct _CustomContext {
    CowlOntology* ontology;
    UOStream* stream;
    CowlClass* clazz;
} CustomContext;

bool forEachAxiom(void* iterator, CowlAny* axiom) {
    CustomContext* context = static_cast<CustomContext*>(iterator);

    //std::cout << "\t" << static_cast<int>(cowl_axiom_get_type(static_cast<CowlAxiom*>(axiom))) << std::endl;
    if (cowl_axiom_get_type(static_cast<CowlAxiom*>(axiom)) != COWL_AT_SUB_CLASS) {
        //std::cout << "\t" << static_cast<int>(cowl_axiom_get_type(static_cast<CowlAxiom*>(axiom))) << std::endl;
        return true;
    }

    CowlClsExp* super = cowl_sub_cls_axiom_get_super(static_cast<CowlSubClsAxiom*>(axiom));
    CowlString* string = cowl_to_string(cowl_get_field(static_cast<CowlAny*>(super), 0));
    CowlString* iri = cowl_to_string(cowl_class_get_iri(context->clazz));
    //std::cout << "\t\t" << static_cast<int>(cowl_cls_exp_get_type(super)) << std::endl;

    if (static_cast<int>(cowl_cls_exp_get_type(super)) != static_cast<int>(COWL_CET_CLASS)) {
        cowl_release(iri);
        cowl_release(string);
        return true;
    }
    if (!cowl_equals(iri, string)) {
        //std::cout << cowl_string_get_cstring(iri) << std::endl << cowl_string_get_cstring(string) << std::endl;
        cowl_release(iri);
        cowl_release(string);
        return true;
    }

    CowlClsExp* sub = cowl_sub_cls_axiom_get_sub(static_cast<CowlSubClsAxiom*>(axiom));

    if ((cowl_cls_exp_get_type(sub) != COWL_CET_OBJ_SOME) && (cowl_cls_exp_get_type(sub) != COWL_CET_OBJ_HAS_VALUE)) {
        //std::cout << "\t" << static_cast<int>(cowl_cls_exp_get_type(sub)) << std::endl;
        cowl_release(iri);
        cowl_release(string);
        return true;
    }

    int count = static_cast<int>(cowl_get_field_count(static_cast<CowlAny*>(sub)));
    std::cout << "\tField Count = " << count << std::endl;
    if (count <= 0) {
        cowl_release(iri);
        cowl_release(string);
        return true;
    }

    for (int i = 0; i < count; ++i) {
        std::cout << "\t" << static_cast<int>(cowl_get_type(cowl_get_field(static_cast<CowlAny*>(sub), i))) << std::endl;
    }

    cowl_release(string);

    return true;
}

// Iterator body, invoked for each class expression matching the query.
bool forEachClass(void* iterator, CowlAny* clazz) {
    CustomContext* context = static_cast<CustomContext*>(iterator);

    if (cowl_cls_exp_get_type(clazz) != COWL_CET_CLASS) {
        return true;
    }

    // Log the IRI remainder.
    cowl_write_static(context->stream, "\"");
    cowl_write_string(context->stream, cowl_iri_get_rem(cowl_class_get_iri(static_cast<CowlClass*>(clazz))));
    cowl_write_static(context->stream, "\" -> \"");
    cowl_write_string(context->stream, cowl_iri_get_rem(cowl_class_get_iri(static_cast<CowlClass*>(context->clazz))));
    cowl_write_static(context->stream, "\"\n");

    // Recurse.
    CustomContext subcontext = { context->ontology, context->stream, static_cast<CowlClass*>(clazz) };
    CowlIterator subiterator = { &subcontext, forEachClass };
    CowlIterator axioms = { &subcontext, forEachAxiom };
    cowl_ontology_iterate_axioms(context->ontology, &axioms, false);
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
    CowlClass* clazz = cowl_class_from_static("http://127.0.0.1:8080/ldj54#Thing");
    CustomContext context = { ontology, stream, clazz };
    CowlIterator iterator = { &context, forEachClass };
    cowl_ontology_iterate_sub_classes(ontology, clazz, &iterator, false);
    cowl_release_all(clazz, ontology);
    return 0;
}


#include "aiws/chunking_strategy.hpp"
#include "aiws/context_builder.hpp"
#include "aiws/context_strategy.hpp"
#include "aiws/processing_core.hpp"
#include "aiws/retrieval_engine.hpp"
#include "aiws/retrieval_strategy.hpp"
#include "aiws/chunker.hpp"
#include <iostream>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include  <vector>
#include <stdexcept>
#include <type_traits>
// Student-written M2 tests
//
// Add your own tests to this file. Your tests are part of the submitted work
// and are evaluated under Student-Written Testing & Validation.
//
// Do not modify tests/public_tests.cpp.
//
// Your tests should exercise important M2 behavior beyond the supplied public
// tests. Consider default compatibility, custom strategies, runtime dispatch,
// invalid configuration, ownership/move behavior, and component interactions.
namespace {
int failures = 0;
void check(bool ok, const char* name) {
    if (!ok) { std::cerr << "FAIL: " << name << '\n'; ++failures; }
}
}
class OneChunk final : public aiws::ChunkingStrategy {
    // custom chunker used to test the interaction with normal RetrievalEngine and ContextBuilder
public:
    std::vector<aiws::Chunk> chunk(const aiws::Document& d, std::size_t order) const override {
        return {{d.id()+"#marker", d.id(), order, 0, "alpha marker", 2, 0, d.text().size()}};
    }
};
//Testing that ProcessingCore is move only
static_assert(!std::is_copy_constructible_v<aiws::ProcessingCore>);
static_assert(!std::is_copy_assignable_v<aiws::ProcessingCore>);
static_assert(std::is_move_constructible_v<aiws::ProcessingCore>);
static_assert(std::is_move_assignable_v<aiws::ProcessingCore>);
int main() {
using namespace aiws;
bool threw = false;
// Testing Null context strategy 
//--
threw = false;
try {
    ProcessingCore bad_process(
        std::make_unique<Chunker>(),
        std::unique_ptr<RetrievalStrategy>{}, 
        std::make_unique<ContextBuilder>()
    );
}
catch(const std::invalid_argument&){
    threw = true;
}
catch(...) {}
check(threw, "the null context strategy was rejected");
//--
// Testing null context strategy
//--
threw = false;
try{
    ProcessingCore bad(std::make_unique<Chunker>(), std::make_unique<RetrievalEngine>(), std::unique_ptr<ContextStrategy>{});

}
catch(const std::invalid_argument&){
    threw = true;
}
catch(...){}
check(threw, "the null context strategy was rejected");
//--
// Testing that move construction preserves the usable state
//--
Workspace w1;
w1.add_document(Document{"move", "", "Alpha beta beta"});
ProcessingCore og;
og.rebuild(w1);
ProcessingCore moved(std::move(og));
auto move_results = moved.search("beta", 2);
check(move_results.size() == 1 && move_results[0].document_id == "move", "the move constructed ProcessingCore is still usable");
//--
//Testing that move assignment will preserve the usable state
//--
ProcessingCore assign;
assign = std::move(moved);
auto assign_results = assign.search("beta", 2);
check (assign_results.size() == 1 && assign_results[0].document_id == "move", "the move assigned ProcessingCore is still usable");
//--
//Testing that a failed rebuild will preserve the previous valid corpus
//--
Workspace w2;
w2.add_document(Document{"og", "", "Original search text"});
ProcessingCore rebuild_core;
rebuild_core.rebuild(w2);
const std::size_t old_count = rebuild_core.chunk_count();
const std::string old_id = rebuild_core.chunks()[0].id;
const std::string old_text = rebuild_core.chunks()[0].text;
Workspace bad_w3;
bad_w3.add_document(Document{"duplicate", "", "First document"});
bad_w3.add_document(Document{"duplicate", "", "Second document."});
threw = false;
try{ 
    rebuild_core.rebuild(bad_w3);
}
catch(const std::invalid_argument&){
    threw = true;
}
catch(...){}
check(threw, "the duplicate document ID's are rejected");
check(rebuild_core.chunk_count() == old_count, "The failed rebuild preserves the previous chunk count");
check(rebuild_core.chunks()[0].id == old_id, "the failed rebuild preserves the previous chunk ID");
check(rebuild_core.chunks()[0].text == old_text, "The failed rebuild preserves the previous chunk text");
//--
// Testing that the custom chunker made integrates with the default retrieval/context
//--
ProcessingCore integration_core(std::make_unique<OneChunk>(), 
std::make_unique<RetrievalEngine>(), std::make_unique<ContextBuilder>());
Workspace integration_w4;
integration_w4.add_document(Document{"document", "", "Text ignored by OneChunk."});
integration_core.rebuild(integration_w4);
check(integration_core.chunk_count() == 1, "the custom chunk output is stored in corpus");
check(integration_core.chunks()[0].text == "alpha marker", "the custom chunk text is stored in the corpus");
auto marker_results = integration_core.search("marker", 1);
check(marker_results.size() == 1 && marker_results[0].document_id == "document","the default retrieval searches the custom chunk output");
auto marker_context = integration_core.build_context("marker", 1, 10);
check(marker_context.size() == 1 && marker_context[0].text == "alpha marker", "the default context builder consumes the custom retrieval output");
//--
// Final
//--
if(failures != 0){
    return 1;
}
std::cout << "All M2 student tests passed\n";
} //end of main

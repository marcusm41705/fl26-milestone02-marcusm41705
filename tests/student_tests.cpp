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
//--
//Null retrieval strategy
//--
bool threw = false;
try{ 
    ProcessingCore bad_process(std::make_unique<Chunker>(), std::unique_ptr<RetrievalStrategy>{},std::make_unique<ContextBuilder>());
}
catch ( const std::invalid_argument&){
    threw = true;
}
catch(...) {} ///used to catch additional exception types that can't be caught by other catch blocks
check(threw, "null retrieval strategy was rejected");

//--
// Testing Null retrieval 
//--
threw = false;
try {
    ProcessingCore bad_process(std::make_unique<Chunker>(), std::make_unique<RetrievalEngine>(), std::unique_ptr<ContextStrategy>{});
}
catch(const std::invalid_argument&){
    threw = true;
}
catch(...) {}
check(threw, "the null retrieval strategy was rejected");
//--
// Testing null context strategy
//--
threw = false;
try{
    ProcessingCore bad(std::make_unique<Chunker>(), std::make_unique<RetrievalEngine>(), std::unique_ptr<ContextStrategy>{});

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
}

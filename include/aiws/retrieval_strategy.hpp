#pragma once

#include "aiws/corpus_index.hpp"
#include "aiws/processing_types.hpp"

#include <string>
#include <vector>

namespace aiws {

// M2 PUBLIC-INTERFACE DESIGN TASK
// Complete this class as a safe abstract polymorphic interface.
// Keep the class name, operation name, parameter types, return type,
// const qualification, and namespace unchanged.
class RetrievalStrategy {
    //abstract interface used for ranked retrieval strategies
public:
    // Is now destruction safe through a base-class pointer.
     virtual ~RetrievalStrategy() = default;

    // Is now a required polymorphic operation.
    virtual std::vector<SearchResult> search(const std::string&,
                                             int,
                                             const std::vector<Chunk>&,
                                             const CorpusIndex&) const = 0;
       
    
};

}  // namespace aiws

# M2 Technical & Design Understanding

This file is an assessed technical-understanding artifact, not ordinary project documentation.
Answer all four questions using your own submitted implementation. Concise answers are acceptable when they are technically correct and specific.

Generic descriptions of C++ concepts or restatements of the assignment that do not identify and explain corresponding parts of your code will receive limited credit.

## 1. Polymorphism and dynamic dispatch - 1.5 points

ProcessingCore::rebuild() uses runtime polymorphism, as the base interface is ChunkingStrategy, and the default derived implementation is Chunker. ProcessingCore::Impl stores the active chunking strategy. During rebuild(), the strategy is called with the auto produced. Chunk is declared as virtual in ChunkingStrategy, therefore the function that runs depends on the object stored in the unique_ptr. With default ProcessingCore constructor, the object is a Chunker, and Chunker::chunk() gets invoked. If a caller instead provides the derived class, such as the student_tests OneChunk strategy, then OneChunk::chunk() gets invoked through the same ChunkingStrategy pointer. This allows the behavior to be selected at runtime without having to change ProcessingCore::rebuild(). This is representative of a requirement for Milestone 2, that the configured chunking strategy has to be used during rebuilding than rather ProcessingCore continuing to call a hard-coded Chunker.

If chunk() aere not declared as virtual, a call through a ChunkingStrategy pointer would not dispatch to the derived implementation. The operation that's associated with the base type would be selected instead, therefore, supplying the custom derived strategy does not change the behavior of ProcessingCore. This defeats the purpose of the strategy abstraction. 

## 2. Ownership and lifetime - 1.5 points


In the implementation, the strategy objects are created by either the default ProcessingCore constructor or by the caller using a configurable constructor. For example, the default constructor creates a Chunker, RetrievalEngine, and ContextBuilder using std::make_unique. These objects then get passed to the configurable constructor, and ownership is transferred using std::move. The objects are ultimately owned by ProcessingCore::Impl, which stores them as std::unique_ptr<ChunkingStrategy> chunking, std::unique_ptr<RetrievalStrategy> retrieval, and std::unique_ptr<ContextStrategy> context. Using std::unique_ptr makes the ownership relationship explicit because each strategy has one owner. The pointers cannot be copied and can be moved, which allows ownership to be transferred safely. When a ProcessingCore object gets destroyed, the impl_ object is also destroyed and automatically destroys the three strategy objects through RAII. This avoids manual memory management using new and delete functions, which is required by the Milestone 2 specification. 

ProcessingCore is move-only because it owns the resources using std::unique_ptr. Copying a ProcessingCore would require copying or sharing the owned strategy objects which conflicts with exclusive ownership design. Move construction and move assignment are both supported because the ownership of the unique_ptr members are able to be safely transferred to another ProcessingCore. 

The strategy base classes require virtual destructors that ensure that when the object is destroyed through the base-class pointer, the destructor for the complete derived object can be called correctly. The Milestone 2 specification requires that each strategy interface is safely destructible using a base-class pointer. 

## 3. Architecture, extensibility, and M1 compatibility - 1.5 points


An architectural decision made in Milestone 2 was to have ProcessingCore depend on abstract strategy interfaces instead of alternatively directly storing the concrete processing classes. In ProcessingCore::Impl, the chunking, retrieval, and context components are stored as std::unique_ptr<ChunkingStrategy>, std::unique_ptr<RetrievalStrategy> and std::unique_ptr<ContextStrategy>. The concrete classes Chunker, RetrievalEngine, and ContextBuilder are used to implement these interfaces. The default ProcessingCore constructor preserves behavior from Milestone 1 by creating the same Milestone 1 compatible concrete implementations. The default Chunker still uses the same maxiumum, 20-token overlap, and paragraph preference behaviors. RetrievalEngine and ContextBuilder preserves the existing Milestone 1 retrieval and context-building behavior. Code used to create ProcessingCore core; will continue to behave like the system from Milestone 1. 

A plausible design alternative would be to keep Chunker, ContextBuilder, and RetrievalEngine as concrete members and use if statements, enums, or switch statements inside of ProcessingCore in order to select between different implementations. This approach requires modifying ProcessingCore when any new processing algorithm is added. The strategy-based design is preferable for Milestone 2 because new implementations are able to be substituted through existing interfaces without having to modify the processing logic.

## 4. Testing and defect reasoning - 1.5 points

One meaningful test in the student_tests.cpp uses the custom OneChunk implementation of ChunkingStrategy paired with the normal RetrievalEngine and ContextBuilder. The custom strategy returns a single predictable chunk:
 return {{d.id()+"#marker", d.id(), order, 0, "alpha marker", 2, 0, d.text().size()}};
 Then constructs ProcessingCore integration_core. After calling rebuild(), the test then verifies that the stored chunk contains "alpha marker". It then searches for "marker" using the normal RetrievalEngine, then verifies that the expected document has been returned. Finally, it calls build_context() then verifies that the normal ContextBuilder recieves the custom chunk text. 

 This test validates the runtime substitution and integration between multiple processing responsibilities. The M2 specification requires testing in order to exercise scenarios and custom strategies that cross multiple processing components. 

 A defect the test could detect is a ProcessingCore::rebuild() implementation that ignores the ChunkingStrategy and instead directly creates or calls a normal Chunker. If that defect exists, the "alpha marker" text would never be produced by OneChunk, so the chunk text check would fail and the subsequent search for "marker" would also fail. 

 The test provides evidence beyond simply rerunning the supplied public tests because it combines the custom chunking strategy with the default retrieval and context strategy with default retrieval strategy. This verifies that the output produced by a substitution component that becomes part of the normal corpus and can flow through the existing M1 compatible retrieval and context stages. It therefore tests that a virtual function was called, and additionally tests that the substituted component integrates successfully with the rest of the processing pipeline. 

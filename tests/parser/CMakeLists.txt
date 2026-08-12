target_sources(juno-pl-tests PRIVATE
  BodyHandler-tests.cpp
  PerformInstructionHandler-tests.cpp
  ParserIntegration-tests.cpp
  Program-tests.cpp
  Function-tests.cpp
  Event-tests.cpp
  VarAssignment-tests.cpp
  ListOperation-tests.cpp
  Conditional-tests.cpp
  Loop-tests.cpp
  Expression-tests.cpp
  ErrorRecovery-tests.cpp)
target_sources(juno-pl-tests PRIVATE Conditional-debug.cpp)

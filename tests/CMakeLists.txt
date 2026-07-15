add_executable(adapter_policy_test
  adapter_policy_test.cpp
  ${PROJECT_SOURCE_DIR}/shared/src/utils.cpp)

target_include_directories(adapter_policy_test PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(adapter_policy_test PRIVATE ${CMAKE_DL_LIBS})

add_test(NAME adapter_policy_test COMMAND adapter_policy_test)

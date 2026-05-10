include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# Allow MSVC to share runtime
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Unit tests
file(GLOB UNIT_TEST_SOURCES unit/*.cpp)
add_executable(unit_tests ${UNIT_TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE chronos_lib gtest_main)
add_test(NAME unit_tests COMMAND unit_tests)

# Integration tests
file(GLOB INTEGRATION_TEST_SOURCES integration/*.cpp)
add_executable(integration_tests ${INTEGRATION_TEST_SOURCES})
target_link_libraries(integration_tests PRIVATE chronos_lib gtest_main)
add_test(NAME integration_tests COMMAND integration_tests)

cmake_minimum_required(VERSION 3.16)
project(LidCavityCPPVerification LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LIDCAVITY_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

add_library(lidcavity_verification
    src/verification/operators.cpp
    src/verification/poisson.cpp
    src/verification/convergence.cpp
)

target_include_directories(lidcavity_verification PUBLIC include)
target_compile_options(lidcavity_verification PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wshadow
)

if(LIDCAVITY_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(lidcavity_verification PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(lidcavity_verification PRIVATE -fsanitize=address,undefined)
endif()

enable_testing()

function(add_lidcavity_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE lidcavity_verification)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(LIDCAVITY_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
    add_test(NAME ${target} COMMAND ${target})
endfunction()

add_lidcavity_test(test_operators tests/test_operators.cpp)
add_lidcavity_test(test_poisson tests/test_poisson.cpp)
add_lidcavity_test(test_convergence tests/test_convergence.cpp)

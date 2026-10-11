cmake_minimum_required(VERSION 3.10)

project(alp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(alp
    src/main.cpp
    src/ALP.cpp
)
target_include_directories(alp PRIVATE include)

# FP contraction (FMA) breaks the exact round-trip guarantee of
# decoded == d * F10[factor] * i_F10[exponent]; it must stay off.
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(alp PRIVATE -ffp-contract=off -Wall -Wextra)
endif()
cmake_minimum_required(VERSION 3.4)

project(MinaFuzzer LANGUAGES C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-g -ggdb2 -O3)

set(APP_DIR "../src")

# Build with code coverage generation
if(CODE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
        add_compile_options(-fprofile-arcs -ftest-coverage)
        link_libraries(gcov)
    else()
        message(FATAL_ERROR "Unsupported compiler used with code coverage generation")
    endif()
endif()

add_library(mina
    ${APP_DIR}/parse_tx.c
    ${APP_DIR}/utils.c
    ${APP_DIR}/random_oracle_input.c
    ${APP_DIR}/transaction.c
)
target_include_directories(mina PUBLIC ${APP_DIR})

add_executable(fuzz_message fuzz_message.c)

target_link_libraries(fuzz_message PUBLIC mina -fsanitize=fuzzer,address,undefined)
target_compile_options(fuzz_message PUBLIC -fsanitize=fuzzer,address,undefined)

cmake_minimum_required(VERSION 3.16)

project(crypto-primitives LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(MSVC)
    add_compile_options(/W4)
endif()

enable_testing()

# 암호 알고리즘 추가: add_cipher(<name>)
#   src/<name>.c                          -> 라이브러리 <name>
#   tests/<name>-test.c                   -> 테스트 실행 파일 <name>-test (CTest 등록)
#   include/crypto-primitives/<name>.h    -> 공개 헤더
function(add_cipher name)
    add_library(${name} src/${name}.c)
    target_include_directories(${name} PUBLIC ${PROJECT_SOURCE_DIR}/include)

    add_executable(${name}-test tests/${name}-test.c)
    target_link_libraries(${name}-test PRIVATE ${name})
    add_test(NAME ${name}-test COMMAND ${name}-test)
endfunction()

add_cipher(template-cipher)
add_cipher(aes)
add_cipher(aes-lut1)
add_cipher(aes-lut2)
add_cipher(aes-lut3)

add_executable(aes-kat-test tests/aes-kat-test.c)
target_include_directories(aes-kat-test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(aes-kat-test PRIVATE aes aes-lut1 aes-lut2 aes-lut3)
add_test(NAME aes-kat-test COMMAND aes-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/aes)

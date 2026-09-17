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

# 해시 함수 추가: add_message_digest(<name>) — add_cipher와 같은 파일 규칙을 따름
function(add_message_digest name)
    add_cipher(${name})
endfunction()

add_cipher(template-cipher)
add_cipher(aes)
add_cipher(aes-lut1)
add_cipher(aes-lut2)
add_cipher(aes-lut3)
add_cipher(lea)
add_cipher(lea-unrolled)
add_cipher(cham)
add_cipher(hight)
add_cipher(hight-lut)
add_cipher(aria)
add_cipher(aria-lut)
add_cipher(seed)

add_message_digest(lsh256)
add_message_digest(lsh512)

add_library(kat-common tests/kat-common.c)
target_include_directories(kat-common PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/tests)

# KAT 테스트 추가: add_kat_test(<name> <vectors-subdir> <lib>...)
#   tests/<name>-kat-test.c  -> 테스트 실행 파일 <name>-kat-test (CTest 등록)
#   tests/vectors/<vectors-subdir> 를 인자로 넘김
function(add_kat_test name vectors_dir)
    add_executable(${name}-kat-test tests/${name}-kat-test.c)
    target_link_libraries(${name}-kat-test PRIVATE kat-common ${ARGN})
    add_test(NAME ${name}-kat-test COMMAND ${name}-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/${vectors_dir})
endfunction()

add_kat_test(aes aes aes aes-lut1 aes-lut2 aes-lut3)
add_kat_test(lea lea lea lea-unrolled)
add_kat_test(cham cham cham)
add_kat_test(hight hight hight hight-lut)
add_kat_test(aria aria aria aria-lut)
add_kat_test(seed seed seed)
add_kat_test(lsh lsh lsh256 lsh512)

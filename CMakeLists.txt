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

add_executable(aes-kat-test tests/aes-kat-test.c)
target_include_directories(aes-kat-test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(aes-kat-test PRIVATE aes aes-lut1 aes-lut2 aes-lut3)
add_test(NAME aes-kat-test COMMAND aes-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/aes)

add_executable(lea-kat-test tests/lea-kat-test.c)
target_include_directories(lea-kat-test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(lea-kat-test PRIVATE lea lea-unrolled)
add_test(NAME lea-kat-test COMMAND lea-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/lea)

add_executable(cham-kat-test tests/cham-kat-test.c)
target_link_libraries(cham-kat-test PRIVATE cham)
add_test(NAME cham-kat-test COMMAND cham-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/cham)

add_executable(hight-kat-test tests/hight-kat-test.c)
target_link_libraries(hight-kat-test PRIVATE hight hight-lut)
add_test(NAME hight-kat-test COMMAND hight-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/hight)

add_executable(aria-kat-test tests/aria-kat-test.c)
target_link_libraries(aria-kat-test PRIVATE aria aria-lut)
add_test(NAME aria-kat-test COMMAND aria-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/aria)

add_executable(seed-kat-test tests/seed-kat-test.c)
target_link_libraries(seed-kat-test PRIVATE seed)
add_test(NAME seed-kat-test COMMAND seed-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/seed)

add_executable(lsh-kat-test tests/lsh-kat-test.c)
target_link_libraries(lsh-kat-test PRIVATE lsh256 lsh512)
add_test(NAME lsh-kat-test COMMAND lsh-kat-test ${PROJECT_SOURCE_DIR}/tests/vectors/lsh)

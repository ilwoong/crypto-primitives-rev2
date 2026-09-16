# crypto-primitives

C로 블록 암호 알고리즘을 구현하기 위한 템플릿 프로젝트입니다.

모든 블록 암호는 `include/crypto-primitives/cipher.h`의 `BlockCipher` 인터페이스(`expand_key`, `encrypt`, `decrypt`)를 구현합니다. `template-cipher`는 이 인터페이스를 보여주는 예제로, 키와 블록을 XOR만 하는 동작을 합니다. 실제 암호로 쓰면 안 됩니다.

## 디렉터리 구조

```
.
├── CMakeLists.txt
├── include/crypto-primitives/
│   ├── cipher.h              # BlockCipher 공통 인터페이스
│   └── template-cipher.h     # 템플릿 암호 헤더
├── src/
│   └── template-cipher.c     # 템플릿 암호 구현
└── tests/
    └── template-cipher-test.c
```

## 요구 사항

- CMake 3.16 이상
- C11을 지원하는 C 컴파일러 (GCC, Clang, MSVC)

## 빌드

```sh
cmake -S . -B build
cmake --build build
```

빌드 타입을 지정하지 않으면 `Debug`로 빌드됩니다. 릴리스 빌드는 다음과 같이 합니다.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## 테스트

```sh
ctest --test-dir build --output-on-failure
```

테스트 실행 파일을 직접 실행할 수도 있습니다.

```sh
./build/template-cipher-test
```

## 새 알고리즘 추가하기

`<name>`이 알고리즘 이름일 때:

1. `include/crypto-primitives/<name>.h`에 컨텍스트 구조체와 함수, `extern const BlockCipher` 선언을 추가합니다.
2. `src/<name>.c`에 구현을 작성합니다.
3. `tests/<name>-test.c`에 테스트 벡터 기반 테스트를 작성합니다.
4. `CMakeLists.txt`에 한 줄을 추가합니다.

   ```cmake
   add_cipher(<name>)
   ```

그러면 라이브러리 `<name>`, 테스트 실행 파일 `<name>-test`가 만들어지고, 테스트가 CTest에 등록됩니다.

## 코드 스타일

`.clang-format`에 정의된 스타일(LLVM 기반, 들여쓰기 4칸, 한 줄 최대 120자)을 따릅니다.

```sh
clang-format -i src/*.c include/crypto-primitives/*.h tests/*.c
```

# crypto-primitives

C11로 작성한 블록 암호와 해시 함수 구현 모음입니다. 블록 암호는 `include/crypto-primitives/cipher.h`의 `block_cipher` 인터페이스(`expand_key`, `encrypt`, `decrypt`)를, 해시 함수는 `include/crypto-primitives/message-digest.h`의 `message_digest` 인터페이스(`init`, `update`, `final`)를 구현합니다. 알고리즘마다 소스 파일 하나로 되어 있어 외부 의존성 없이 빌드됩니다.

`template-cipher`는 이 인터페이스를 보여주는 예제로, 키와 블록을 XOR만 하는 동작을 합니다. 실제 암호로 쓰면 안 됩니다.

## 구현된 알고리즘

| 알고리즘 | 라이브러리 | 컨텍스트 | `block_cipher` 인스턴스 | 설명 |
|---|---|---|---|---|
| AES | `aes` | `aes_ctx` | `aes128_block_cipher`, `aes192_block_cipher`, `aes256_block_cipher` | 레퍼런스 구현. GF(2^8) 연산을 실행 시점에 계산 |
| | `aes-lut1` | `aes_lut1_ctx` | `aes128_lut1_block_cipher`, `aes192_lut1_block_cipher`, `aes256_lut1_block_cipher` | S-box와 GF 곱셈 테이블 사용 |
| | `aes-lut2` | `aes_lut2_ctx` | `aes128_lut2_block_cipher`, `aes192_lut2_block_cipher`, `aes256_lut2_block_cipher` | MixColumns 결합 테이블 |
| | `aes-lut3` | `aes_lut3_ctx` | `aes128_lut3_block_cipher`, `aes192_lut3_block_cipher`, `aes256_lut3_block_cipher` | SubBytes+MixColumns 결합 테이블 |
| LEA | `lea` | `lea_ctx` | `lea128_block_cipher`, `lea192_block_cipher`, `lea256_block_cipher` | 레퍼런스 구현 |
| | `lea-unrolled` | `lea_unrolled_ctx` | `lea128_unrolled_block_cipher`, `lea192_unrolled_block_cipher`, `lea256_unrolled_block_cipher` | 4라운드 언롤 |
| CHAM | `cham` | `cham_ctx` | `cham64_128_block_cipher`, `cham128_128_block_cipher`, `cham128_256_block_cipher` | 블록 64/128비트 |
| HIGHT | `hight` | `hight_ctx` | `hight_block_cipher` | 레퍼런스 구현 (블록 64비트) |
| | `hight-lut` | `hight_lut_ctx` | `hight_lut_block_cipher` | 룩업 테이블 사용 |
| ARIA | `aria` | `aria_ctx` | `aria128_block_cipher`, `aria192_block_cipher`, `aria256_block_cipher` | 레퍼런스 구현 |
| | `aria-lut` | `aria_lut_ctx` | `aria128_lut_block_cipher`, `aria192_lut_block_cipher`, `aria256_lut_block_cipher` | 룩업 테이블 사용 |
| SEED | `seed` | `seed_ctx` | `seed_block_cipher` | RFC 4269 |

`expand_key`는 키 길이를 인자로 받지 않으므로, 키 길이가 여러 가지인 알고리즘은 키 길이마다 별도의 `block_cipher` 인스턴스를 제공합니다. 같은 알고리즘의 인스턴스들은 컨텍스트 구조체와 `encrypt`/`decrypt` 함수를 공유합니다.

같은 알고리즘의 변형(`aes`, `aes-lut1` 등)은 모두 동일한 출력을 내며 속도와 코드 크기만 다릅니다.

## 구현된 해시 함수

| 알고리즘 | 라이브러리 | 컨텍스트 | `message_digest` 인스턴스 | 다이제스트 / 블록 |
|---|---|---|---|---|
| LSH-256-256 | `lsh256` | `lsh256_ctx` | `lsh256_message_digest` | 32 / 128 바이트 |
| LSH-512-512 | `lsh512` | `lsh512_ctx` | `lsh512_message_digest` | 64 / 256 바이트 |

`message_digest`는 함수 포인터 외에 `digest_size`와 `block_size`를 담고 있어, 알고리즘을 모르는 코드(테스트, HMAC 등)도 버퍼 크기를 알 수 있습니다. `final`을 호출하면 컨텍스트가 초기화되므로 다시 쓰려면 `init`부터 시작합니다.

## 사용 예

```c
#include "crypto-primitives/aes.h"

aes_ctx ctx;
uint8_t out[16];

aes128_block_cipher.expand_key(&ctx, key);       // key: 16바이트
aes128_block_cipher.encrypt(&ctx, out, in);      // in, out: 16바이트 블록
aes128_block_cipher.decrypt(&ctx, out, out);     // in-place 호출 가능
```

컨텍스트는 호출자가 할당하며, 블록 크기는 알고리즘이 정합니다(CHAM-64, HIGHT는 8바이트, 나머지는 16바이트).

해시 함수도 같은 방식입니다. `update`는 임의 길이로 여러 번 나눠 호출할 수 있습니다.

```c
#include "crypto-primitives/lsh256.h"

lsh256_ctx ctx;
uint8_t out[32];                                  // lsh256_message_digest.digest_size

lsh256_message_digest.init(&ctx);
lsh256_message_digest.update(&ctx, data, len);
lsh256_message_digest.final(&ctx, out);
```

모든 테스트는 little-endian 호스트(x86-64)에서 검증했습니다. AES, LEA, CHAM 구현은 입력 바이트를 호스트 바이트 순서의 워드로 읽으므로 big-endian 환경에서는 검증되지 않았습니다. HIGHT는 바이트 단위로만 동작하고, ARIA, SEED, LSH는 바이트 순서를 명시적으로 처리하므로 호스트와 무관합니다.

## 디렉터리 구조

```
.
├── CMakeLists.txt
├── include/crypto-primitives/
│   ├── cipher.h              # block_cipher 공통 인터페이스
│   ├── message-digest.h      # message_digest 공통 인터페이스
│   ├── template-cipher.h     # 템플릿 암호 헤더
│   └── <name>.h              # 알고리즘별 헤더
├── src/
│   ├── template-cipher.c
│   └── <name>.c              # 알고리즘별 구현 (파일 하나)
└── tests/
    ├── <name>-test.c         # 스펙 테스트 벡터 기반 단위 테스트
    ├── <algo>-kat-test.c     # Known Answer Test
    └── vectors/<algo>/       # KAT 벡터 (.rsp)
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

테스트는 두 종류입니다.

- `<name>-test`: 알고리즘 변형마다 하나씩 있으며, 표준 문서의 테스트 벡터로 암호화, 복호화, in-place 동작을 확인합니다.
- `<algo>-kat-test`: `tests/vectors/<algo>/`의 `.rsp` 파일을 읽어 해당 알고리즘의 모든 변형을 검증합니다. 블록 암호 파일은 NIST CAVS 형식(`ECBVarKey*`, `ECBVarTxt*`)을 따르며 `[ENCRYPT]`와 `[DECRYPT]` 섹션이 있습니다. 해시 파일(`*ShortMsg.rsp`)은 NIST SHA 벡터 형식(`Len`, `Msg`, `MD`)이며, 0~512바이트의 모든 길이를 담고 있어 블록 경계 처리를 확인합니다. 해시 KAT는 메시지를 한 번에 넣는 경우와 잘게 나눠 넣는 경우를 모두 검사합니다.

테스트 실행 파일을 직접 실행할 수도 있습니다. KAT 테스트는 벡터 디렉터리를 인자로 받습니다.

```sh
./build/aes-test
./build/aes-kat-test tests/vectors/aes
```

### 테스트 벡터 출처

| 알고리즘 | 단위 테스트 벡터 | KAT 벡터 |
|---|---|---|
| AES | NIST FIPS-197, SP 800-38A | NIST CAVS `ECBVarKey`/`ECBVarTxt` 원본 파일 |
| LEA | 원 저장소 [crypto-primitives](https://github.com/ilwoong/crypto-primitives) | 원 저장소 구현으로 생성 |
| CHAM | 원 저장소 | 원 저장소 구현으로 생성 |
| HIGHT | KISA 테스트 벡터 | 원 저장소 구현으로 생성 |
| ARIA | 원 저장소, RFC 5794 Appendix A.1 | 원 저장소 구현으로 생성 |
| SEED | RFC 4269 Appendix B | 원 저장소 구현에 RFC 4269 키 스케줄을 적용해 생성 |
| LSH | Crypto++ `TestVectors/lsh256.txt`, `lsh512.txt` | 원 저장소 구현으로 생성 |

AES 외의 KAT 벡터는 원 저장소의 구현을 직접 링크해 생성했으며, 생성 전에 그 구현이 표준 문서의 벡터를 통과하는지 확인했습니다.

SEED는 원 저장소 구현의 키 스케줄이 RFC 4269와 달라(키가 전부 0일 때만 일치) 이 저장소에서는 RFC대로 구현했습니다. 따라서 원 저장소의 SEED와는 호환되지 않습니다.

## 새 알고리즘 추가하기

`<name>`이 알고리즘 이름일 때:

1. `include/crypto-primitives/<name>.h`에 컨텍스트 구조체와 함수, `extern const block_cipher` 선언을 추가합니다.
2. `src/<name>.c`에 구현을 작성합니다. 테이블 등 필요한 것은 모두 이 파일 안에 `static`으로 둡니다.
3. `tests/<name>-test.c`에 테스트 벡터 기반 테스트를 작성합니다.
4. `CMakeLists.txt`에 한 줄을 추가합니다.

   ```cmake
   add_cipher(<name>)
   ```

그러면 라이브러리 `<name>`, 테스트 실행 파일 `<name>-test`가 만들어지고, 테스트가 CTest에 등록됩니다.

해시 함수는 `message-digest.h`의 `message_digest`를 구현하고 `add_message_digest(<name>)`으로 등록합니다. 파일 규칙은 같습니다.

KAT 테스트를 추가하려면 `tests/vectors/<algo>/`에 `.rsp` 파일을 넣고 `tests/<algo>-kat-test.c`를 작성한 뒤, 기존 KAT 테스트와 같은 방식으로 `CMakeLists.txt`에 실행 파일과 테스트를 등록합니다.

## 코드 스타일

- 식별자는 모두 snake_case를 사용합니다 (구조체 타입 포함).
- 포맷은 `.clang-format`에 정의된 스타일(LLVM 기반, 들여쓰기 4칸, 한 줄 최대 120자)을 따릅니다.

```sh
clang-format -i src/*.c include/crypto-primitives/*.h tests/*.c
```

## 라이선스

MIT License. `LICENSE` 파일을 참고하세요.

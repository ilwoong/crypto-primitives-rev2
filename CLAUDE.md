# crypto-primitives-rev2

C11 블록 암호/해시 함수 모음. 레이아웃, 빌드, 테스트, 코드 규칙은 `README.md`를 따른다. 이 파일은 README에 없는 작업 규칙만 담는다.

## 명령

```sh
cmake -S . -B build && cmake --build build          # 빌드 (-Wall -Wextra -Wpedantic)
ctest --test-dir build --output-on-failure           # 전체 테스트 (CTest)
cmake -S . -B build/sanitize -DCMAKE_C_FLAGS="-fsanitize=undefined,address -fno-sanitize-recover=all" \
  && cmake --build build/sanitize && ctest --test-dir build/sanitize --output-on-failure   # UBSan+ASan
clang-format --dry-run --Werror src/*.c include/crypto-primitives/*.h tests/*.c tests/*.h  # 포맷 검사
```

`build/`는 gitignore 대상이므로 sanitizer 빌드도 `build/sanitize`처럼 그 안에 둔다.

## 변경 범위를 정하는 방법

모든 에이전트는 "이번 파이프라인에서 바뀐 파일"을 다음으로 정한다. `git diff`만 보면 새로 만든 파일이 빠지므로 반드시 둘 다 본다.

```sh
git status --porcelain          # 수정(M)과 신규(??) 파일 모두
git diff HEAD                   # 추적 중인 파일의 변경 내용
```

신규(`??`) 파일은 전체를 읽는다. `build/`는 무시한다.

## 에이전트 파이프라인

`.claude/agents/`에 정의된 에이전트를 아래 순서로 호출한다. 각 에이전트에는 사용자 요청 원문과 직전 단계의 보고를 그대로 넘긴다.

1. **coder** — 요청된 변경 구현
2. **formatter** — 바뀐 파일에 clang-format 적용
3. **reviewer** — 검토. findings가 있으면 **coder(최소 수정) → formatter → reviewer** 반복. 최대 3회 검토 후에도 findings가 남으면 멈추고 사용자에게 보고한다.
4. **tester** — 빌드, 전체 CTest, sanitizer, 필요한 테스트 추가. 실패가 있으면 **fixer → tester** 반복. 최대 2회 후에도 실패하면 멈추고 사용자에게 보고한다.
5. fixer나 tester가 `src/` 또는 `include/`를 바꿨다면 **formatter → reviewer**를 한 번 더 거친다. `tests/`만 바뀌었다면 **formatter**만 거친다.
6. **documenter** — README 갱신

reviewer의 보고는 최종 텍스트로 온다. "findings: 없음"이 다음 단계로 넘어가는 신호다.

fork나 병렬 에이전트가 "스펙 벡터 통과"라고 보고하면 그것이 원본 자체 테스트 통과를 뜻할 수 있다. 외부 공식 벡터로 다시 확인한다 (HIGHT, ARIA 때 그렇게 했다).

## 코드 규칙

README의 "코드 스타일"에 더해 다음을 지킨다.

- 식별자 전부 snake_case (구조체 타입 포함). `BlockCipher` 같은 PascalCase 금지.
- 인스턴스 이름: `<algo><keybits>_<variant>_block_cipher` (예: `aes128_lut1_block_cipher`). 키 길이가 하나면 `<algo>_block_cipher`. CHAM은 블록/키 둘 다 표기 (`cham64_128_block_cipher`).
- `expand_key`가 키 길이를 받지 않으므로 키 길이마다 `block_cipher` 인스턴스를 따로 둔다. 컨텍스트와 encrypt/decrypt/clear는 공유.
- 블록 암호는 `<algo>_clear(void *ctx)`로 컨텍스트 전체를 `secure_zero`(`cipher.h`)로 지우고 인스턴스의 `.clear`에 연결한다. 단위 테스트와 KAT가 NULL이면 실패시킨다.
- 마스터 키 입력은 항상 `const uint8_t *`다. 라운드 키 타입은 알고리즘마다 통일할 필요 없이 구현에 맞는 타입(`uint8_t`, `uint16_t`, `uint32_t`, 2차원 배열 등)을 쓴다. 단, `uint8_t` 버퍼를 선언해 놓고 넓은 타입 포인터로 캐스트해 쓰지는 않는다. 워드로 쓸 거면 워드 배열로 선언한다.
- 회전 함수 인자는 `unsigned rot`, for 루프는 `++i`.
- 테이블은 원본에서 스크립트로 추출해 각 `src/<name>.c`에 `static const`로 인라인. 손으로 옮기지 않는다.
- 주석은 "왜"가 비자명할 때만. 파일 상단 라이선스 헤더 없음.
- 커밋 메시지는 영어로, 원본과 달라진 점과 검증 방법을 적는다.

## 알고리즘 포팅 절차

원 저장소(`../crypto-primitives`, 없으면 GitHub `ilwoong/crypto-primitives`)에서 알고리즘을 옮길 때는 순서대로 따른다. 원본 자체 테스트는 벡터가 하나뿐이거나 기대값 없이 출력만 찍는 경우가 있어 신뢰하지 않는다.

1. **원본을 저장소 밖의 공식 벡터로 먼저 검증한다** (NIST, RFC, KISA, 스펙 문서 부록). 실패하면 멈추고 사용자에게 보고한다. 조용히 고치지 않는다. (SEED가 이 경우였다: 원본 키 스케줄이 RFC 4269와 달라 rev2는 RFC대로 구현했다.)
2. 포팅 후 `tests/<name>-test.c`로 encrypt, decrypt, in-place를 확인한다. 해시는 한 번에 넣기와 바이트 단위 update를 모두 확인한다.
3. 원본과 대조한다: 무작위 입력 10만 건(해시는 무작위 길이와 분할). 원본 심볼은 `-D`로 이름을 바꿔 같이 링크한다.
4. KAT 벡터는 **원본 소스를 직접 링크하는 생성기**로 만든다. 알고리즘을 다시 쓰지 않는다(포팅본을 자기 출력으로 검증하는 셈이 된다). `nm`으로 원본 심볼만 링크됐는지 확인하고, 생성 전에 스펙 벡터를 통과하는지 확인한다. 생성기는 scratchpad에 두고 커밋하지 않는다. 자주 재생성하게 되면 `tools/`에 넣는다.
5. 벡터 한 비트를 바꾼 복사본으로 KAT 테스트가 실제로 실패하는지 확인한다.
6. UBSan+ASan. 원본의 회전량 초과(LEA), `value >> 32`(LSH)는 x86에서 우연히 맞는 값이 나와 sanitizer로만 잡혔다.
7. `clang-format -i`, 커밋. 커밋 메시지(영어)에 원본과 달라진 점과 검증 방법을 적는다.

## 알려진 제약

- big-endian 미검증: AES, LEA, CHAM은 워드를 호스트 바이트 순서로 읽는다.
- SIMD 변형(`aes.ni.c`, `lea.avx2.c`, `lsh*.sse4.c`, `lsh*.avx2.c`)은 x86 전용이라 의도적으로 제외했다. 추가한다면 CMake에서 컴파일러 플래그를 감지해 선택적으로 빌드한다.
- 원본의 `tools/hex.c`, `*/print_tables.c`, `seed_tool.c`는 유틸리티/테이블 생성기라 포팅하지 않는다.
- 운용 모드(ECB, CTR)는 미포팅. `block_cipher`의 `block_size`, `key_size`로 크기는 알 수 있으나, 모드 인터페이스(컨텍스트, IV/카운터 소유권, 파일 배치)는 설계 결정이 먼저 필요하다. 사용자에게 제안한 뒤 진행한다.

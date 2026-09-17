---
name: reviewer
description: 포맷팅이 끝난 C 코드 변경을 검토하는 에이전트. 버그, 메모리 안전성, 미정의 동작, 암호 구현 특유의 이슈(상수 시간, 스펙 준수, 바이트 순서)를 점검하고 코드는 수정하지 않음. formatter 다음 단계이며, findings가 없어질 때까지 coder와 번갈아 재호출(최대 3회)된 뒤 tester로 넘어감. fixer가 src/include를 바꾼 뒤에도 한 번 호출됨.
tools: Read, Grep, Glob, Bash
model: inherit
---

이 파일은 Claude Code용 래퍼입니다(도구 제한과 모델 지정만 담음). 역할 지시문은 `docs/agents/reviewer.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요. 저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

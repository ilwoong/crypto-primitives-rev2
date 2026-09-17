---
name: formatter
description: 저장소 루트의 .clang-format(LLVM 기반, 4칸 들여쓰기, 120자)으로 이번 파이프라인에서 바뀐 C 파일만 포맷팅하는 에이전트. coder, fixer, tester가 코드를 바꾼 뒤마다 호출됨.
tools: Read, Edit, Bash, Glob, Grep
model_hint: low
---

이 파일은 Hermes용 래퍼입니다(Claude Code용 `.claude/agents/formatter.md`와 같은 패턴).
역할 지시문은 `docs/agents/formatter.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요.
저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

## Hermes 위임 시
- 이 에이전트는 신규 파일을 만들지 않습니다. `tools:`와 `model_hint`는 안내용이므로, 오케스트레이터가 역할 제한을 목표에 명시하고 상태 비교로 검증합니다.
- `delegate_task`로 호출할 때 목표는 "포맷팅 단계 수행"으로, 컨텍스트에는 이번 파이프라인에서 바뀐 파일 목록을 넘기세요.

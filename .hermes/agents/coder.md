---
name: coder
description: crypto-primitives-rev2의 C 코드를 작성/수정하는 에이전트. 새 알고리즘 포팅, 새 변형 추가, 버그 수정 등 요청된 변경의 실제 코드를 작성. 파이프라인의 첫 단계이며, reviewer의 findings를 받아 최소 수정하는 역할로도 재호출됨.
tools: Read, Write, Edit, Bash, Grep, Glob
model_hint: high
---

이 파일은 Hermes용 래퍼입니다(Claude Code용 `.claude/agents/coder.md`와 같은 패턴).
역할 지시문은 `docs/agents/coder.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요.
저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

## Hermes 위임 시
- `delegate_task`로 이 역할을 호출할 때는 목표/컨텍스트에 위 설명( description )을 그대로 넘기세요.
- 도구 권한은 이 파일의 `tools:` 줄이 아니라 `docs/agents/coder.md`의 "권한" 줄과 AGENTS.md 파이프라인 설명에 근거합니다.
- `model_hint`는 안내용입니다. `delegate_task`는 이 값을 적용하지 않으며 전역 모델 정책을 사용합니다.

---
name: tester
description: crypto-primitives-rev2를 빌드하고 CTest 전체, UBSan+ASan, KAT 변조 검사를 실행하는 에이전트. reviewer가 findings 없음을 보고한 뒤, fixer 이전 단계로 사용. 누락된 테스트 케이스를 추가하되 로직은 고치지 않음. fixer 수정 후 재호출됨(최대 2회).
tools: Bash, Read, Write, Edit, Glob, Grep
model_hint: medium
---

이 파일은 Hermes용 래퍼입니다(Claude Code용 `.claude/agents/tester.md`와 같은 패턴).
역할 지시문은 `docs/agents/tester.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요.
저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

## Hermes 위임 시
- 이 에이전트는 `tests/`만 씁니다. `tools:`와 `model_hint`는 안내용이므로, 오케스트레이터가 역할 제한을 목표에 명시하고 상태 비교로 검증합니다.
- 명령은 AGENTS.md의 "명령" 절을 그대로 쓰세요.
- 실패 시 로직은 수정하지 않고 보고만 합니다. 수정은 fixer가 담당합니다.

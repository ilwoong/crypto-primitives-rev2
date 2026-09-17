---
name: fixer
description: tester가 보고한 빌드 경고/에러, CTest 실패, sanitizer 실패를 수정하는 에이전트. tester 다음 단계이며, 수정 후 tester가 재호출됨(최대 2회). reviewer findings는 coder가 처리하므로 이 에이전트의 대상이 아님.
tools: Read, Edit, Bash, Grep, Glob
model_hint: medium
---

이 파일은 Hermes용 래퍼입니다(Claude Code용 `.claude/agents/fixer.md`와 같은 패턴).
역할 지시문은 `docs/agents/fixer.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요.
저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

## Hermes 위임 시
- 이 에이전트는 기존 파일만 수정합니다(신규 파일 생성 없음). `tools:`와 `model_hint`는 안내용이므로, 오케스트레이터가 역할 제한을 목표에 명시하고 상태 비교로 검증합니다.
- 입력은 tester의 실패 보고입니다. reviewer findings는 받지 않습니다.
- `src/` 또는 `include/`를 바꿨다면 오케스트레이터가 formatter → reviewer를 다시 돌리도록 보고하세요.

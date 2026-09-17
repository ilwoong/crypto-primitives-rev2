---
name: documenter
description: 파이프라인의 마지막 단계로, 완료된 변경을 README.md의 알고리즘 표, 테스트 벡터 출처 표, 제약 사항에 반영하는 에이전트. 코드 작성/포맷/리뷰/테스트가 모두 끝난 뒤 사용.
tools: Read, Edit, Write, Grep, Glob, Bash
model_hint: low
---

이 파일은 Hermes용 래퍼입니다(Claude Code용 `.claude/agents/documenter.md`와 같은 패턴).
역할 지시문은 `docs/agents/documenter.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요.
저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

## Hermes 위임 시
- 이 에이전트는 `README.md`만 씁니다. `tools:`와 `model_hint`는 안내용이므로, 오케스트레이터가 역할 제한을 목표에 명시하고 상태 비교로 검증합니다.
- 메모리 셸은 읽기 명령만 허용하세요.

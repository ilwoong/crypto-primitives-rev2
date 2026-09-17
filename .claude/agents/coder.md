---
name: coder
description: crypto-primitives-rev2의 C 코드를 작성/수정하는 에이전트. 새 알고리즘 포팅, 새 변형 추가, 버그 수정 등 요청된 변경의 실제 코드를 작성. 파이프라인의 첫 단계이며, reviewer의 findings를 받아 최소 수정하는 역할로도 재호출됨.
tools: Read, Write, Edit, Bash, Grep, Glob
model: opus
---

이 파일은 Claude Code용 래퍼입니다(도구 제한과 모델 지정만 담음). 역할 지시문은 `docs/agents/coder.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요. 저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

---
name: tester
description: crypto-primitives-rev2를 빌드하고 CTest 전체, UBSan+ASan, KAT 변조 검사를 실행하는 에이전트. reviewer가 findings 없음을 보고한 뒤, fixer 이전 단계로 사용. 누락된 테스트 케이스를 추가하되 로직은 고치지 않음. fixer 수정 후 재호출됨(최대 2회).
tools: Bash, Read, Write, Edit, Glob, Grep
model: sonnet
---

이 파일은 Claude Code용 래퍼입니다(도구 제한과 모델 지정만 담음). 역할 지시문은 `docs/agents/tester.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요. 저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

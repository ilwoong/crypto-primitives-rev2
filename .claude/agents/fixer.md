---
name: fixer
description: tester가 보고한 빌드 경고/에러, CTest 실패, sanitizer 실패를 수정하는 에이전트. tester 다음 단계이며, 수정 후 tester가 재호출됨(최대 2회). reviewer findings는 coder가 처리하므로 이 에이전트의 대상이 아님.
tools: Read, Edit, Bash, Grep, Glob
model: sonnet
---

이 파일은 Claude Code용 래퍼입니다(도구 제한과 모델 지정만 담음). 역할 지시문은 `docs/agents/fixer.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요. 저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

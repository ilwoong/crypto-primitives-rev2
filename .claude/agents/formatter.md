---
name: formatter
description: 저장소 루트의 .clang-format(LLVM 기반, 4칸 들여쓰기, 120자)으로 이번 파이프라인에서 바뀐 C 파일만 포맷팅하는 에이전트. coder, fixer, tester가 코드를 바꾼 뒤마다 호출됨.
tools: Read, Edit, Bash, Glob, Grep
model: haiku
---

이 파일은 Claude Code용 래퍼입니다(도구 제한과 모델 지정만 담음). 역할 지시문은 `docs/agents/formatter.md`에 있습니다. 작업을 시작하기 전에 그 파일을 끝까지 읽고 그대로 따르세요. 저장소 규칙은 `AGENTS.md`와 `README.md`에 있습니다.

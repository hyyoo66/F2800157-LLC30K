# Git 백업 설정 가이드 (다른 DSP 프로젝트용 참고)

이 문서는 `F2800157-LLC30K` 프로젝트에서 설정한 git 백업 방법을 정리한 것입니다.
새로운 프로젝트 디렉토리에 동일하게 적용하면 됩니다.

---

## 1. GitHub 계정 정보

- **GitHub 사용자명**: `hyyoo66`
- **기존 저장소 예시**: `https://github.com/hyyoo66/F2800157-LLC30K`

---

## 2. 새 GitHub 저장소 생성 방법

1. [github.com/new](https://github.com/new) 접속
2. **Repository name**: 프로젝트명 입력 (예: `DSP280039C-작업명`)
3. **Description**: 간단한 설명 (선택)
4. **Visibility**: Private 권장
5. **"Initialize this repository" 체크 해제** ← 중요
6. **Add README / Add .gitignore / Choose a license → 모두 None**
7. **Create repository** 클릭

---

## 3. 로컬 프로젝트에 git 초기화 및 연결

```bash
# 프로젝트 디렉토리에서
git init
git add -A
git commit -m "init: 초기 커밋"

# GitHub 저장소 연결 (위에서 만든 URL 사용)
git remote add origin https://github.com/hyyoo66/저장소이름.git
git push -u origin master
```

---

## 4. 일상적인 백업 워크플로우

```bash
git add -A
git commit -m "설명 메시지"
git push
```

---

## 5. .gitignore 설정 (CCS C2000 프로젝트용)

아래 내용으로 프로젝트 루트에 `.gitignore` 파일을 만드세요.

```
# 빌드 출력물
CPU1_LAUNCHXL_FLASH/
CPU1_LAUNCHXL_RAM/
*.out
*.map
*.obj
*.pp
*.asm
*_linkInfo.xml

# SysConfig 자동 생성
syscfg/

# 백업 압축파일 (git으로 대체)
backup_*.tar.gz
backup_*.zip
*.zip
*.tar.gz

# 상태/메모 텍스트 (임시)
STATUS_*.txt
CHANGES_*.txt
BACKUP_*.txt
QUICK_REFERENCE_*.txt
README_*.txt

# driverlib 바이너리 (C2000Ware에서 재빌드 가능)
device/driverlib/ccs/Debug/
device/driverlib/ccs/Release/

# CCS 임시
.settings/
RemoveCommand.bat
makefile
objects.mk
sources.mk
subdir*.mk
```

---

## 6. Claude Code 자율 권한 설정 (settings.local.json)

Claude가 매번 허락 없이 프로젝트 내 파일을 쓰고 지울 수 있도록 아래 설정을 추가합니다.

파일 위치: `프로젝트디렉토리\.claude\settings.local.json`

```json
{
  "permissions": {
    "allow": [
      "Write(D:/Ti_280039_Work_Space/프로젝트폴더/**)",
      "Edit(D:/Ti_280039_Work_Space/프로젝트폴더/**)",
      "PowerShell(Copy-Item *)",
      "PowerShell(Remove-Item *)",
      "PowerShell(Move-Item *)",
      "PowerShell(New-Item *)",
      "PowerShell(git *)",
      "Bash(git *)",
      "Bash(Remove-Item \"D:/Ti_280039_Work_Space/프로젝트폴더/*\" *)",
      "Bash(Copy-Item * \"D:/Ti_280039_Work_Space/프로젝트폴더/*\" *)"
    ]
  }
}
```

> **참고**: git이 버전 관리를 해주므로 실수로 파일을 지워도 `git restore 파일명` 으로 복구 가능합니다.

---

## 7. 실수로 파일 삭제했을 때 복구

```bash
# 특정 파일 복구
git restore 파일명.c

# 전체 복구
git restore .

# 이전 커밋으로 되돌리기
git log --oneline        # 커밋 목록 확인
git checkout 커밋해시 -- 파일명.c
```

# IOCP Simple MMORPG

2020년 학부 서버 프로그래밍 텀 프로젝트. IOCP 기반 멀티스레드 MMORPG 서버.

## 구조
- 워커 스레드 4 + 타이머 스레드 1
- 40×40 섹터 기반 시야 처리
- A* 길찾기 / Lua NPC AI
- ODBC(SQL Server) 연동

## 부하 테스트 (SimpleStressTest)
서버 응답에 담긴 시각으로 왕복 지연을 측정하고, 지연이 임계(150ms)를 넘으면
접속 증가를 멈추는 방식으로 최대 동시 접속을 탐색. 동시 접속 9,000개 확인.

## 링크
- [개발 이력 (IOCPServer)](https://github.com/pypy8171/IOCPServer)
- [시연 영상](https://www.youtube.com/@%EB%B0%95%EC%9A%A9%ED%99%98-o9s)

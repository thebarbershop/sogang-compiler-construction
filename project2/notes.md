# 4월 16일 수업 중 가이드라인

LALR parser를 짜서 AST를 만들어야.
p. 492쪽을 열심히 보자.
void token이 id나 unknown으로 쓰면 안되지? 책에 syntex analyzer를 보자.

test코드는 안 내도 된다.
올려주는 예제 코드는 그냥 활용만 해라.

due는 5/3 23시
디렉터리째로 압축할 것

* node type: declaration타입 이 필요하다 (강의자료 7-7 Syntax tree structure for the tiny compiler)에 NodeKind에 추가; ExpKind도 변형.
Decl Node는 변수, 함수, 파라미터 각각에 대해 필요.
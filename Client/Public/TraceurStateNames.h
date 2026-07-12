#pragma once
#include "Client_Define.h"
#include "StateMachine.h"

NS_BEGIN(Client)

// JSON의 문자열("GROUND/Move", "FallingIdle")을 enum 값으로 해석하는 정적 테이블.
// 상태/애니메이션 enum(TraceurState_Enum.h)이 바뀌면 이 파일의 테이블도 같이 갱신할 것.
class CTraceurStateNames final
{
public:
	// "GROUND/Move" 형식 → StateKey. 실패 시 false
	static _bool Resolve_StateKey(const _string& strPath, Engine::StateKey& OutKey);

	// 해당 상태의 애니메이션 이름 → 애님 인덱스. 실패 시 false
	static _bool Resolve_AnimIndex(const Engine::StateKey& Key, const _string& strAnim, _uint& iOutIndex);

	// 오류 메시지용 역변환 ("GROUND/Move" 형식, 미지의 키면 "?/?")
	static _string To_String(const Engine::StateKey& Key);
};

NS_END

#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CBlackBoard final : public CBase
{
public:
	//using StringID = _uint;
	//inline StringID sid(const char* s)
	//{
	//	_uint hash = 2166136261u;
	//	for(; *s; ++s)
	//	{ 
	//		hash ^= _ubyte(*s);  
	//		hash *= 16777619u;
	//	}
	//	return hash;
	//}
#pragma region DATA_TYPE
	enum DATA_TYPE : _ubyte
	{
		INT,
		FLOAT,
		MASK,
		BOOL,
		VECTOR3,
		VECTOR4,
		DATA_END
	};

	template<class T> constexpr DATA_TYPE DeduceType(T value) { return DATA_TYPE::DATA_END; }
	template<> inline constexpr DATA_TYPE DeduceType<_int>(_int value) { return DATA_TYPE::INT; }
	template<> inline constexpr DATA_TYPE DeduceType<_float>(_float value) { return DATA_TYPE::FLOAT; }
	template<> inline constexpr DATA_TYPE DeduceType<_uint>(_uint value) { return DATA_TYPE::MASK; }
	template<> inline constexpr DATA_TYPE DeduceType<_bool>(_bool value) { return DATA_TYPE::BOOL; }
	template<> inline constexpr DATA_TYPE DeduceType<_float3>(_float3 value) { return DATA_TYPE::VECTOR3; }
	template<> inline constexpr DATA_TYPE DeduceType<_float4>(_float4 value) { return DATA_TYPE::VECTOR4; }
#pragma endregion
	/*typedef struct tFieldAccessor
	{
		DATA_TYPE eType;
		std::function<_bool(_variant& out)> getter;
		std::function<_bool(const _variant& in)> setter;
	}ACCESSOR;*/

	//typedef map<StringID, ACCESSOR> BLACKBOARD_DATA;

private:
	explicit CBlackBoard();
	virtual ~CBlackBoard() = default;

public:
	HRESULT Add_Data(const _string& strDataTag, DATA_TYPE eType, void* pValue);
	//void Add_Data(StringID uKey, ACCESSOR tAcc);
	//template<class Owner, class T>
	//ACCESSOR BindField(const Owner* pOwner, T Owner::* value);

	void* Get_Data(const _string& strDataTag);
	//_bool Get_Data(StringID uDataKey, _variant& out);
	//_bool Set_Data(StringID uDataKey, const _variant& in);
	HRESULT Add_Condition(const _string& strDataTag, function<_bool()> Condition);
	_bool Get_Condition(const _string& strFuncTag);

	_bool Necessary_Key_Check(vector<_string>& RequireKey);

#ifdef _DEBUG
	// 블랙보드에 바인딩 된 데이터 시각화
	void Bind_Data_to_GUI();
	void Unbind_Data(const _string& strDataTag);
	void Clear_Data();
#endif // _DEBUG


private:
	typedef map<_string, pair<DATA_TYPE, void*>> BLACKBOARD_DATA;
	BLACKBOARD_DATA			m_Datas;
	map<const _string, function<_bool()>> m_Conditions;

private:
	//_bool TypeMatches(DATA_TYPE eType, const _variant& Var);
	_bool Find_Data(const _string& strDataTag);
	_bool Find_Condition(const _string& strDataTag);
	//ACCESSOR* Find(StringID uKey);
	//const ACCESSOR* Find(StringID uKey) const;

public:
	static CBlackBoard* Create();
	virtual void Free() override;
};

NS_END

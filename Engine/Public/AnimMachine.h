#pragma once
#include "Component.h"

NS_BEGIN(Engine)
class CModel;
class CComputeShader;
class CTransform;
class CAnimState;
class CAnimTransition;

class ENGINE_DLL CAnimMachine final : public CComponent
{
public:
	typedef struct tagAnimMacnineDesc
	{
		_string pAnimationTag;
	}ANIMMACNINE_DESC;

private:
	explicit CAnimMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	explicit CAnimMachine(const CAnimMachine& Prototype);
	virtual ~CAnimMachine() = default;

public:
#ifdef _DEBUG
	_bool				isEmpty() { return m_AnimStates.empty(); }
	virtual HRESULT		Initialize_Prototype(/*const _char* AnimMachineDataPath*/) override;
#endif // _DEBUG
	virtual HRESULT		Initialize_Prototype(const _char* AnimMachineDataPath);
	virtual HRESULT		Initialize_Clone(void* pArg) override;

	//void Handle_Input(CModel* pModelCom, _uint* pState, _uint iIndex);
	void Handle_Input(CModel* pModelCom, _uint* pState,_string& strAnimTag, _float fTargetTrackPos = 0.f);
	// cpu
	void Update(CModel* pModelCom, CTransform* pTransform, _uint* pState, _bool& m_isAnimFinished, _float fTimeDelata);
	// gpu
	void Update(CModel* pModelCom, CComputeShader* pComputeShaderCom, CTransform* pTransform, _uint* pState, _bool& isAnimFinished, _float fTimeDelta);
	// facial
	void Update(CModel* pModelCom, CComputeShader* pComputeShaderCom, CComputeShader* pFacialShaderCom, CTransform* pTransform, _uint* pState, _bool& isAnimFinished, _float fTimeDelta);
	
	void Reset(CModel* pModelCom, const _string& strAnimTag);

	_string Get_CurrentAnimationTag() const;

#ifdef _DEBUG
	void Create_AnimStates(const vector<_string>& AnimationNames);
	void Clear_States();
	void Reset_StateData(_string& strAnimName,
						_bool isRootMotion,
						_bool isRootMotionRotate,
						_bool isRootMotionTranslate,
						_bool isLoop,
						_float fRootMotionRate,
						_float fTransitTrackPos,
						_float fAnimationSpeed);
	void Get_StateData(_string& strAnimName,
						_bool& isRootMotion,
						_bool& isRootMotionRotate,
						_bool& isRootMotionTranslate,
						_bool& isLoop,
						_float& fRootMotionRate,
						_float& fTransitTrackPos,
						_float& fAnimationSpeed);

	_bool Render_CurrentStateGUI(_string& strCurrentAnim);

	void Save_AnimDatas(json& jsonOutput);
	void Load_AnimDatas(json& jsonInput);
#endif // _DEBUG


private:
	_string m_strCurrentAnimTag;
	//vector<class CAnimState*> m_AnimStates;
	map<_string, CAnimState*> m_AnimStates;
	vector<CAnimTransition*> m_AnyState;
	//_uint m_iCurrentStateIndex{};

	_float m_fCurrentTrackPositon{};

	//상황에 따른 변동 변수
	_bool	m_isRootMotion{};
	_bool	m_isRootMotionRotate{};
	_bool	m_isRootMotionTranslate{};
	_bool	m_isLoop{};
	_float	m_fRootMotionRate{};
	_float	m_fTransitTrackPos{};
	_float	m_fAnimationSpeed{};

public:
#ifdef _DEBUG
	static CAnimMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext /*, const _char* AnimMachineDataPath*/);
#endif // _DEBUG
	static CAnimMachine* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext , const _char* AnimMachineDataPath);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};
NS_END

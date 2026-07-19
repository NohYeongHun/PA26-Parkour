#include "EnginePch.h"
#include "AnimMachine.h"
#include "Model.h"
#include "Animator.h"
#include "AnimState.h"
#include "AnimTransition.h"
#include "AnimStateFactory.h"

#pragma region ANIM_STATE

#pragma endregion

CAnimMachine::CAnimMachine(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CAnimMachine::CAnimMachine(const CAnimMachine& Prototype)
	: CComponent{ Prototype }
	, m_AnimStates { Prototype.m_AnimStates }
	, m_AnyState { Prototype.m_AnyState }
{
	for(auto& Pair : m_AnimStates)
		Safe_AddRef(Pair.second);

	for(auto& pTransition : m_AnyState)
		Safe_AddRef(pTransition);
}
#ifdef _DEBUG
HRESULT CAnimMachine::Initialize_Prototype()
{
	return S_OK;
}
#endif // _DEBUG
HRESULT CAnimMachine::Initialize_Prototype(const _char* AnimMachineDataPath)
{
	ifstream file(AnimMachineDataPath);
	if(false == file.is_open())
		CRASH("Load File Open");
	
	json ASM_Data;
	file >> ASM_Data;

	for(auto& AnimState : ASM_Data["AnimStates"])
	{
		_string StateName = AnimState["Name"];
		CAnimState::ANIMSTATE_DESC DataDesc{};

		m_AnimStates.emplace(StateName, CAnimState::Create(AnimState, ASM_Data["Transitions"]));
	}

	for(auto& AnyTransition : ASM_Data["AnyState"])
	{
		m_AnyState.push_back(CAnimTransition::Create(AnyTransition, CAnimStateFactory::Register_Transition(AnyTransition)));
	}
	std::sort(m_AnyState.begin(), m_AnyState.end(), [](CAnimTransition* Src, CAnimTransition* Dst)->_bool{
		return Src->Get_Priority() < Dst->Get_Priority();
		});

	return S_OK;
}

HRESULT CAnimMachine::Initialize_Clone(void* pArg)
{
	ANIMMACNINE_DESC* pDesc = (ANIMMACNINE_DESC*)pArg;
	if(nullptr == pDesc)
		return E_FAIL;
	m_strCurrentAnimTag = pDesc->pAnimationTag;

	CAnimState::ANIMSTATE_DESC Desc = m_AnimStates[m_strCurrentAnimTag]->Get_StateData();

	m_isRootMotion = Desc.isRootMotion;
	m_isRootMotionRotate = Desc.isRootMotionRotate;
	m_isRootMotionTranslate = Desc.isRootMotionTranslate;
	m_fRootMotionRate = Desc.fRootMotionRate;
	m_fTransitTrackPos = Desc.fTransitTrackPos;
	m_fAnimationSpeed = Desc.fAnimationSpeed;
	m_isLoop = Desc.isLoop;

    return S_OK;
}

void CAnimMachine::Handle_Input(CModel* pModelCom, _uint* pState, _string& strAnimTag, _float fTargetTrackPos)
{
	if(m_AnimStates.end() == m_AnimStates.find(strAnimTag))
		return;

	if(0 != m_strCurrentAnimTag.compare(strAnimTag))
	{
		m_AnimStates[m_strCurrentAnimTag]->Exit(pModelCom, pState);
		//pModelCom->Clear_Animation(m_strCurrentAnimTag);
		// Enter 새로운 상태
		CAnimState::ANIMSTATE_DESC Desc{};
		m_AnimStates[strAnimTag]->Enter(pModelCom, pState, &m_strCurrentAnimTag, Desc);

		m_isRootMotion = Desc.isRootMotion;
		m_isRootMotionRotate = Desc.isRootMotionRotate;
		m_isRootMotionTranslate = Desc.isRootMotionTranslate;
		m_isLoop = Desc.isLoop;
		m_fRootMotionRate = Desc.fRootMotionRate;
		m_fTransitTrackPos = Desc.fTransitTrackPos;
		m_fAnimationSpeed = Desc.fAnimationSpeed;

		pModelCom->Set_TrackPosition(m_strCurrentAnimTag, fTargetTrackPos);
		m_fCurrentTrackPositon = fTargetTrackPos;
	}
}

//cpu
void CAnimMachine::Update(CModel* pModelCom, CTransform* pTransform, _uint* pState, _bool& isAnimFinished, _float fTimeDelata)
{
	_bool AnyStateResult{};
	_string strNextAnimation;
	_float fTargetTrackPosition{};
	// 0. Any State Transition
	for(auto& pTransition : m_AnyState)
	{
		if(AnyStateResult = pTransition->Is_Transit(pState, strNextAnimation, fTargetTrackPosition))
		{
			Handle_Input(pModelCom, pState, strNextAnimation, fTargetTrackPosition);
			break;
		}
	}

	if(false == AnyStateResult)
	{
		// 1. 현재 상태 업데이트
		//m_AnimStates[m_iCurrentStateIndex]->Update( this, pModelCom, pState, &m_strCurrentAnimTag, fTimeDelata);
		m_AnimStates[m_strCurrentAnimTag]->Update(this, pModelCom, pState, &m_strCurrentAnimTag, m_fCurrentTrackPositon);
	}

	// 2. 애니메이션 재생 (CAnimator lazy-bound to the passed model)
	if (nullptr == m_pAnimator)
		m_pAnimator = CAnimator::Create(pModelCom);

	isAnimFinished = m_pAnimator->Play_Animation_CPU(m_strCurrentAnimTag, fTimeDelata, &m_fCurrentTrackPositon, false, m_isRootMotion, m_isRootMotionRotate, m_isRootMotionTranslate, m_fRootMotionRate);
	m_pAnimator->Sync_RootNode(pTransform, fTimeDelata);

	// 모델 클래스가 애니메이션 한 트랙이 끝까지 재생되었을 때 true 반환, 이후 모델 내에서 트랙 위치 초기화

	// 3. 결과 피드백 (우선 Norify에서 해결하는 방식으로 생각 중)
	m_AnimStates[m_strCurrentAnimTag]->Feedback(isAnimFinished, pState, this, pModelCom);
	if (m_isLoop)
		isAnimFinished = true;
}

//gpu
void CAnimMachine::Update(CModel* pModelCom, CComputeShader* pComputeShaderCom, CTransform* pTransform, _uint* pState, _bool& isAnimFinished, _float fTimeDelta)
{
	_bool AnyStateResult{};
	_string strNextAnimation;
	_float fTargetTrackPosition{};
	// 0. Any State Transition
	for(auto& pTransition : m_AnyState)
	{
		if(AnyStateResult = pTransition->Is_Transit(pState, strNextAnimation, fTargetTrackPosition))
		{
			Handle_Input(pModelCom, pState, strNextAnimation, fTargetTrackPosition);
			break;
		}
	}

	if(false == AnyStateResult)
		m_AnimStates[m_strCurrentAnimTag]->Update(this, pModelCom, pState, &m_strCurrentAnimTag, m_fCurrentTrackPositon);

	ANIMATION_PLAY_DESC playDesc{};
	playDesc.fSpeed = fTimeDelta;
	playDesc.strAnimationName = m_strCurrentAnimTag;
	playDesc.pTrackPosition = &m_fCurrentTrackPositon;
	playDesc.isFacial = false;

	ROOTMOTION_DESC rootMotionDesc{};
	rootMotionDesc.isEnable = m_isRootMotion;
	rootMotionDesc.isRotate = m_isRootMotionRotate;
	rootMotionDesc.isTranslate = m_isRootMotionTranslate;

	if (nullptr == m_pAnimator)
		m_pAnimator = CAnimator::Create(pModelCom);

	isAnimFinished = m_pAnimator->Play_NonRibAnimation_GPU(pComputeShaderCom, playDesc, rootMotionDesc, fTimeDelta);
	m_pAnimator->Sync_RootNode(pTransform, fTimeDelta);

	m_AnimStates[m_strCurrentAnimTag]->Feedback(isAnimFinished, pState, this, pModelCom);
	if (m_isLoop)
		isAnimFinished = true;
}

//facial
void CAnimMachine::Update(CModel* pModelCom, CComputeShader* pComputeShaderCom, CComputeShader* pFacialShaderCom, CTransform* pTransform, _uint* pState, _bool& isAnimFinished, _float fTimeDelta)
{
	_bool AnyStateResult{};
	_string strNextAnimation;
	_float fTargetTrackPosition{};
	// 0. Any State Transition
	for (auto& pTransition : m_AnyState)
	{
		if (AnyStateResult = pTransition->Is_Transit(pState, strNextAnimation, fTargetTrackPosition))
		{
			Handle_Input(pModelCom, pState, strNextAnimation, fTargetTrackPosition);
			break;
		}
	}

	if (false == AnyStateResult)
		m_AnimStates[m_strCurrentAnimTag]->Update(this, pModelCom, pState, &m_strCurrentAnimTag, m_fCurrentTrackPositon);


	ANIMATION_PLAY_DESC playDesc{};
	playDesc.fSpeed = 1.f;
	playDesc.strAnimationName = m_strCurrentAnimTag;
	playDesc.pTrackPosition = &m_fCurrentTrackPositon;
	playDesc.isFacial = false;

	ROOTMOTION_DESC rootMotionDesc{};
	rootMotionDesc.isEnable = m_isRootMotion;
	rootMotionDesc.isRotate = m_isRootMotionRotate;
	rootMotionDesc.isTranslate = m_isRootMotionTranslate;

	if (nullptr == m_pAnimator)
		m_pAnimator = CAnimator::Create(pModelCom);

	isAnimFinished = m_pAnimator->Play_Animation_GPU(pComputeShaderCom, pFacialShaderCom, playDesc, rootMotionDesc, fTimeDelta);

	m_pAnimator->Sync_RootNode(pTransform, fTimeDelta);

	m_AnimStates[m_strCurrentAnimTag]->Feedback(isAnimFinished, pState, this, pModelCom);
	if (m_isLoop)
		isAnimFinished = true;
}

void CAnimMachine::Reset(CModel* pModelCom, const _string& strAnimTag)
{
	if (m_AnimStates.end() == m_AnimStates.find(strAnimTag))
		return;
	for (auto& Pair : m_AnimStates)
	{
		pModelCom->Clear_Animation(Pair.first);
	}
	CAnimState::ANIMSTATE_DESC Desc{};
	m_AnimStates[strAnimTag]->Enter(pModelCom, nullptr, &m_strCurrentAnimTag, Desc);

	m_isRootMotion = Desc.isRootMotion;
	m_isRootMotionRotate = Desc.isRootMotionRotate;
	m_isRootMotionTranslate = Desc.isRootMotionTranslate;
	m_isLoop = Desc.isLoop;
	m_fRootMotionRate = Desc.fRootMotionRate;
	m_fTransitTrackPos = Desc.fTransitTrackPos;
	m_fAnimationSpeed = Desc.fAnimationSpeed;
}

_string CAnimMachine::Get_CurrentAnimationTag() const
{
	return m_strCurrentAnimTag;
}

#ifdef _DEBUG
void CAnimMachine::Create_AnimStates(const vector<_string>& AnimationNames)
{
	Clear_States();
	for(auto& strAnimationName : AnimationNames)
	{
		CAnimState::ANIMSTATE_DESC Temp{ true, false, true, false, 1.f, 0.f, 1.f };
		m_AnimStates.emplace(strAnimationName, CAnimState::Create(strAnimationName, Temp));
	}
}

void CAnimMachine::Clear_States()
{
	for(auto& Pair : m_AnimStates)
		Safe_Release(Pair.second);
	m_AnimStates.clear();
}

void CAnimMachine::Reset_StateData(_string& strAnimName,_bool isRootMotion,  _bool isRootMotionRotate, _bool isRootMotionTranslate, _bool isLoop, _float fRootMotionRate, _float fTransitTrackPos, _float fAnimationSpeed)
{
	if(m_AnimStates.find(strAnimName) == m_AnimStates.end())
		return;

	CAnimState::ANIMSTATE_DESC AnimStateDesc{};
	AnimStateDesc.isRootMotion = isRootMotion;
	AnimStateDesc.isRootMotionRotate = isRootMotionRotate;
	AnimStateDesc.isRootMotionTranslate = isRootMotionTranslate;
	AnimStateDesc.isLoop = isLoop;
	AnimStateDesc.fRootMotionRate = fRootMotionRate;
	AnimStateDesc.fTransitTrackPos = fTransitTrackPos;
	AnimStateDesc.fAnimationSpeed = fAnimationSpeed;

	m_AnimStates[strAnimName]->Set_Data(AnimStateDesc);

}

void CAnimMachine::Get_StateData(_string& strAnimName, _bool& isRootMotion, _bool& isRootMotionRotate, _bool& isRootMotionTranslate, _bool& isLoop, _float& fRootMotionRate, _float& fTransitTrackPos, _float& fAnimationSpeed)
{
	if(m_AnimStates.find(strAnimName) == m_AnimStates.end())
		return;

	CAnimState::ANIMSTATE_DESC AnimStateDesc = m_AnimStates[strAnimName]->Get_StateData();
	isRootMotion = AnimStateDesc.isRootMotion;
	isRootMotionRotate = AnimStateDesc.isRootMotionRotate;
	isRootMotionTranslate = AnimStateDesc.isRootMotionTranslate;
	isLoop = AnimStateDesc.isLoop;
	fRootMotionRate = AnimStateDesc.fRootMotionRate;
	fTransitTrackPos = AnimStateDesc.fTransitTrackPos;
	fAnimationSpeed = AnimStateDesc.fAnimationSpeed;
}

_bool CAnimMachine::Render_CurrentStateGUI(_string& strCurrentAnim)
{
	if(m_AnimStates.find(strCurrentAnim) == m_AnimStates.end())
		return false;

	m_AnimStates[strCurrentAnim]->Render_GUI();
	return true;
}

CAnimMachine* CAnimMachine::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CAnimMachine* pInstance = new CAnimMachine(pDevice, pContext);
    if(FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CAnimMachine");
        Safe_Release(pInstance);
    }
    return pInstance;
}
#endif

CAnimMachine* CAnimMachine::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _char* AnimMachineDataPath)
{
	CAnimMachine* pInstance = new CAnimMachine(pDevice, pContext);
	if(FAILED(pInstance->Initialize_Prototype(AnimMachineDataPath)))
	{
		MSG_BOX("Failed to Created : CAnimMachine");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CAnimMachine::Clone(void* pArg)
{
    CAnimMachine* pInstance = new CAnimMachine(*this);
    if(FAILED(pInstance->Initialize_Clone(pArg)))
    {
        MSG_BOX("Failed to Cloned : CAnimMachine");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CAnimMachine::Free()
{
    __super::Free();

	for(auto& Pair : m_AnimStates)
		Safe_Release(Pair.second);
	for(auto& AnyTransit : m_AnyState)
		Safe_Release(AnyTransit);

	m_AnimStates.clear();

	Safe_Release(m_pAnimator);
}

#ifdef _DEBUG
void CAnimMachine::Save_AnimDatas(json& jsonOutput)
{

	jsonOutput["AnimStates"] = json::array();
	for(auto& Pair : m_AnimStates)
	{
		json AnimState;
		AnimState["Name"] = Pair.first;
		CAnimState::ANIMSTATE_DESC DataDesc = Pair.second->Get_StateData();

		AnimState["isRootMotion"] = DataDesc.isRootMotion;
		AnimState["isRootMotionRotate"] = DataDesc.isRootMotionRotate;
		AnimState["isRootMotionTranslate"] = DataDesc.isRootMotionTranslate;
		AnimState["isLoop"] = DataDesc.isLoop;
		AnimState["fRootMotionRate"] = DataDesc.fRootMotionRate;
		AnimState["fTransitTrackPos"] = DataDesc.fTransitTrackPos;
		AnimState["fAnimationSpeed"] = DataDesc.fAnimationSpeed;

		jsonOutput["AnimStates"].push_back(AnimState);
	}
}

void CAnimMachine::Load_AnimDatas(json& jsonInput)
{
	for(auto& AnimState : jsonInput["AnimStates"])
	{
		_string StateName = AnimState["Name"];
		CAnimState::ANIMSTATE_DESC DataDesc{};
		
		DataDesc.isRootMotion = AnimState["isRootMotion"];
		DataDesc.isRootMotionRotate = AnimState["isRootMotionRotate"];
		DataDesc.isRootMotionTranslate = AnimState["isRootMotionTranslate"];
		DataDesc.isLoop = AnimState["isLoop"];
		DataDesc.fRootMotionRate = AnimState["fRootMotionRate"];
		DataDesc.fTransitTrackPos = AnimState["fTransitTrackPos"];
		DataDesc.fAnimationSpeed = AnimState["fAnimationSpeed"];
		
		m_AnimStates.emplace(StateName, CAnimState::Create(AnimState, jsonInput["Transitions"]));
	}

	for(auto& AnyTransition : jsonInput["AnyState"])
	{
		m_AnyState.push_back(CAnimTransition::Create(AnyTransition, CAnimStateFactory::Register_Transition(AnyTransition)));
	}
	std::sort(m_AnyState.begin(), m_AnyState.end(), [](CAnimTransition* Src, CAnimTransition* Dst)->_bool{
		return Src->Get_Priority() < Dst->Get_Priority();
		});
}
#endif
#include "EnginePch.h"
#include "AnimState.h"
#include "Model.h"
#include "AnimMachine.h"
#include "AnimTransition.h"
#include "AnimStateFactory.h"

CAnimState::CAnimState(const CAnimState& Prototype)
	: m_Transitions { Prototype.m_Transitions },
	m_strAnimationTag { Prototype.m_strAnimationTag },
	m_StateData{ Prototype.m_StateData }
{
	for(auto& Transition : m_Transitions)
		Safe_AddRef(Transition);
}

#ifdef _DEBUG

HRESULT CAnimState::Initialize(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc)
{
	m_StateData = StateDesc;
	m_strAnimationTag = strAnimationTag;
    return S_OK;
}
void CAnimState::Set_Data(ANIMSTATE_DESC& StateDesc)
{
	m_StateData = StateDesc;
}
void CAnimState::Render_GUI()
{
	if(ImGui::CollapsingHeader("Anim State Property"))
	{
		ImGui::Text("Animation: %s", m_strAnimationTag.c_str());
		m_StateData.isRootMotion ? ImGui::Text("isRootMotion : true") : ImGui::Text("isRootMotion : false");
		m_StateData.isRootMotionRotate ? ImGui::Text("isRootMotionRotate : true") : ImGui::Text("isRootMotionRotate : false");
		m_StateData.isRootMotionTranslate ? ImGui::Text("isRootMotionTranslate : true") : ImGui::Text("isRootMotionTranslate : false");
		m_StateData.isLoop ? ImGui::Text("isLoop : true") : ImGui::Text("isLoop : false");
		ImGui::Text("RootMotionRate: %.3f", m_StateData.fRootMotionRate);
		ImGui::Text("TransitTrackPos: %.3f", m_StateData.fTransitTrackPos);
		ImGui::Text("AnimationSpeed: %.3f", m_StateData.fAnimationSpeed);
		//ImGui::Text("RootMotionRate: %u", m_iConstAnimRunning);
	}
}
#endif

HRESULT CAnimState::Initialize(json& jsonParser, json& jsonTransitions)
{
	m_strAnimationTag = jsonParser["Name"];
	m_StateData.isRootMotion = jsonParser["isRootMotion"];
	m_StateData.isRootMotionRotate = jsonParser["isRootMotionRotate"];
	m_StateData.isRootMotionTranslate = jsonParser["isRootMotionTranslate"];
	m_StateData.isLoop = jsonParser["isLoop"];
	m_StateData.fRootMotionRate = jsonParser["fRootMotionRate"];
	m_StateData.fTransitTrackPos = jsonParser["fTransitTrackPos"];
	m_StateData.fAnimationSpeed = jsonParser["fAnimationSpeed"];

	for(auto& jsonTransition : jsonTransitions)
	{
		if(0 == m_strAnimationTag.compare(jsonTransition["From"]))
			m_Transitions.push_back(CAnimTransition::Create(jsonTransition, CAnimStateFactory::Register_Transition(jsonTransition)));
	}

	//오름차순 정렬
	sort(m_Transitions.begin(), m_Transitions.end(), [](CAnimTransition* Src, CAnimTransition* Dst)->_bool{
		return Src->Get_Priority() < Dst->Get_Priority();
		});

	return S_OK;
}

void CAnimState::Enter(CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag, ANIMSTATE_DESC& StateData)
{
	*pCurrentAnimTag = m_strAnimationTag;
	StateData = m_StateData;
	//상태 전환 시 초기 데이터 복원
	pModelCom->Clear_Animation(m_strAnimationTag);
}

void CAnimState::Update(class CAnimMachine* pAnimMachine, CModel* pModelCom, _uint* pOwnerState, _string* pCurrentAnimTag, _float fTrackPosition/*, ANIMSTATE_DESC& StateData*/)
{
	//*pOwnerState |= ENUM_CLASS(TEST_STATE::ANIMATION_PLAYING);

	_string strNextAnimTag{};
	_float fNextTargetTrackPos{};
	//루프가 아닌 애니메이션 자동 전환은 백터 맨 마지막에 설정
	CAnimTransition* pTemp = nullptr;
	if(false == m_Transitions.empty()) 
		pTemp = m_Transitions.back();
	if(pTemp && false == m_StateData.isLoop)
		m_Transitions.pop_back();
	for(auto& Transition : m_Transitions)
	{
		if(fTrackPosition < Transition->Get_TransitEnablePos())
			continue;

		if(Transition->Is_Transit(pOwnerState, strNextAnimTag, fNextTargetTrackPos))
		{
			pAnimMachine->Handle_Input(pModelCom, pOwnerState, strNextAnimTag, fNextTargetTrackPos);
			//if(pTemp && false == m_StateData.isLoop)
			//	m_Transitions.push_back(pTemp);
			//return;
			break;
		}
	}
	if(pTemp && false == m_StateData.isLoop)
		m_Transitions.push_back(pTemp);
}

void CAnimState::Exit(CModel* pModelCom, _uint* pOwnerState)
{

}

//_bool CAnimState::Play_Animation(CModel* pModelCom, _float fTimeDelta)
//{
//	return pModelCom->Play_Animation_CPU(m_strAnimationTag, fTimeDelta * m_fAnimationSpeed, &m_fCurrentTrackPositon, 
//										m_isBlend, m_isRootMotion, m_fRootMotionRate);
//	//return pModelCom->Play_Animation_CPU(m_strAnimationTag, fTimeDelta * m_StateData.fAnimationSpeed, &m_fCurrentTrackPositon);
//}
//
//_bool CAnimState::Play_Animation_GPU(CModel* pModelCom, CComputeShader* pComputeShaderCom, _float fTimeDelta)
//{
//	return pModelCom->Play_Animation_GPU(pComputeShaderCom, m_strAnimationTag, fTimeDelta * m_fAnimationSpeed, &m_fCurrentTrackPositon, 
//										m_isRootMotion, m_fRootMotionRate);
//	//return pModelCom->Play_Animation_GPU(pComputeShaderCom, m_strAnimationTag, fTimeDelta * m_StateData.fAnimationSpeed, &m_fCurrentTrackPositon);
//}

void CAnimState::Feedback(_bool isAnimationFinished, _uint* pOwnerState, CAnimMachine* pAnimMachineCom, CModel* pModelCom)
{
	if(isAnimationFinished)
	{
		//pAnimMachineCom->Handle_Input(pModelCom, pOwnerState, strNextAnimTag);
		if(m_Transitions.empty())
		{
			//다른 애니메이션으로 변환되지 않는 leap
		}
		//루프가 아닌 애니메이션 자동 전환은 백터 맨 마지막에 설정
		else if(false == m_StateData.isLoop)
		{
			_string strToState;
			_float fTargetTrackPos{};
			m_Transitions.back()->Get_ToStateData(strToState, fTargetTrackPos);
			pAnimMachineCom->Handle_Input(pModelCom, pOwnerState, strToState, fTargetTrackPos);
		}
	}
}
#ifdef _DEBUG
CAnimState* CAnimState::Create(const _string& strAnimationTag, ANIMSTATE_DESC& StateDesc)
{
	CAnimState* pInstance = new CAnimState();
	if(FAILED(pInstance->Initialize(strAnimationTag, StateDesc)))
	{
		MSG_BOX("Failed to Created : CAnimState");
		Safe_Release(pInstance);
	}
    return pInstance;
}
#endif

CAnimState* CAnimState::Create(json& jsonParser, json& jsonTransitions)
{
	CAnimState* pInstance = new CAnimState();
	if(FAILED(pInstance->Initialize(jsonParser, jsonTransitions)))
	{
		MSG_BOX("Failed to Created : CAnimState");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CAnimState* CAnimState::Clone()
{
	//CAnimState* pInstance = new CAnimState(*this);
	return new CAnimState(*this);
}

void CAnimState::Free()
{
    __super::Free();

	for(auto& pTransition : m_Transitions)
		Safe_Release(pTransition);
	m_Transitions.clear();
}

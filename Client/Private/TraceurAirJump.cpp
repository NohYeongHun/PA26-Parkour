#include "ClientPch.h"
#include "TraceurAirJump.h"
#include "Traceur.h"
#include "TraceurState_Enum.h"
#include "MovementComponent.h"
#include "EnvironmentQueryComponent.h"

HRESULT CTraceurAirJump::Initialize(CTraceur* pOwner)
{
	if (FAILED(__super::Initialize(pOwner)))
		return E_FAIL;

	Register_Flag("Land");
	Register_Flag("Fall");
	Register_Flag("Move");
	Register_Flag("Run");
	Register_Flag("ExitOpen");

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirJump::Jump);

	return S_OK;
}


void CTraceurAirJump::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurAirJump::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(true);
}

void CTraceurAirJump::Check_State()
{
	_float3 vGroundN{};
	_bool bSupported = m_pColliderCom->IsLand(&vGroundN);
	_bool bLand = bSupported && vGroundN.y >= cosf(XMConvertToRadians(50.f));
	Set_Flag("Land", bLand);
	Set_Flag("Fall", !bLand);
	//Set_Flag("ExitOpen", Get_Flag("Land") && m_fTrackPosition > 40.f);
	Set_Flag("Move", m_pInputControllerCom->Check_AnyInput(m_iMoveKey));
	Set_Flag("Run",  Get_Flag("Move") && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)));

#ifdef _DEBUG
	Debug_PrintFlag();
#endif // _DEBUG
}

void CTraceurAirJump::Update_Animations(_float fTimeDelta)
{
	__super::Play_Animation(fTimeDelta);

	_float fTargetWeight = 0.f;
	if (Get_Flag("Run"))
		fTargetWeight = 0.5f;
	else if (Get_Flag("Move"))
		fTargetWeight = 0.2f;

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	// 기존 움직임 외에 추가적인 움직임
	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurAirJump::Check_Physics(_float fTimeDelta)
{
}

void CTraceurAirJump::SetUp_Animations()
{
	CState::Add_Animations(ENUM_CLASS(ETraceurAirJump::Jump),
		{ &m_fTrackPosition, "Jump", 1.f, 0.1f, 0.2f, 0.f, false }, { 1.f, true, true, true });
}

CTraceurAirJump* CTraceurAirJump::Create(CTraceur* pOwner)
{
	CTraceurAirJump* pInstance = new CTraceurAirJump();

	if (FAILED(pInstance->Initialize(pOwner)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Create : CTraceurAirJump");
		return nullptr;
	}

	return pInstance;
}

void CTraceurAirJump::Free()
{
	__super::Free();
}

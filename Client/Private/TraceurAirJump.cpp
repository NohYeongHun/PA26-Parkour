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
	Register_Flag("Move");
	Register_Flag("Run");

	SetUp_Animations();
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirJump::Jump);

	return S_OK;
}

static constexpr _float JUMP_FORCE    = 450.f;
static constexpr _float GRAVITY_ACCEL = 9.81f * 0.7f;

void CTraceurAirJump::OnEnter(void* pArg)
{
	__super::OnEnter(pArg);
	m_iCurrentAnimIdx = ENUM_CLASS(ETraceurAirJump::Jump);
	m_pColliderCom->Set_Gravity(false);
	m_fVelocityY = JUMP_FORCE;
	Clear_Flags();
}

void CTraceurAirJump::OnExit()
{
	__super::OnExit();
	m_pColliderCom->Set_Gravity(false);
}

void CTraceurAirJump::Check_State()
{
	Set_Flag("Land", m_pColliderCom->IsLand());
	Set_Flag("Move", m_pInputControllerCom->Check_AnyInput(m_iMoveKey));
	Set_Flag("Run",  Get_Flag("Move") && m_pInputControllerCom->Check_AnyInput(ENUM_CLASS(KEYINPUT::LSHIFT)));
}

void CTraceurAirJump::Update_Animations(_float fTimeDelta)
{
	CTraceurState::Play_Animation(fTimeDelta);

	_float fTargetWeight = 0.f;
	if (Get_Flag("Run"))
		fTargetWeight = 0.5f;
	else if (Get_Flag("Move"))
		fTargetWeight = 0.2f;

	_vector vWorldDir = CMovementComponent::Calc_GroundDir(
		CMovementComponent::Calculate_Direction(m_pInputControllerCom),
		m_pOwner->Get_CamForward(), m_pOwner->Get_CamRight());

	m_pMoveCom->Move(vWorldDir, fTimeDelta, fTargetWeight);
}

void CTraceurAirJump::Check_Physics(_float fTimeDelta)
{
	//if (Get_Flag("Land")) return;

	//m_fVelocityY -= GRAVITY_ACCEL * fTimeDelta;

	_vector vPos = m_pTransformCom->Get_State(Engine::STATE::POSITION);
	vPos = XMVectorSetY(vPos, XMVectorGetY(vPos) + m_fVelocityY * fTimeDelta);
	m_pTransformCom->Set_State(Engine::STATE::POSITION, XMVectorSetW(vPos, 1.f));
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

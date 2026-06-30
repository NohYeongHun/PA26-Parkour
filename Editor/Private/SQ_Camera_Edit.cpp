#include "EditorPch.h"
#include "SQ_Camera_Edit.h"

CSQ_Camera_Edit::CSQ_Camera_Edit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CSQ_Camera_Edit::CSQ_Camera_Edit(const CSQ_Camera_Edit& Prototype)
	: CCamera{ Prototype }
{
}

HRESULT CSQ_Camera_Edit::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSQ_Camera_Edit::Initialize_Clone(void* pArg)
{
	if (FAILED(__super::Initialize_Clone(pArg)))
		CRASH("SQ_Camera_Edit");

    return S_OK;
}

void CSQ_Camera_Edit::Priority_Update(_float fTimeDelta)
{
}

void CSQ_Camera_Edit::Update(_float fTimeDelta)
{
	if (false == m_isActivate)
		return;

	m_fTrackPosition += fTimeDelta * m_fTrackPerSec;
	if (m_fTrackPosition >= m_fEndFrame || m_iFrameIndex == m_Frames.size() - 1)
	{
		m_isActivate = false;
		return;
	}

	if(-1 == m_iFrameIndex)
		m_fRatio = m_fTrackPosition / m_Frames[m_iFrameIndex + 1].fStartFrame;
	else	
		m_fRatio = (m_fTrackPosition - m_Frames[m_iFrameIndex].fStartFrame) / (m_Frames[m_iFrameIndex + 1].fStartFrame - m_Frames[m_iFrameIndex].fStartFrame);

	if (0 <= m_iFrameIndex && m_Frames[m_iFrameIndex + 1].isLerp == true)
	{
		// 1. Quat SLerp
		Lerp_Quat();
		// 2. Spline (Catmull-Rom)
		Spline();
	}

	// Frame Check
	if (m_fTrackPosition >= m_Frames[m_iFrameIndex + 1].fStartFrame)
	{
		++m_iFrameIndex;
		if (false == m_Frames[m_iFrameIndex].isLerp)
		{
			m_pTransformCom->Rotation_Quaternion(XMLoadFloat4(&m_Frames[m_iFrameIndex].vQuaternion));
			m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_Frames[m_iFrameIndex].vPosition), 1.f));
		}
	}
}

void CSQ_Camera_Edit::Late_Update(_float fTimeDelta)
{
}

void CSQ_Camera_Edit::Reset(const _fmatrix& WorldMatrix, void* pArg)
{
	SQ_CAMERA_DATA* pData = static_cast<SQ_CAMERA_DATA*>(pArg);
	if (nullptr == pData)
		return;
	m_isActivate = true;

	m_iFrameIndex = -1;
	m_Frames = pData->Frames;
	m_fStartFrame = pData->fStartFrame;
	m_fEndFrame = pData->fEndFrame;
	m_fTrackPosition = m_fStartFrame;
	m_fTrackPerSec = pData->fTrackPerSec;
}

void CSQ_Camera_Edit::Lerp_Quat()
{
	_vector vPreQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex].vQuaternion);
	_vector vDestQuat = XMLoadFloat4(&m_Frames[m_iFrameIndex + 1].vQuaternion);
	_vector vLerpQuat = XMQuaternionSlerp(vPreQuat, vDestQuat, m_fRatio);
	m_pTransformCom->Rotation_Quaternion(vLerpQuat);
}

void CSQ_Camera_Edit::Spline()
{
	_int iEnd = m_Frames.size() - 1;

	_int iIndex0{}, iIndex1{}, iIndex2{}, iIndex3{};

	if(true == m_Frames[m_iFrameIndex].isLerp)
		iIndex0 = clamp(m_iFrameIndex - 1,	0, iEnd);
	else
		iIndex0 = clamp(m_iFrameIndex, 0, iEnd);

	iIndex1 = clamp(m_iFrameIndex, 0, iEnd);
	iIndex2 = clamp(m_iFrameIndex + 1,	0, iEnd);
	iIndex3 = clamp(m_iFrameIndex + 2,	0, iEnd);

	if (false == m_Frames[iIndex3].isLerp)
	{
		if (false == m_Frames[iIndex2].isLerp)
		{
			iIndex3 = m_iFrameIndex;
			iIndex2 = m_iFrameIndex;
		}
		else
			iIndex3 = clamp(m_iFrameIndex + 1, 0, iEnd);
	}

	_vector vP0 = XMLoadFloat3(&m_Frames[iIndex0].vPosition);
	_vector vP1 = XMLoadFloat3(&m_Frames[iIndex1].vPosition);
	_vector vP2 = XMLoadFloat3(&m_Frames[iIndex2].vPosition);
	_vector vP3 = XMLoadFloat3(&m_Frames[iIndex3].vPosition);

	_float fSplineRatio = (m_fTrackPosition - m_Frames[iIndex1].fStartFrame) / (m_Frames[iIndex2].fStartFrame - m_Frames[iIndex1].fStartFrame);

	_float fRatio2 = fSplineRatio * fSplineRatio;
	_float fRatio3 = fRatio2 * fSplineRatio;

	_vector vSpline = 0.5f * (
		(2.f * vP1) + 
		(-1.f * vP0 + vP2) * fSplineRatio + 
		(2.f * vP0 - 5.f * vP1 + 4.f * vP2 - vP3) * fRatio2 +
		(-1.f * vP0 + 3.f * vP1 - 3.f * vP2 + vP3) * fRatio3
		);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(vSpline, 1.f));
}

CSQ_Camera_Edit* CSQ_Camera_Edit::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSQ_Camera_Edit* pInstance = new CSQ_Camera_Edit(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
		CRASH("SQ_Camera_Edit")

    return pInstance;
}

CGameObject* CSQ_Camera_Edit::Clone(void* pArg)
{
	CSQ_Camera_Edit* pClone = new CSQ_Camera_Edit(*this);

	if (FAILED(pClone->Initialize_Clone(pArg)))
		CRASH("SQ_Camera_Edit (Clone)")

	return pClone;
}

void CSQ_Camera_Edit::Free()
{
	__super::Free();
}

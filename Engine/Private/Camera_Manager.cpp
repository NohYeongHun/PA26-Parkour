#include "EnginePch.h"
#include "Camera_Manager.h"

#include "GameInstance.h"

#include "FreeCamera.h"

CCamera_Manager::CCamera_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pGameInstance { CGameInstance::GetInstance() },
	m_pDevice { pDevice }, m_pContext { pContext }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CCamera_Manager::Add_Camera(_uint iLevelID, const _wstring& strCameraTag, CCamera* pCamera)
{
    if (nullptr != Find_Camera(iLevelID, strCameraTag))
        return E_FAIL;

    m_Cameras[iLevelID].emplace(strCameraTag, pCamera);

    return S_OK;
}

HRESULT CCamera_Manager::Add_Camera(_uint iLevelID, const _wstring& strCameraTag, _uint iPrototypeLevelID, const _wstring& strPrototypeTag, void* pArg)
{
    if (nullptr != Find_Camera(iLevelID, strCameraTag))
        return E_FAIL;

    CCamera* pCamera = static_cast<CCamera*>(m_pGameInstance->Clone_Prototype(iPrototypeLevelID, strPrototypeTag, PROTOTYPE::GAMEOBJECT, pArg));
    if (nullptr == pCamera)
        return E_FAIL;

    m_Cameras[iLevelID].emplace(strCameraTag, pCamera);

    return S_OK;
}

HRESULT CCamera_Manager::Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag)
{
    CCamera* pCamera = Find_Camera(iLevelID, strCameraTag);
    if (nullptr == pCamera)
        return E_FAIL;

    Safe_Release(m_pMainCamera);
    m_pMainCamera = pCamera;
    Safe_AddRef(m_pMainCamera);

	m_pGameInstance->SetUp_CameraNF();

    return S_OK;
}

HRESULT CCamera_Manager::Change_MainCamera(_uint iLevelID, const _wstring& strCameraTag, void* pArg)
{
	CCamera* pCamera = Find_Camera(iLevelID, strCameraTag);
	if (nullptr == pCamera)
		return E_FAIL;

	Safe_Release(m_pMainCamera);
	m_pMainCamera = pCamera;
	m_pMainCamera->Reset(XMMatrixIdentity(), pArg);
	Safe_AddRef(m_pMainCamera);

	m_pGameInstance->SetUp_CameraNF();

	return S_OK;
}

_float CCamera_Manager::Get_CurrentCamera_Near()
{
    if (nullptr == m_pMainCamera || true == m_isFree)
        return m_pFreeCamera->Get_Near();
    else
        return m_pMainCamera->Get_Near();
}

_float CCamera_Manager::Get_CurrentCamera_Far()
{
    if (nullptr == m_pMainCamera || true == m_isFree)
        return m_pFreeCamera->Get_Far();
    else
        return m_pMainCamera->Get_Far();
}

void CCamera_Manager::Set_CurrentCamera_Far(_float fFar)
{
	if (nullptr == m_pMainCamera || true == m_isFree)
		m_pFreeCamera->Set_Far(fFar);
	else
		m_pMainCamera->Set_Far(fFar);
}

void CCamera_Manager::OnShake(const CAMERA_SHAKE& tData)
{
	if (nullptr == m_pMainCamera)
		return;

	m_pMainCamera->OnShake(tData);
}

HRESULT CCamera_Manager::Initialize(_uint iNumLevel)
{
    m_iNumLevel = iNumLevel;
    m_Cameras = new CAMERA[m_iNumLevel];

	Ready_FreeCamera();

    return S_OK;
}

void CCamera_Manager::Update(_float fTimeDelta)
{
//	if (m_pGameInstance->Get_DIKeyState(DIK_F11) == KEYSTATE::DOWN)
	if (m_pGameInstance->Get_DIKeyState(DIK_F10) == KEYSTATE::DOWN)
		m_isFree = !m_isFree;

	if (nullptr == m_pMainCamera || true == m_isFree)
	{
		m_pFreeCamera->Update(fTimeDelta);
	}
	else
	{
		m_pMainCamera->Update(fTimeDelta);
	}
}

void CCamera_Manager::Late_Update(_float fTimeDelta)
{
	if (nullptr == m_pMainCamera || true == m_isFree)
	{
		m_pFreeCamera->Late_Update(fTimeDelta);
		m_pFreeCamera->Update_Matrix();
	}
	else
	{
		m_pMainCamera->Late_Update(fTimeDelta);
		m_pMainCamera->Update_Matrix();
	}
}

HRESULT CCamera_Manager::Clear_Resource(_uint iCurrentLevelID)
{
    for (auto& Pair : m_Cameras[iCurrentLevelID])
        Safe_Release(Pair.second);
    m_Cameras[iCurrentLevelID].clear();

	Safe_Release(m_pMainCamera);
	m_pMainCamera = nullptr;

    return S_OK;
}

CCamera* CCamera_Manager::Find_Camera(_uint iLevelID, const _wstring& strCameraTag)
{
    if (m_iNumLevel <= iLevelID)
        return nullptr;

    auto iter = m_Cameras[iLevelID].find(strCameraTag);
    if (iter == m_Cameras[iLevelID].end())
        return nullptr;

    return iter->second;
}

void CCamera_Manager::Ready_FreeCamera()
{
	// Camera
	CCamera::CAMERA_DESC CameraDesc = {};
	CameraDesc.fFovy = XMConvertToRadians(60.f);
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 2000.f;
	/*CameraDesc.vEye = _float4(0.f, 200.f, -150.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 200.f, 1.f);*/
	CameraDesc.vEye = _float4(-1.019107, 5.458634, -15.936163, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	//CameraDesc.fSpeedPerSec = 1000.f;
	CameraDesc.fSpeedPerSec = 10.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.f);
	CameraDesc.fMouseSensor = 0.004f;
	
	m_pFreeCamera = CFreeCamera::Create(m_pDevice, m_pContext);
	ASSERT_CRASH(m_pFreeCamera);
	if (FAILED(m_pFreeCamera->Initialize_Clone(&CameraDesc)))
		CRASH("Free Camera");
}

CCamera_Manager* CCamera_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iNumLevel)
{
    CCamera_Manager* pInstance = new CCamera_Manager(pDevice, pContext);

    if (FAILED(pInstance->Initialize(iNumLevel)))
    {
        MSG_BOX("Failed to Create : Camera_Manager");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CCamera_Manager::Free()
{
    __super::Free();

    for (_uint i = 0; i < m_iNumLevel; ++i)
    {
        for (auto& Pair : m_Cameras[i])
            Safe_Release(Pair.second);
        m_Cameras[i].clear();
    }
    Safe_Delete_Array(m_Cameras);

    Safe_Release(m_pMainCamera);
    Safe_Release(m_pFreeCamera);

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
}

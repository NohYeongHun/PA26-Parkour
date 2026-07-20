#pragma once
#include "Base.h"

// [Component]
#include "Shader.h"
#include "DeferredShader.h"
#include "ComputeShader.h"
#include "Texture.h"
#include "Model.h"
#include "AnimationController.h"
#include "VIBuffer_Rect.h"
#include "VIBuffer_Cube.h"
#include "VIBuffer_Sphere.h"
#include "VIBuffer_Point_Instance.h"
#include "VIBuffer_Point.h"
#include "VIBuffer_FXMesh_Instance.h"
#include "VIBuffer_Mesh.h"
#include "VIBuffer_Rect_Instance.h"
#include "Transform.h"
#include "Navigation.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "Model_Instance.h"
#include "UIObject.h"
#include "InputController.h"
#include "StateMachine.h"
#include "Behavior_Tree.h"
#include "AnimMachine.h"
#include "VIBuffer_Spectrum.h"
#include "Model_Streaming.h"
#include "ModelAnim_Instance.h"
#include "Mesh_Instance_FireFly.h"
#include "Model_Instance_FireFly.h"
#include "HdrTexture.h"
#include "VAMesh.h"
// ==================

NS_BEGIN(Engine)

class CPrototype_Manager final : public CBase
{
private:
	explicit CPrototype_Manager();
	virtual ~CPrototype_Manager() = default;

public:
	HRESULT		Add_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, CBase* pPrototype);
	void		Remove_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag);
	CBase*		Clone_Prototype(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, PROTOTYPE eType, void* pArg);
	HRESULT		Clear_Resource(_uint iClearLevelID);

public:
	HRESULT		Initialize(_uint iNumLevel);

private:
	map<const _wstring, CBase*>*		m_Prototypes = { nullptr };
	typedef map<const _wstring, CBase*> PROTOTYPES;

	_uint										m_iNumLevel = {};
	mutex										m_Mutex;

public:
	static		CPrototype_Manager*	Create(_uint iNumLevel);
	virtual		void							Free() override;
};

NS_END
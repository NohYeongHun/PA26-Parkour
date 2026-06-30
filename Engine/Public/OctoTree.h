#pragma once
#include "Base.h"

NS_BEGIN(Engine)

static atomic<_int>		m_iLODCnt;

class COctoTree final : public CBase
{
private:
	explicit COctoTree();
	virtual ~COctoTree() = default;

public:
	void		SetUp_OctoTree(_float3 vCenter, _float3 vExtent);
	void		Add_To_OctoTree(class CStaticObject* pObject, const BoundingBox* pBox);
	void		Clear_OctoTree();
public:
	void		Update();

private:
	class CGameInstance*			m_pGameInstance = { nullptr };
	_uint							m_iDepth = {};

	class CCubeCell*				m_pRootCell = { nullptr };

public:
	static		COctoTree*		Create();
	virtual		void				Free() override;
};

NS_END
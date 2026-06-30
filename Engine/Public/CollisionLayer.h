#pragma once
#include "Base.h"

NS_BEGIN(Engine)

class BPLayer final : public BroadPhaseLayerInterface
{
public:
	explicit BPLayer(_uint iNumObjectLayer) {
		m_iNumObjectLayer = iNumObjectLayer;
		m_ObjectToBroadPhase = new BroadPhaseLayer[m_iNumObjectLayer];
	}
	virtual ~BPLayer() {
		Safe_Delete_Array(m_ObjectToBroadPhase);
	}

public:
	virtual _uint	GetNumBroadPhaseLayers() const override { return ENUM_CLASS(BPLAYER::END); }
	virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		ASSERT_CRASH(inLayer < m_iNumObjectLayer);
		return m_ObjectToBroadPhase[inLayer];
	}
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		switch ((BroadPhaseLayer::Type)inLayer)
		{
		case ENUM_CLASS(BPLAYER::NONE): return "NONE";
		case ENUM_CLASS(BPLAYER::NON_MOVE): return "NON_MOVE";
		case ENUM_CLASS(BPLAYER::MOVE):		return "MOVING";
		case ENUM_CLASS(BPLAYER::DEBRIS):		return "DEBRIS";
		case ENUM_CLASS(BPLAYER::SENSOR):		return "SENSOR";
		default: 
			return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	void			SetUp_ObjectToBP(_uint iObjectLayer, _uint iBPLayer)
	{
		ASSERT_CRASH(iObjectLayer < m_iNumObjectLayer && iBPLayer < ENUM_CLASS(BPLAYER::END));
		m_ObjectToBroadPhase[iObjectLayer] = BroadPhaseLayer(iBPLayer);
	}

private:
	_uint						m_iNumObjectLayer = {};
	BroadPhaseLayer*		m_ObjectToBroadPhase = { nullptr };
};

class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
	explicit ObjectLayerPairFilterImpl(_uint iNumObjectLayer) {
		m_iNumObjectLayer = iNumObjectLayer;
		m_ObjectLayerFilter = new _bool*[m_iNumObjectLayer];
		for(_uint i = 0; i < m_iNumObjectLayer; ++i)
			m_ObjectLayerFilter[i] = new _bool[m_iNumObjectLayer]{ false };
	}
	virtual ~ObjectLayerPairFilterImpl() {
		for (_uint i = 0; i < m_iNumObjectLayer; ++i)
			Safe_Delete_Array(m_ObjectLayerFilter[i]);
		Safe_Delete_Array(m_ObjectLayerFilter);
	}

public:
	virtual _bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		ASSERT_CRASH(inObject1 < m_iNumObjectLayer && inObject2 < m_iNumObjectLayer);
		return m_ObjectLayerFilter[inObject1][inObject2];
	}

	void			SetUp_ObjectFilter(_uint iSrc, _uint iDst)
	{
		ASSERT_CRASH(iSrc < m_iNumObjectLayer && iDst < m_iNumObjectLayer);
		m_ObjectLayerFilter[iSrc][iDst] = true;
		m_ObjectLayerFilter[iDst][iSrc] = true;
	}

private:
	_uint						m_iNumObjectLayer = {};
	_bool**					m_ObjectLayerFilter = { nullptr };
};

class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
	explicit ObjectVsBroadPhaseLayerFilterImpl(_uint iNumObjectLayer) {
		m_iNumObjectLayer = iNumObjectLayer;
		m_ObjectVsBPLayerFilter = new _bool * [m_iNumObjectLayer];
		for (_uint i = 0; i < m_iNumObjectLayer; ++i)
			m_ObjectVsBPLayerFilter[i] = new _bool[ENUM_CLASS(BPLAYER::END)]{ false };
	}
	virtual ~ObjectVsBroadPhaseLayerFilterImpl() {
		for (_uint i = 0; i < m_iNumObjectLayer; ++i)
			Safe_Delete_Array(m_ObjectVsBPLayerFilter[i]);
		Safe_Delete_Array(m_ObjectVsBPLayerFilter);
	}

public:
	virtual _bool					ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
	{
		ASSERT_CRASH(inLayer1 < m_iNumObjectLayer && inLayer2.GetValue() < ENUM_CLASS(BPLAYER::END));
		return m_ObjectVsBPLayerFilter[inLayer1][inLayer2.GetValue()];
	}

	void			SetUp_ObjectVsBPFilter(_uint iObjectLayer, _uint iBPLayer)
	{
		ASSERT_CRASH(iObjectLayer < m_iNumObjectLayer && iBPLayer < ENUM_CLASS(BPLAYER::END));
		m_ObjectVsBPLayerFilter[iObjectLayer][iBPLayer] = true;
	}

private:
	_uint						m_iNumObjectLayer = {};
	_bool**					m_ObjectVsBPLayerFilter = { nullptr };
};

NS_END
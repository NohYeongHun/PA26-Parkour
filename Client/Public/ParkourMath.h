#pragma once
#include "Client_Define.h"

NS_BEGIN(Client)
namespace ParkourMath
{
	inline _vector Calc_WallTiltAxisLocal(_fvector vWallNormal, _fvector vWorldUp, _matrix WorldMatrix)
	{
		_vector vAxisWorld = XMVector3Normalize(XMVector3Cross(XMVectorNegate(vWallNormal), vWorldUp));
		_matrix WorldInv = XMMatrixInverse(nullptr, WorldMatrix);
		return XMVector3Normalize(XMVector3TransformNormal(vAxisWorld, WorldInv));
	}

	inline _vector Calc_HangPos(_fvector vGrabEdge, _fvector vWallNormal,
	                            _float fRadius, _float fHeight,
	                            _float fHangOffsetMult, _float fWallOffset)
	{
		_vector vPos = vGrabEdge + vWallNormal * (fRadius + fWallOffset);
		return XMVectorSetW(XMVectorSetY(vPos, XMVectorGetY(vGrabEdge) - fHeight * fHangOffsetMult), 1.f);
	}

	inline _vector Calc_FacingYawQuat(_fvector vFrontNormal, _fvector vFallbackLook)
	{
		_vector vFacing = XMVectorNegate(XMVectorSetY(vFrontNormal, 0.f));
		if (XMVectorGetX(XMVector3LengthSq(vFacing)) < 1e-4f)
			vFacing = vFallbackLook;

		vFacing = XMVector3Normalize(vFacing);
		_float fYaw = atan2f(XMVectorGetX(vFacing), XMVectorGetZ(vFacing));
		return XMQuaternionRotationRollPitchYaw(0.f, fYaw, 0.f);
	}
}
NS_END

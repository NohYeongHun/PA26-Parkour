#pragma once
#include "Client_Struct.h"

NS_BEGIN(Client)
VAULT_CURVE BuildCurve(const VAULT_TARGET& Target, _float fHeight = 0.5f)
{
	VAULT_CURVE Curve;

	Curve.P0 = Target.vStartPos;

	Curve.P2 = Target.vEndPos;

	Curve.P1 = Target.vObstacleTop;
	Curve.P1.y += fHeight;

	return Curve;
}


NS_END
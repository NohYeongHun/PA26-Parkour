#pragma once
#ifndef Engine_Function_h__
#define Engine_Function_h__

#include "Engine_Typedef.h"

namespace Engine
{
	template<typename T>
	void	Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void	Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete [] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	unsigned long Safe_Release(T& pInstance)
	{
		unsigned long		dwRefCnt = 0;

		if (nullptr != pInstance)
		{
			dwRefCnt = pInstance->Release();

			if (0 == dwRefCnt)
				pInstance = nullptr;
		}

		return dwRefCnt;
	}

	template<typename T>
	unsigned long Safe_AddRef(T& pInstance)
	{
		unsigned long		dwRefCnt = 0;

		if (nullptr != pInstance)
		{
			dwRefCnt = pInstance->AddRef();
		}

		return dwRefCnt;
	}

	inline wstring StringToWString(const string& str)
	{
		int iLength = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
		if (0 == iLength)
			return L"";

		wstring wstr(iLength, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], iLength);

		if (!wstr.empty() && wstr.back() == L'\0')
			wstr.pop_back();
		return wstr;
	}

	inline string WStringToString(const wstring& wstr)
	{
		if (wstr.empty())
			return "";

		int iLength = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (0 == iLength)
			return "";

		string str(iLength - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], iLength, nullptr, nullptr);

		return str;
	}

	inline XMFLOAT4 ComputeAtlasUV(float fTextureSizeX, float fTextureSizeY, float fLeft, float fRight, float Top, float Bottom)
	{
		float fStartU = ( fLeft ) / ( fTextureSizeX );
		float fStartV = ( Top ) / ( fTextureSizeY );
		float fEndU = ( fRight ) / ( fTextureSizeY );
		float fEndV = ( Bottom ) / ( fTextureSizeY );

		return XMFLOAT4(fStartU, fStartV, fEndU, fEndV);
	}

	inline _float floatlerp(_float fCur, _float fMax, _float fRatio)
	{
		return Clamp((fCur * (1 - fRatio) + fMax * fRatio), 0.f, 1.f);
	}

	inline _float lerp(_float fSrc, _float fDst, _float fRatio)
	{
		return (fSrc * (1.f - fRatio)) + (fDst * fRatio);
	}

	inline _float SmoothStep(_float fMin, _float fMax, _float fValue)
	{
		_float t = Clamp((fValue - fMin) / (fMax - fMin), 0.f, 1.f);
		return t * t * (3.f - 2.f * t);
	}

	inline _float Saturate(_float fValue)
	{
		return max(min(fValue, 1.f), 0.f);
	}

#ifdef _DEBUG
	inline void OutPutDebugFloat4(_wstring strPrePix, _float4 fVector)
	{
		_wstring strDebug = strPrePix + L" : " +  to_wstring(fVector.x) + L", " + to_wstring(fVector.y) + L", " + to_wstring(fVector.z) + L", " + to_wstring(fVector.w) + L"\n";
		OutputDebugString(strDebug.c_str());
	}

	inline void OutPutDebugFloat(_wstring strPrePix, _float fValue)
	{
		_wstring strDebug = strPrePix + L" : " + to_wstring(fValue) + L"\n";
		OutputDebugString(strDebug.c_str());
	}

	inline void OutPutDebugMatrix(_wstring strPrePix, const _float4x4& mat)
	{
		_wstring strDebug = strPrePix + L" : " + L"\n";
		OutputDebugString(strDebug.c_str());

		_float4 fValue = {};
		memcpy(&fValue, mat.m[0], sizeof(_float4));
		OutPutDebugFloat4(TEXT("Right"), fValue);
		memcpy(&fValue, mat.m[1], sizeof(_float4));
		OutPutDebugFloat4(TEXT("Up"), fValue);
		memcpy(&fValue, mat.m[2], sizeof(_float4));
		OutPutDebugFloat4(TEXT("Look"), fValue);
		memcpy(&fValue, mat.m[3], sizeof(_float4));
		OutPutDebugFloat4(TEXT("Position"), fValue);

		OutputDebugString(TEXT("\n"));
		
	}

	
#endif

	inline Vec3 LoadVec3(const _float3& vVector) { return Vec3(vVector.x, vVector.y, vVector.z); }
	inline Vec3 LoadVec3(const _fvector& vVector) { return Vec3(vVector.m128_f32[0], vVector.m128_f32[1], vVector.m128_f32[2]); }
	inline _float3 StoreFloat3(const Vec3& vVector) { return _float3(vVector.GetX(), vVector.GetY(), vVector.GetZ()); }
	inline _vector StoreVector3(const Vec3& vVector) { return XMVectorSet(vVector.GetX(), vVector.GetY(), vVector.GetZ(), 0.f); }
	inline Quat LoadQuat(const _float4& vQuat) { return Quat(vQuat.x, vQuat.y, vQuat.z, vQuat.w); }
	inline Quat LoadQuat(const _fvector& vQuat) { return Quat(vQuat.m128_f32[0], vQuat.m128_f32[1], vQuat.m128_f32[2], vQuat.m128_f32[3]); }
	inline _float4 StoreQuat(const Quat& vQuat) { return _float4(vQuat.GetX(), vQuat.GetY(), vQuat.GetZ(), vQuat.GetW()); }
	
	// BoundingBox Local -> World ?숆린??
	inline void Sync_BoundingBox(BoundingBox* pBox, _fmatrix WorldMatrix) { pBox->Transform(*pBox, WorldMatrix); }
}

#endif // Engine_Function_h__

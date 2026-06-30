#pragma once
#include "EditorPch.h"

typedef struct tagChangeLevel : public CEvent
{
	LEVEL eNextLevel;
	_bool isLoad;
	tagChangeLevel(LEVEL _eNextLevel, _bool _isLoad)
		: eNextLevel{ _eNextLevel }, isLoad{ _isLoad } {};
}CHANGE_LEVEL_EVENT;

typedef struct tagLoadingEnd : public CEvent
{
	_bool isFinish;
	tagLoadingEnd(_bool _isFinish) : isFinish{ _isFinish } {};
}LOADING_END_EVENT;

typedef struct tagMapObjectPick : public CEvent
{
	void* pObject = { nullptr };
	_float fDistance;
	tagMapObjectPick(void* _pObject, _float _fDistance) : pObject(_pObject), fDistance(_fDistance) {};
}MAP_PICK;

typedef struct tagMapCreate: public CEvent
{
	_char ModelName[MAX_PATH];
	void* pObject = { nullptr };
	tagMapCreate(_char* _ModelName, void* _pObject) : pObject(_pObject)
	{
		strcpy_s(ModelName, _ModelName);
	};
}MAP_CREATE;

typedef struct tagMapSave : public CEvent
{
	ofstream& File;
	unordered_set<_string>& ModelName;
	tagMapSave(ofstream& _File, unordered_set<_string>& _ModelName) :File(_File), ModelName(_ModelName) {};
}MAP_SAVE;

typedef struct tagMapInstanceCreate : public CEvent
{
	_char ModelName[MAX_PATH];
	_uint iNumSaveIndex;
	void* pObject = { nullptr };
	tagMapInstanceCreate(_char* _ModelName, _uint _iNumSaveIndex, void* _pObject) :iNumSaveIndex(_iNumSaveIndex), pObject(_pObject)
	{
		strcpy_s(ModelName, _ModelName);
	};
}INSTANCE_CREATE;


typedef struct tagMapInstanceSave : public CEvent
{
	vector<_float4x4>& Totalmatrix;
	vector<_vector>& Objectmatrix;
	_uint* iNumTotalInstance = {};

	tagMapInstanceSave(vector<_float4x4>& _Totalmatrix, vector<_vector>& _Objectmatrix, _uint* _iNumTotalInstance) : Totalmatrix(_Totalmatrix), 
		Objectmatrix(_Objectmatrix), iNumTotalInstance(_iNumTotalInstance) { };

}INSTANCE_SAVE;

typedef struct tagMapBound : public CEvent
{
	_vector* vMin;
	_vector* vMax;
	tagMapBound(_vector* _vMin, _vector* _vMax) : vMin(_vMin),
		vMax(_vMax){
	};

}MAP_BOUND;

typedef struct tagMapLightCreate : public CEvent
{
	_uint iNumCreateIndex;
	void* pObject = { nullptr };
	tagMapLightCreate(_uint _iNumCreateIndex, void* _pObject) :iNumCreateIndex(_iNumCreateIndex), pObject(_pObject) {};
}LIGHT_CREATE;

typedef struct tagMapFireFly: public CEvent
{
	void* pObject = { nullptr };
	tagMapFireFly(void* _pObject) :pObject(_pObject) {};
}FLY;


#ifndef Engine_Typedef_h__
#define Engine_Typedef_h__

namespace Engine
{
	typedef		bool							_bool;

	typedef		signed char					_byte;
	typedef		unsigned char				_ubyte;
	typedef		char							_char;

	typedef		wchar_t						_tchar;
	typedef		string							_string;
	typedef		wstring						_wstring;
	
	typedef		signed short				_short;
	typedef		unsigned short				_ushort;

	typedef		signed int					_int;
	typedef		unsigned int				_uint;

	typedef		signed long					_long;
	typedef		unsigned long				_ulong;

	typedef		float							_float;
	typedef		double						_double;

	typedef		XMUINT4					_uint4;

	typedef		XMFLOAT2					_float2;
	typedef		XMFLOAT3					_float3;
	typedef		XMFLOAT4					_float4;

	typedef		XMFLOAT4X4				_float4x4;

	typedef		XMVECTOR					_vector;
	typedef		FXMVECTOR				_fvector;		// 1~3
	typedef		GXMVECTOR				_gvector;		// 4
	typedef		HXMVECTOR				_hvector;		// 5~6
	typedef		CXMVECTOR				_cvector;		// 7~

	typedef		XMMATRIX					_matrix;
	typedef		FXMMATRIX					_fmatrix;		// 1
	typedef		CXMMATRIX				_cmatrix;		// 2~
}

#endif // Engine_Typedef_h__

// Font Manager Experiment Cpp(1)

#include "EnginePch.h"
#include "Font_Manager.h"

#include "Shader.h"
#include "CustomFont.h"
#include "GameInstance.h"

#define KSTA_DEBUGATLASTEST
#define KSTA_FONTSCREEN_TO3D

CFont_Manager::CFont_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::GetInstance() }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CFont_Manager::Initialize(_uint iWinSizeX, _uint iWinSizeY)
{
	if (FT_Init_FreeType(&m_pFTLibrary))
		CRASH("FT Library");

	//Ready_FontBuffer();

	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	return S_OK;
}

HRESULT CFont_Manager::Add_Font(const _wstring& strFontTag, const _char* pFilePath, const _uint iPixelHeight, _uint iPadding)
{
	if (nullptr != Find_Font(strFontTag))
		return E_FAIL;

	// load for freetype

	//FT_Face pFont = { nullptr };
	FTCUSTOM_FONT* pFontInfo = new FTCUSTOM_FONT();

	if (FT_New_Face(m_pFTLibrary, pFilePath, 0, &pFontInfo->pFace))
		CRASH("Add Font");

	pFontInfo->iPadding = iPadding;
	Load_Font(pFontInfo, pFilePath, iPixelHeight);	// height : font height
	m_Fonts.emplace(strFontTag, pFontInfo);

	return S_OK;
}

HRESULT CFont_Manager::Load_Font(FTCUSTOM_FONT* pFontInfo, const _char* pFilePath, _uint iPixelHeight)
{
	FT_Select_Charmap(pFontInfo->pFace, FT_ENCODING_UNICODE);	// 유니코드 문자맵
	FT_Set_Pixel_Sizes(pFontInfo->pFace, 0, iPixelHeight);

	pFontInfo->iPixelHeight = iPixelHeight;
	pFontInfo->isHasKerning = FT_HAS_KERNING(pFontInfo->pFace) ? true : false;

	Create_EmptyAtlas(pFontInfo);

	return S_OK;
}

_bool CFont_Manager::Reset_AtlasTexture(FTCUSTOM_FONT* pFont, _uint newW, _uint newH)
{
	Safe_Release(pFont->pSampler);
	Safe_Release(pFont->pAtlasSRV);
	Safe_Release(pFont->pAtlasTex);

	return SUCCEEDED(Create_EmptyAtlas(pFont, newW, newH));
}

HRESULT CFont_Manager::Create_EmptyAtlas(FTCUSTOM_FONT* pFontInfo, _uint iAtlasW, _uint iAtlasH)
	{
	// 입력받은 크기에 맞게 빈 아틀라스를 만듭니다.

	pFontInfo->iAtlasW = iAtlasW;
	pFontInfo->iAtlasH = iAtlasH;
	pFontInfo->iPenX = pFontInfo->iPenY = pFontInfo->iRowH = 0;

	D3D11_TEXTURE2D_DESC td = {};
	td.Width				= iAtlasW;
	td.Height				= iAtlasH;
	td.MipLevels			= 1; td.ArraySize = 1;
	td.Format				= DXGI_FORMAT_R8_UNORM;
	td.SampleDesc.Count		= 1;
	td.Usage				= D3D11_USAGE_DEFAULT;
	td.BindFlags			= D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = m_pDevice->CreateTexture2D(&td, nullptr, &pFontInfo->pAtlasTex);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
	sd.Format				= td.Format;
	sd.ViewDimension		= D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Texture2D.MipLevels	= 1;

	hr = m_pDevice->CreateShaderResourceView(pFontInfo->pAtlasTex, &sd, &pFontInfo->pAtlasSRV);
	if (FAILED(hr))
	{
		Safe_Release(pFontInfo->pAtlasTex);
		return hr;
	}

	D3D11_SAMPLER_DESC smp = {};
	smp.Filter				= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	smp.AddressU = smp.AddressV = smp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	hr = m_pDevice->CreateSamplerState(&smp, &pFontInfo->pSampler);
	if (FAILED(hr))
	{
		Safe_Release(pFontInfo->pSampler);
		Safe_Release(pFontInfo->pAtlasSRV);
		Safe_Release(pFontInfo->pAtlasTex);
		return hr;
	}

	return S_OK;
}

_bool CFont_Manager::Rebuild_Atlas(FTCUSTOM_FONT* pFontInfo, _uint iAtlasW, _uint iAtlasH)
{
	if (!Reset_AtlasTexture(pFontInfo, iAtlasW, iAtlasH)) return false;

	pFontInfo->iPenX = pFontInfo->iPenY = pFontInfo->iRowH = 0;

	// 기존 코드포인트 다시 베이크
	auto codes = vector<_uint>();
	codes.reserve(pFontInfo->mapGlyphs.size());
	for (auto& kv : pFontInfo->mapGlyphs) codes.push_back(kv.first);
	pFontInfo->mapGlyphs.clear();

	for (auto code : codes)
		if (!BakeOneGlyph(pFontInfo, code)) return false;

	return true;
}

_bool CFont_Manager::FT_RenderGlyph(FT_Face face, _uint iCodePoint, FT_GlyphSlot& outSlot)
{
	// FreeType이 글자를 그레이스케일 비트맵으로 변환.
	if (FT_Load_Char(face, iCodePoint, FT_LOAD_RENDER))
		return false;

	if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF))
		return false;

	outSlot = face->glyph;
	return true;
}

_bool CFont_Manager::Atlas_AllocRect(FTCUSTOM_FONT* pFontInfo, _int iGlyphWidth, _int iGlyphHeight, _int& outX, _int& outY)
{
	// 생성했던 아틀라스에 빈 영역을 할당하여, 글리프를 담을 공간을 인자로 내보냅니다.
	// 
	// 1. 폰트 정보와 폰트가 차지할 크기의 글리프 크기를 받아와,
	// 2. 빈 공간 또는 당장 사용하지 않을 공간을 찾아내서,
	// 3. 해당 공간을 사용할 공간으로써 내보냄.
	//
	// 
	// 공간을 찾는 기준?
	// 
	// 매 호출마다 오른쪽, 아래로 인덱스를 넘기며 글리프를 채우고
	// 마지막에 도달하여 더이상 공간이 없을 경우 false를 떨궈, 해당 함수가 쓰이는 곳에서 리빌드를 하게 됨


	// 공백문자와 같이, 할당할 공간이 필요 없는 경우에는 무시합니다.
	if (iGlyphWidth <= 0 || iGlyphHeight <= 0)
	{
		outX = outY = 0;
		return true;
	}

	// 아틀라스의 현재 행에 공간이 없으면 줄바꿈을 시도합니다.
	if (pFontInfo->iPenX + iGlyphWidth > pFontInfo->iAtlasW)
	{
		pFontInfo->iPenX = 0;
		pFontInfo->iPenY += pFontInfo->iRowH + 1;
		pFontInfo->iRowH = 0;
	}

	// 아틀라스 공간이 부족한 경우
	if (pFontInfo->iPenY + iGlyphHeight > pFontInfo->iAtlasH)
		return false; // 나중에 리빌드 (더 큰 아틀라스 이미지 사용)

	outX = pFontInfo->iPenX;
	outY = pFontInfo->iPenY;

	pFontInfo->iPenX += iGlyphWidth + 1;
	if (iGlyphHeight > pFontInfo->iRowH)
		pFontInfo->iRowH = iGlyphHeight;

	return true;
}

_bool CFont_Manager::Atlas_CheckSize(FTCUSTOM_FONT* pFontInfo, _int gw, _int gh, _int& outX, _int& outY)
{
	// 먼저 시도
	if (Atlas_AllocRect(pFontInfo, gw, gh, outX, outY))
		return true;
	
	// 리빌드
	_uint newW = pFontInfo->iAtlasW;
	_uint newH = pFontInfo->iAtlasH;

	newW = max(newW * 2, (_uint)(pFontInfo->iAtlasW + gw + 8));
	newH = max(newH * 2, (_uint)(pFontInfo->iAtlasH + gh + 8));

	const _uint MAX_ATLAS = 4096; // 상한
	newW = min(newW, MAX_ATLAS);
	newH = min(newH, MAX_ATLAS);

	if (!Rebuild_Atlas(pFontInfo, newW, newH))
		return false;

	// 리빌드 후 다시 시도
	return Atlas_AllocRect(pFontInfo, gw, gh, outX, outY);
}

_bool CFont_Manager::Atlas_UploadBitmap(FTCUSTOM_FONT& Font, _int x, _int y, _int w, _int h, const uint8_t* pSrc, _int srcPitch)
{
	// 얻은 비트맵 데이터를 실제 GPU 텍스쳐로 복사합니다.
	if (w <= 0 || h <= 0)
		return true;

	// 기본 유효성 검사
	if (!m_pContext || !Font.pAtlasTex || !pSrc)
		return false;

	if (x < 0 || y < 0 || x + w > Font.iAtlasW || y + h > Font.iAtlasH)
		return false;

	const UINT bytesPerPixel = 1; // DXGI_FORMAT_R8_UNORM
	const int absPitch = srcPitch < 0 ? -srcPitch : srcPitch;

	// 버퍼가 최소 한 행을 담을 수 있는지 확인
	if (absPitch < static_cast<int>(w * bytesPerPixel))
		return false;

	for (int row = 0; row < h; ++row)
	{
		// FreeType은 pitch가 음수일 수 있음(하단부터 위로 저장)
		const uint8_t* pRow = nullptr;
		if (srcPitch >= 0)
		{
			pRow = pSrc + static_cast<size_t>(row) * absPitch;
		}
		else
		{
			// 음수 pitch면 버퍼의 마지막 행에서 역방향으로 읽어야 함
			pRow = pSrc + static_cast<size_t>(h - 1 - row) * absPitch;
		}

		D3D11_BOX box = { (UINT)x, (UINT)(y + row), 0, (UINT)(x + w), (UINT)(y + row + 1), 1 };
		m_pContext->UpdateSubresource(Font.pAtlasTex, 0, &box, pRow, static_cast<UINT>(absPitch), 0);
	}
	return true;
}

_bool CFont_Manager::BakeOneGlyph(FTCUSTOM_FONT* pFontInfo, _uint iCodePoint)
{
	unordered_map<_uint, FTCUSTOM_FONT_GLYPH>& vecGlyphMap = pFontInfo->mapGlyphs;

	// 이미 존재하면 스킵
	if (vecGlyphMap.find(iCodePoint) != vecGlyphMap.end())
		return true;

	FT_GlyphSlot slot = nullptr;
	if (!FT_RenderGlyph(pFontInfo->pFace, iCodePoint, slot))
		return false;

	FT_Bitmap& bmp = slot->bitmap;
	int gw = (int)bmp.width;
	int gh = (int)bmp.rows;
	const _int iPadding =  static_cast<_int>(pFontInfo->iPadding);
	int paddedW = gw + iPadding * 2;
	int paddedH = gh + iPadding * 2;

	int x, y;
	if (!Atlas_CheckSize(pFontInfo, paddedW, paddedH, x, y))
		return false; // 공간 부족

	if (gw > 0 && gh > 0)
		Atlas_UploadBitmap(*pFontInfo, x + iPadding, y + iPadding, gw, gh, bmp.buffer, bmp.pitch);

	FTCUSTOM_FONT_GLYPH tFontGlyph = {};
	tFontGlyph.iCodepoint = iCodePoint;
	tFontGlyph.sOffsetX = (_short)slot->bitmap_left;   // bearing X
	tFontGlyph.sOffsetY = (_short)slot->bitmap_top;    // bearing Y
	tFontGlyph.sWidth = (_short)(gw + iPadding * 2);
	tFontGlyph.sHeight = (_short)(gh + iPadding * 2);
	tFontGlyph.sAdvance = (_short)(slot->advance.x >> 6); // 픽셀 단위 advance

	tFontGlyph.fU0 = float(x) / pFontInfo->iAtlasW;
	tFontGlyph.fV0 = float(y) / pFontInfo->iAtlasH;
	tFontGlyph.fU1 = float(x + iPadding * 2 + gw) / pFontInfo->iAtlasW;
	tFontGlyph.fV1 = float(y + iPadding * 2 + gh) / pFontInfo->iAtlasH;

	vecGlyphMap[iCodePoint] = tFontGlyph;
	return true;
}

FTCUSTOM_FONT* CFont_Manager::Find_Font(const _wstring& strFontTag)
{
	auto iter = m_Fonts.find(strFontTag);

	if (iter == m_Fonts.end())
		return nullptr;

	return iter->second;
}

const FTCUSTOM_FONT_GLYPH* CFont_Manager::Get_Glyph(const _wstring& tag, _uint code)
{
	FTCUSTOM_FONT* pFont = Find_Font(tag);
	if (!pFont) return nullptr;

	// 없으면 즉시 베이크 시도
	if (pFont->mapGlyphs.find(code) == pFont->mapGlyphs.end())
	{
		if (!BakeOneGlyph(pFont, code))
			return nullptr;
	}
	return &pFont->mapGlyphs[code];
}


ID3D11ShaderResourceView* CFont_Manager::Get_AtlasSRV(const _wstring& tag)
{
	FTCUSTOM_FONT* pFont = Find_Font(tag);
	if (!pFont)
		return nullptr;

	return pFont->pAtlasSRV;
}

const FTCUSTOM_FONT_GLYPH* CFont_Manager::Get_GlyphAndAdvance(const _wstring& fontTag, _uint codePoint, _uint prevCodePoint, _int& outAdvanceX)
{
	FTCUSTOM_FONT* pFont = Find_Font(fontTag);
	if (!pFont) return nullptr;

	// 글리프가 없다면 atlas에 추가 (자동)
	if (BakeOneGlyph(pFont, codePoint) == false)
		return nullptr;

	auto iter = pFont->mapGlyphs.find(codePoint);
	if (iter == pFont->mapGlyphs.end())
		return nullptr;

	const FTCUSTOM_FONT_GLYPH* glyph = &iter->second;

	// 기본 advance 설정
	outAdvanceX = glyph->sAdvance;

	// 커닝 적용 (이제 엔진에서만 FreeType 사용)
	if (pFont->isHasKerning && prevCodePoint != 0)
	{
		FT_Vector kerning = {};
		FT_Get_Kerning(
			pFont->pFace,
			FT_Get_Char_Index(pFont->pFace, prevCodePoint),
			FT_Get_Char_Index(pFont->pFace, codePoint),
			FT_KERNING_DEFAULT,
			&kerning
		);

		outAdvanceX += (kerning.x >> 6);
	}

	return glyph;
}

CFont_Manager* CFont_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWinSizeX, _uint iWinSizeY)
{
	CFont_Manager* pInstance = new CFont_Manager(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iWinSizeX, iWinSizeY)))
		CRASH("FontManager");

	return pInstance;
}

void CFont_Manager::Free()
{
	__super::Free();

	for (auto& [tag, face] : m_Fonts)
	{
		if (face->pSampler)			face->pSampler->Release();
		if (face->pAtlasSRV)		face->pAtlasSRV->Release();
		if (face->pAtlasTex)		face->pAtlasTex->Release();

		FT_Done_Face(face->pFace);
		Safe_Delete(face);
		face = nullptr;
	}

	m_Fonts.clear();

	if (m_pFTLibrary)
	{
		FT_Done_FreeType(m_pFTLibrary);
		m_pFTLibrary = nullptr;
	}

	//Safe_Release(m_pFontVertexBuffer);

	//Safe_Release(m_pShaderCom);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

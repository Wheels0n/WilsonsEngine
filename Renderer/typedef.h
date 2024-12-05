#pragma once
#include <Windows.h>
#include <wrl.h>
#include <cwchar>
#include <cmath>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <mmsyscom.h>
#include <random>
#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <process.h>
#include <pix.h>
#include <DirectXMath.h>
#include <fbxsdk.h>

#define CUSTUM_ALIGN(LEN, ALIGN) (LEN+ALIGN-1u)&~(ALIGN-1u)
constexpr UINT _4KB = 4 * 1024;
constexpr UINT _64KB = 64 * 1024;
constexpr UINT _CB_HEAP_SIZE = 256ull * 1024 * 1024;
constexpr UINT _VB_HEAP_SIZE = 1024ull * 1024 * 1024;
constexpr UINT _IB_HEAP_SIZE = 1024ull * 1024 * 1024;
constexpr UINT _TEX_HEAP_SIZE = 3 * 1024ull * 1024 * 1024;
constexpr UINT _CBV_READ_SIZE = 256;
constexpr UINT _HEAP_BLOCK_COUNT = 8;
constexpr INT _UNSELECT = -1;
namespace wilson
{	 
	struct Tri
	{
		int v[3];
		int ref=0;
		bool operator<(const Tri& a)
		{
			for (int i = 0; i < 3; ++i)
			{
				if (this->v[i] != a.v[i])
				{
					return this->v[i] < a.v[i];
				}
			}
			return false;
		}
	};
	struct TriCmp
	{
		bool operator()(const std::pair<Tri, int>& a, const std::pair<Tri, int>& b)
		{
			return a.first.ref < b.first.ref;
		}
	};

	//Cam
	constexpr float _RAD = 6.28319;

	//Frustum11
	struct Plane
	{
		DirectX::XMVECTOR norm;
		float d;
	};

	//DirLight
	struct DirLightProperty
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;
		DirectX::XMFLOAT3 direction;
		float pad;
	};

	//PntLight
	struct CubeLightProperty
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;

		DirectX::XMFLOAT3 position;
		float range;

		DirectX::XMFLOAT3 attenuation;
		float pad;
	};

	//SptLight
	constexpr float _S_NEAR = 1.0f;
	constexpr float _S_FAR = 150.0f;
	struct SpotLightProperty
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;

		DirectX::XMFLOAT3 position;
		float range;

		DirectX::XMFLOAT3 direction;
		float cutoff;

		DirectX::XMFLOAT3 attenuation;
		float outerCutoff;
	};
	//Light11
	enum class eLIGHT_TYPE
	{
		DIR,
		CUBE,
		SPT
	};

	//LightBuffer11
	constexpr UINT _MAX_DIR_LIGHTS = 5;
	constexpr UINT _MAX_PNT_LIGHTS = 24;
	constexpr UINT _MAX_SPT_LIGHTS = 10;
	constexpr UINT _CASCADE_LEVELS = 5;

	struct DirLightMatrices
	{
		DirectX::XMMATRIX matrices[_MAX_DIR_LIGHTS][_CASCADE_LEVELS];
		UINT dirCnt;
	};
	struct SpotLightMatrices
	{
		DirectX::XMMATRIX matrices[_MAX_SPT_LIGHTS];
		UINT spotCnt;
	};
	struct LightBufferProperty
	{
		DirLightProperty dirLights[_MAX_DIR_LIGHTS];
		UINT dirCnt;
		CubeLightProperty pntLights[_MAX_PNT_LIGHTS];
		UINT pntCnt;
		SpotLightProperty  sptLights[_MAX_SPT_LIGHTS];
		UINT sptCnt;
		UINT padding;
	};

	//MatrixBuffer
	struct MatrixBuffer
	{
		DirectX::XMMATRIX worldMat;
		DirectX::XMMATRIX viewMat;
		DirectX::XMMATRIX invWorldMat;
		DirectX::XMMATRIX invWVMat;
		DirectX::XMMATRIX wvpTransposedMat;
	};

	//Mesh
	enum class eTexType
	{
		diffuse,
		normal,
		specular,
		emissive,
		alpha,
		cnt
	};
	enum class eFileType
	{
		OBJ,
		FBX
	};
	struct PerModel
	{
		BOOL hasNormal;
		BOOL hasSpecular;
		BOOL hasEmissive;
		BOOL hasAlpha;
	};
	struct VertexData
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT3 tangent;
	};

	struct Material
	{
		DirectX::XMVECTOR ambient;
		DirectX::XMVECTOR diffuse;
		DirectX::XMVECTOR specular;//w component is SpecPower
		DirectX::XMVECTOR reflect;
	};
	struct MaterialInfo
	{
		std::string diffuseMap;
		std::string specularMap;
		std::string normalMap;
		std::string alphaMap;
		std::string emissiveMap;
		Material material;
	};
	constexpr UINT _MAX_INSTANCES = 50;
	
	//Shader12
	enum class eZpassRP
	{
		vsMat,
		psDiffuse,
		psWrap,
		cnt
	};

	enum class eDownSampleRP
	{
		depthMap,
		psSampler,
		cnt
	};
	enum class eGenHiZRP
	{
		psLastResoltion,
		psLastMip,
		psSampler,
		cnt
	};

	enum class eHiZCullRP
	{
		csHiZ,
		csDst,
		csResolution,
		csBound,
		csMatrix,
		csBorder,
		cnt

	};
	
	enum class eCascadeShadowRP
	{
		vsMat,
		gsMat,
		cnt
	};

	enum class eSpotShadowRP
	{
		vsMat,
		psDiffuseMap,
		psSampler,
		cnt
	};

	enum class eCubeShadowRP
	{
		vsMat,
		gsMat,
		psLightPos,
		psDiffuseMap,
		psSampler,
		cnt
	};

	enum class eSkyboxRP
	{
		vsMat,
		psDiffuseMap,
		psSampler,
		cnt
	};

	enum class ePbrGeoRP
	{
		vsMat,
		psDiffuse,
		psNormal,
		psSpecular,
		psEmissive,
		psAlpha,
		psSampler,
		psCamPos,
		psHeightScale,
		cnt
	};

	enum class eSsaoRP
	{
		csNoise,
		csVpos,
		csVnomral,
		csUAV,
		csWrap,
		csClamp,
		csSamplePoints,
		csProj,
		csParameters,
		cnt
	};

	enum class eSsaoBlurRP
	{
		csDst,
		csDebug,
		csSsao,
		csWrap,
		cnt
	};

	enum class ePbrLightRP
	{
		psPos,
		psNormal,
		psAlbedo,
		psSpecular,
		psEmissive,
		psAO,
		psIrradiance,
		psPrefilter,
		psBrdf,
		psDirShadow,
		psSpotShadow,
		psCubeShadow,
		psWrap,
		psCubeShadowSampler,
		psShadowSampler,
		psCamPos,
		psCasCadeLevels,
		psProjMat,
		psViewMat,
		psLight,
		psDirLitMat,
		psSpotLitMat,
		cnt
	};

	enum class eOutlinerRP
	{	
		vsMat,
		psAlbedo,
		psWrap,
		cnt
	};

	enum class ePostProcessRP
	{
		csTex,
		csUav,
		csSampler,
		csExposure,
		cnt
	};

	enum class eDiffuseIrraidianceRP
	{	
		gsCb,
		psTex,
		psSampler,
		cnt
	};

	enum class ePrefilterRP
	{	
		gsCb,
		psTex,
		psSampler,
		psCb,
		cnt
	};

	enum class eEquirect2CubeRP
	{
		gsCb,
		psTex,
		psSampler,
		cnt
	};

	enum class eGenMipRP
	{
		csTex,
		csUAV,
		csSampler,
		csCb,
		cnt
	};
	//Engine
	static constexpr float _DRAG_THRESHOLD = 6.0f;
	static bool g_bShutdown = false;

	//Importer11
	enum class eTEX
	{
		Kd,
		Ks,
		Bump,
		d
	};

	//Viewport11
	static const char* g_types[] =
	{
		"obj", "fbx", "dir", "pnt", "spt"
	};

	constexpr UINT _VIEWPORT_WIDTH = 1024;
	constexpr UINT _VIEWPORT_HEIGHT = 720;

	//DirectX
	constexpr UINT _WORKER_THREAD_COUNT= 4;
	constexpr UINT _OBJECT_PER_THREAD = 100;
	constexpr float _REFRESH_RATE = 75.f;
	constexpr UINT  _BUFFER_COUNT = 2;
	constexpr UINT _SHADOWMAP_SIZE = 512;
	constexpr float _CUBE_SIZE = 0.25;
	constexpr UINT  _KERNEL_COUNT = 64;
	constexpr UINT  _NOISE_VEC_COUNT = 16;
	constexpr UINT  _NOISE_TEX_SIZE = 4;
	constexpr UINT  _SAMPLER_COUNT = 50;
	constexpr UINT  _CBV_SRV_COUNT = 100000;
	constexpr UINT  _RTV_DSV_COUNT = 1000;
	constexpr UINT  _PREFILTER_MIP_LEVELS = 5;
	constexpr UINT  _PREFILTER_WIDTH = 128;
	constexpr UINT  _PREFILTER_HEIGHT = 128;
	constexpr UINT  _DIFFIRRAD_WIDTH = 32;
	constexpr UINT  _DIFFIRRAD_HEIGHT = 32;
	constexpr UINT  _QUAD_IDX_COUNT = 6;
	constexpr UINT  _CUBE_IDX_COUNT = 36;
	constexpr UINT  _HI_Z_CULL_COUNT = 256;
	constexpr UINT  _HI_Z_TEX_COUNT = 9;

	enum class ePass
	{
		zPass,
		cascadeDirShadowPass,
		spotShadowPass,
		cubeShadowPass,
		occlusionTestPass,
		hi_zPass,
		HWOcclusionPass,
		geoPass,
		ssaoPass,
		ssaoBlurPass,
		lightingPass,
		skyBoxPass,
		postProcess,
		cnt
	};

	enum class eGbuf
	{
		pos,
		normal,
		albedo,
		specular,
		emissive,
		vPos,
		vNormal,
		cnt
	};

	struct QUAD
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};
	struct SamplePoints
	{
		DirectX::XMVECTOR coord[_KERNEL_COUNT];
	};

	//Renderer11
	constexpr bool g_bFULL_SCREEN = false;
	constexpr bool g_bVSYNC_ENABLE = false;
	constexpr float g_fSCREEN_FAR = 1000.0f;
	constexpr float g_fSCREEN_NEAR = 0.1f;

	enum class eAPI {
		DX11,
		DX12
	};
}
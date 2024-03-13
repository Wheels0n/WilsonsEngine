#pragma once
#include <DirectXMath.h>
#include <string>
#include <Windows.h>

#define _64KB 64*1024
#define _CBV_READ_SIZE 256;
#define _CBV_ALIGN(LEN) (LEN+D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT-1u)&~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT-1u)
#define _64KB_ALIGN(LEN) (LEN+_64KB-1u)&~(_64KB-1u)
#define _ALIGN
namespace wilson
{	 
	//Cam
	constexpr float _RAD = 6.28319;

	//Frustum
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
	//Light
	enum class eLIGHT_TYPE
	{
		DIR,
		CUBE,
		SPT
	};

	//LightBuffer
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
		DirectX::XMMATRIX m_worldMat;
		DirectX::XMMATRIX m_viewMat;
		DirectX::XMMATRIX m_invWorldMat;
		DirectX::XMMATRIX m_invWVMat;
		DirectX::XMMATRIX m_wvpMat;
	};

	//Model
	enum eTexType
	{
		Diffuse,
		Normal,
		Specular,
		Emissive,
		Alpha
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
	
	//Shader
	enum eCascadeShadowRP
	{
		eCascadeShadow_eVsMat,
		eCascadeShadow_eGsMat,
		eCascadeShadow_eCnt
	};

	enum eSpotShadowRP
	{
		eSpotShadow_eVsMat,
		eSpotShadow_ePsDiffuseMap,
		eSpotShadow_ePsSampler,
		eSpotShadow_eCnt
	};

	enum eCubeShadowRP
	{
		eCubeShadow_eVsMat,
		eCubeShadow_eGsMat,
		eCubeShadow_ePsLightPos,
		eCubeShadow_ePsDiffuseMap,
		eCubeShadow_ePsSampler,
		eCubeShadow_eCnt
	};

	enum eSkyboxRP
	{
		eSkybox_eVsMat,
		eSkybox_ePsDiffuseMap,
		eSkybox_ePsSampler,
		eSkybox_eCnt
	};

	enum ePbrGeoRP
	{
		ePbrGeo_eVsMat,
		ePbrGeo_ePsDiffuse,
		ePbrGeo_ePsNormal,
		ePbrGeo_ePsSpecular,
		ePbrGeo_ePsEmissive,
		ePbrGeo_ePsAlpha,
		ePbrGeo_ePsSampler,
		ePbrGeo_ePsCamPos,
		ePbrGeo_ePsMaterial,
		ePbrGeo_ePsHeightScale,
		ePbrGeo_eCnt
	};

	enum eSsaoRP
	{
		eSsao_eCsNoise,
		eSsao_eCsVpos,
		eSsao_eCsVnomral,
		eSsao_eCsUAV,
		eSsao_eCsWrap,
		eSsao_eCsClamp,
		eSsao_eCsSamplePoints,
		eSsao_eCsProj,
		eSsao_eCnt
	};

	enum eSsaoBlurRP
	{
		eSsaoBlur_eCsDst,
		eSsaoBlur_eCsDebug,
		eSsaoBlur_eCsSsao,
		eSsaoBlur_eCsWrap,
		eSsaoBlur_eCnt
	};

	enum ePbrLightRP
	{
		ePbrLight_ePsPos,
		ePbrLight_ePsNormal,
		ePbrLight_ePsAlbedo,
		ePbrLight_ePsSpecular,
		ePbrLight_ePsEmissive,
		ePbrLight_ePsAO,
		ePbrLight_ePsIrradiance,
		ePbrLight_ePsPrefilter,
		ePbrLight_ePsBrdf,
		ePbrLight_ePsDirShadow,
		ePbrLight_ePsSpotShadow,
		ePbrLight_ePsCubeShadow,
		ePbrLight_ePsWrap,
		ePbrLight_ePsCubeShadowSampler,
		ePbrLight_ePsShadowSampler,
		ePbrLight_ePsCamPos,
		ePbrLight_ePsCasCadeLevels,
		ePbrLight_ePsProjMat,
		ePbrLight_ePsViewMat,
		ePbrLight_ePsLight,
		ePbrLight_ePsDirLitMat,
		ePbrLight_ePsSpotLitMat,
		ePbrLight_eCnt
	};

	enum eOutlinerRP
	{	
		eOutliner_eVsMat,
		eOutliner_ePsAlbedo,
		eOutliner_ePsWrap,
		eOutliner_eCnt
	};

	enum ePostProcessRP
	{
		ePostProcess_eCsTex,
		ePostProcess_eCsUav,
		ePostProcess_eCsSampler,
		ePostProcess_eCsExposure,
		ePostProcess_eCnt
	};

	enum eDiffuseIrraidianceRP
	{	
		eDiffuseIrraidiance_eGsCb,
		eDiffuseIrraidiance_ePsTex,
		eDiffuseIrraidiance_ePsSampler,
		eDiffuseIrraidiance_eCnt
	};

	enum ePrefilterRP
	{	
		ePrefilter_eGsCb,
		ePrefilter_ePsTex,
		ePrefilter_ePsSampler,
		ePrefilter_ePsCb,
		ePrefilter_eCnt
	};

	enum eEquirect2CubeRP
	{
		eEquirect2Cube_eGsCb,
		eEquirect2Cube_ePsTex,
		eEquirect2Cube_ePsSampler,
		eEquirect2Cube_eCnt
	};

	enum eGenMipRP
	{
		eGenMipRP_eCsTex,
		eGenMipRP_eCsUAV,
		eGenMipRP_eCsSampler,
		eGenMipRP_eCsCb,
		eGenMipRP_eCnt
	};
	//Engine
	static constexpr float _DRAG_THRESHOLD = 6.0f;
	static bool g_bShutdown = false;

	//Importer
	enum class eTEX
	{
		Kd,
		Ks,
		Bump,
		d
	};

	//Viewport
	static const char* g_types[] =
	{
		"obj", "fbx", "dir", "pnt", "spt"
	};

	constexpr UINT _VIEWPORT_WIDTH = 1024;
	constexpr UINT _VIEWPORT_HEIGHT = 720;

	//DirectX
	constexpr UINT _WORKER_THREAD_COUNT= 4;
	constexpr UINT _OBJECT_PER_THREAD = 10;
	constexpr float _REFRESH_RATE = 75.f;
	constexpr UINT  _BUFFER_COUNT = 2;
	constexpr UINT _SHADOWMAP_SIZE = 1024;
	constexpr float CUBE_SIZE = 0.25;
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

	enum ePass
	{
		eZpass,
		eCascadeDirShadowPass,
		eSpotShadowPass,
		eCubeShadowPass,
		eGeoPass,
		eDefault
	};

	enum eGbuf
	{
		eGbuf_pos,
		eGbuf_normal,
		eGbuf_albedo,
		eGbuf_specular,
		eGbuf_emissive,
		eGbuf_vPos,
		eGbuf_vNormal,
		eGbuf_cnt
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
	constexpr float g_fSCREEN_FAR = 3000.0f;
	constexpr float g_fSCREEN_NEAR = 0.1f;

	enum eAPI {
		DX11,
		DX12
	};
}
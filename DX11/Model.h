#ifndef MODEL_H
#define MODEL_H
#include <D3D11.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
namespace wilson {
	enum EFileType
	{
		OBJ,
		FBX
	};
	struct CbPerModel
	{
		BOOL isInstanced;
		BOOL hasSpecular;
		BOOL hasNormal;
		BOOL Padding;
	};
	struct VertexData
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 UV;
		DirectX::XMFLOAT3 norm;
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
		std::string normalMap;
		Material material;
	};
	struct TextureData
	{
		std::string name;
		std::string path;
		ID3D11ShaderResourceView* texture;
	};

	constexpr int MAX_INSTANCES = 50;
	class Model
	{
	private:
		void CreateInstanceMatrices();
	public:

		bool Init(ID3D11Device* pDevice);
		bool Init(ID3D11Device* pDevice, Material* material, ID3D11ShaderResourceView* pDiffuse);
		void UploadBuffers(ID3D11DeviceContext* context);
		void UploadBuffers(ID3D11DeviceContext* context, int i);

		inline std::string& GetMaterialName()
		{
			return m_matName;
		}
		inline DirectX::XMMATRIX* GetTranslationMatrix()
		{
			return &m_trMat;
		}
		inline DirectX::XMMATRIX* GetScaleMatrix()
		{
			return &m_scMat;
		}
		inline DirectX::XMMATRIX* GetRoatationMatrix()
		{
			return &m_rtMat;
		}
		DirectX::XMMATRIX GetTransformMatrix();
		inline DirectX::XMVECTOR* GetAngle()
		{
			return &m_angleVec;
		}
		inline unsigned int GetIndexCount()
		{
			return m_indexCount;
		}
		
		inline std::string GetName()
		{
			return m_Name;
		}

		inline void SetInstanced(bool bIsInstanced)
		{
			m_isInstanced = bIsInstanced;
		}
		inline std::vector<unsigned int>& GetNumVertexData()
		{
			return m_numVertexData;
		}
		inline std::vector<unsigned int>& GetNumIndice()
		{
			return m_numIndices;
		}
		inline std::vector<unsigned int>& GetVertexDataPos()
		{
			return m_vertexDataPos;
		}
		inline std::vector<unsigned int>& GetIndicesPos()
		{
			return m_indicesPos;
		}
		inline bool isInstanced()
		{
			return m_isInstanced;
		}
		inline int GetNumInstance()
		{
			return m_numInstance;
		}
		inline void SetNumInstance(UINT n)
		{	
			m_numInstance = n;
		}
		inline void ToggleInstancing()
		{
			m_isInstanced = ~m_isInstanced;
		}

		Model(VertexData* pVertices,
			unsigned long* pIndices,
			std::vector<unsigned int> vertexDataPos,
			std::vector<unsigned int> indicesPos,
			std::vector<MaterialInfo> materials,
			std::vector<TextureData> textures,
			wchar_t* pName);
		Model(VertexData* pVertices,
			unsigned long* pIndices,
			unsigned int vertexCount,
			unsigned int indexCount,
			wchar_t* pName,
			std::string matName);
		Model(const Model&) = delete;
		~Model();

	private:
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		ID3D11Buffer* m_pMaterialBuffer;
		ID3D11Buffer* m_pInstancePosBuffer;
		ID3D11Buffer* m_pPerModelBuffer;
		ID3D11Device* m_pDevice;

		std::string m_Name;
		std::string m_matName;

		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_vertexCount;
		unsigned int m_indexCount;
		std::vector<unsigned int> m_vertexDataPos;
		std::vector<unsigned int> m_indicesPos;
		std::vector<unsigned int> m_numVertexData;
		std::vector<unsigned int> m_numIndices;

		std::vector<TextureData> m_textures;
		std::vector<MaterialInfo> m_materials;
		Material* m_pMaterial;
		ID3D11ShaderResourceView* m_diffuseMap;

		DirectX::XMMATRIX m_scMat;
		DirectX::XMMATRIX m_rtMat;
		DirectX::XMMATRIX m_trMat;
		DirectX::XMVECTOR m_angleVec;
		
		DirectX::XMMATRIX* m_instancedData;
		UINT m_numInstance;
		bool m_isInstanced;
	};
}
#endif
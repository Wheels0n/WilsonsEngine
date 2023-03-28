#ifndef MODEL_H
#define MODEL_H
#include <D3D11.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

namespace wilson {
	enum EObjectType
	{
		OBJ,
		FBX
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

	struct TextureData
	{
		std::string name;
		std::string path;
		ID3D11ShaderResourceView* texture;
	};

	class Model
	{
	public:
		bool Init(ID3D11Device* device);
		void UploadBuffers(ID3D11DeviceContext* context);
		
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
		inline LPCWSTR GetName()
		{
			return m_pName;
		}
		inline void SetTex(ID3D11ShaderResourceView* srv)
		{
			m_SRV = srv;
		}

		Model(VertexData* pVertices,
			unsigned long* pIndices,
			unsigned int vertexCount,
			unsigned int indexCount,
			std::vector<Material> materialV,
			std::vector<TextureData> texDataV,
			wchar_t* pName);
		Model(VertexData* pVertices,
			unsigned long* pIndices,
			unsigned int vertexCount,
			unsigned int indexCount,
			wchar_t* pName);
		Model(const Model&) = delete;
		~Model();

	private:
		EObjectType m_eObjectType;
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		ID3D11Buffer* m_pMaterialBuffer;

		wchar_t* m_pName;
		VertexData** m_pPolygons;
		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_vertexCount;
		unsigned int m_indexCount;

		std::vector<Material> m_materials;
		std::vector<TextureData> m_textures;
		ID3D11ShaderResourceView* m_SRV;

		DirectX::XMMATRIX m_scMat;
		DirectX::XMMATRIX m_rtMat;
		DirectX::XMMATRIX m_trMat;
		DirectX::XMVECTOR m_angleVec;
	};
}
#endif
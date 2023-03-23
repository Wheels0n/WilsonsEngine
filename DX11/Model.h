#ifndef MODEL_H
#define MODEL_H
#include <D3D11.h>
#include <DirectXMath.h>
#include <string>

namespace wilson {

	struct VertexData
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 tex;
		DirectX::XMFLOAT3 norm;
	};

	class Model
	{
	public:
		bool Init(ID3D11Device* device);
		void UploadBuffers(ID3D11DeviceContext* context);
		
		inline void SetTex(ID3D11ShaderResourceView* tex)
		{
			m_pSRV = tex;
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
		inline LPCWSTR GetName()
		{
			return m_pName;
		}

		Model(VertexData* pVertices,
			unsigned long* pIndices,
			unsigned int vertexCount,
			unsigned int indexCount,
			wchar_t* pName);
		Model(const Model&);
		~Model();

	private:

		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		ID3D11ShaderResourceView* m_pSRV;

		wchar_t* m_pName;

		VertexData* m_pVertices;
		unsigned long* m_pIndices;
		unsigned int m_vertexCount;
		unsigned int m_indexCount;

		DirectX::XMMATRIX m_scMat;
		DirectX::XMMATRIX m_rtMat;
		DirectX::XMMATRIX m_trMat;
		DirectX::XMVECTOR m_angleVec;
	};
}
#endif
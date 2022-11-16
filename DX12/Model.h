#ifndef _MODEL_H_
#define _MODEL_H_

#include <d3d12.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
using namespace DirectX;
class CModel
{
private:
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

public:
	CModel();
	CModel(const CModel&) = delete;
	~CModel();

	bool Initialize(ID3D12Device*);
	void Shutdown();
	void Render(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCmdlist);

private:
	bool InitializeBuffers(ID3D12Device*);
	//void ShutdownBuffers();
	//void RenderBuffers();

private:
	ID3D12Resource* m_uploadBuffer;
	ID3D12Resource* m_constantBuffer;
	ID3D12Resource* m_vertexBuffer;
	ID3D12Resource* m_indexBuffer;
	ID3D12DescriptorHeap* m_cbvHeap;
	ID3D12RootSignature* m_RootSignature;
	
	ID3DBlob* m_serializedBlob, *m_vsBlob, *m_psBlob;
	ID3DBlob* m_errorBlob;

	int m_vertexCnt;
	int m_indexCnt;

	XMFLOAT4X4 m_worldViewProj  = 
	   { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };
};

#endif // !_MODEL_H_


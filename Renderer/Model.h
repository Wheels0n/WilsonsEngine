#pragma once 

#include <D3D11.h>
#include <DirectXMath.h>
#include <string>
#include <unordered_map>
#include <vector>


#include "AABB.h"
#include "typedef.h"
namespace wilson {
	
	class Model
	{

	public:

		void UpdateWorldMatrix();
		void BindMaterial(std::unordered_map<std::string, int>& mathash, std::vector<MaterialInfo>& matInfos,
			std::unordered_map<std::string, int>& texhash, std::vector<ID3D11ShaderResourceView*>& textures);
		void UploadBuffers(ID3D11DeviceContext* context, int i, bool bGeoPass);
		AABB GetGlobalAABB();

		inline AABB* GetAABB() const
		{
			return m_pAABB;
		}
		inline UINT GetMatCount() const
		{
			return m_matInfos.size();
		}
		inline DirectX::XMMATRIX GetTransformMatrix(bool bOutliner)
		{
			return bOutliner?m_outlinerMat:m_wMat;
		}
		inline DirectX::XMMATRIX GetInverseWorldMatrix()
		{
			return m_invWMat;
		}
		inline DirectX::XMMATRIX* GetTranslationMatrix()
		{
			return &m_trMat;
		}
		inline DirectX::XMMATRIX* GetScaleMatrix()
		{
			return &m_scMat;
		}
		inline DirectX::XMMATRIX* GetOutlinerScaleMatrix()
		{
			return &m_outlinerScaleMat;
		}
		inline DirectX::XMMATRIX* GetRoatationMatrix()
		{
			return &m_rtMat;
		}
		inline DirectX::XMVECTOR* GetAngle()
		{
			return &m_angleVec;
		}
		inline UINT GetIndexCount(UINT i)
		{	
			
			return m_indicesPos[i+1]-m_indicesPos[i];
		}
		
		inline std::string GetName() const
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

		Model(ID3D11Device* pDevice, VertexData* pVertices,
			unsigned long* pIndices,
			std::vector<unsigned int> vertexDataPos,
			std::vector<unsigned int> indicesPos,
			wchar_t* pName,
			std::vector<std::string> matNames);
		Model(const Model&) = delete;
		~Model();

	private:
		void CreateInstanceMatrices();

	private:
		ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;
		ID3D11Buffer* m_pMaterialBuffer;
		ID3D11Buffer* m_pInstancePosBuffer;
		ID3D11Buffer* m_pPerModelBuffer;
		ID3D11Device* m_pDevice;

		std::string m_Name;
		std::vector<std::string>m_matNames;

		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_vertexCount;
		unsigned int m_indexCount;
		std::vector<unsigned int> m_vertexDataPos;
		std::vector<unsigned int> m_indicesPos;
		std::vector<unsigned int> m_numVertexData;
		std::vector<unsigned int> m_numIndices;

		std::vector<MaterialInfo> m_matInfos;
		std::vector<PerModel>m_perModels;
		std::vector<ID3D11ShaderResourceView*> m_texSrvs;
		std::unordered_map<std::string, int> m_texHash;

		DirectX::XMMATRIX m_outlinerScaleMat;
		DirectX::XMMATRIX m_outlinerMat;
		DirectX::XMMATRIX m_scMat;
		DirectX::XMMATRIX m_rtMat;
		DirectX::XMMATRIX m_trMat;
		DirectX::XMMATRIX m_wMat;
		DirectX::XMMATRIX m_invWMat;
		DirectX::XMVECTOR m_angleVec;
		
		DirectX::XMMATRIX* m_instancedData;
		UINT m_numInstance;
		bool m_isInstanced;

		AABB* m_pAABB;
	};
}
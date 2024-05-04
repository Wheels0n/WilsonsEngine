#pragma once 
#include <D3D11.h>
#include <DirectXMath.h>

#include "AABB.h"
#include "typedef.h"
namespace wilson {
	
	class Mesh11
	{

	public:

		void BindMaterial(const std::unordered_map<std::string, int>& mathash, const std::vector<MaterialInfo>& matInfos,
			const std::unordered_map<std::string, int>& texhash, const std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& textures);
		inline AABB* GetAabb() const
		{
			return m_pAABB;
		}
		inline DirectX::XMVECTOR* GetAngle()
		{
			return &m_angleVec;
		}
		AABB GetGlobalAabb();
		inline std::string GetName() const
		{
			return m_name;
		}
		inline UINT GetNumMaterial() const
		{
			return m_matInfos.size();
		}
		inline UINT GetNumIndex(UINT i)
		{

			return m_indicesPos[i + 1] - m_indicesPos[i];
		}
		inline std::vector<unsigned int>& GetNumIndice()
		{
			return m_nIndices;
		}
		inline UINT GetNumInstance()
		{
			return m_nInstance;
		}
		inline std::vector<unsigned int>& GetNumVertexData()
		{
			return m_nVertexData;
		}
		inline std::vector<unsigned int>& GetIndicesPos()
		{
			return m_indicesPos;
		}
		inline DirectX::XMMATRIX GetInverseWorldMatrix()
		{
			return m_invWMat;
		}
		inline DirectX::XMMATRIX* GetOutlinerScaleMatrix()
		{
			return &m_outlinerScaleMat;
		}
		inline DirectX::XMMATRIX* GetRoatationMatrix()
		{
			return &m_rtMat;
		}
		inline DirectX::XMMATRIX* GetScaleMatrix()
		{
			return &m_scMat;
		}
		inline DirectX::XMMATRIX GetTransformMatrix(bool bOutliner)
		{
			return bOutliner?m_outlinerMat:m_wMat;
		}
		inline DirectX::XMMATRIX* GetTranslationMatrix()
		{
			return &m_trMat;
		}
		inline std::vector<unsigned int>& GetVertexDataPos()
		{
			return m_vertexDataPos;
		}
		inline bool IsInstanced()
		{
			return m_bInstanced;
		}
		inline void SetNumInstance(UINT n)
		{
			m_nInstance = n;
		}
		inline void SetInstanced(bool bIsInstanced)
		{
			m_bInstanced = bIsInstanced;
		}
		inline void ToggleInstancing()
		{
			m_bInstanced = ~m_bInstanced;
		}
		void UpdateWorldMatrix();
		void UploadBuffers(ID3D11DeviceContext* const context, const UINT i, const bool bGeoPass);

		Mesh11(ID3D11Device* const pDevice, VertexData* const pVertices,
			unsigned long* const pIndices,
			const std::vector<unsigned int> vertexDataPos,
			const std::vector<unsigned int> indicesPos,
			wchar_t* const  pName,
			const std::vector<std::string> matNames);
		Mesh11(const Mesh11&) = delete;
		~Mesh11();

	private:
		void CreateInstanceMatrices();

	private:
		Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pMaterialCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pInstancePosCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pPerModelCb;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVb;

		std::string m_name;
		std::vector<std::string>m_matNames;

		VertexData* m_pVertexData;
		unsigned long* m_pIndices;
		unsigned int m_nIndex;
		unsigned int m_nVertex;
		
		std::vector<unsigned int> m_indicesPos;
		std::vector<unsigned int> m_nIndices;
		std::vector<unsigned int> m_nVertexData;
		std::vector<unsigned int> m_vertexDataPos;

		std::vector<MaterialInfo> m_matInfos;
		std::vector<PerModel>m_perModels;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_texSrvs;
		std::unordered_map<std::string, int> m_texHash;

		DirectX::XMVECTOR m_angleVec;
		DirectX::XMMATRIX m_invWMat;
		DirectX::XMMATRIX m_outlinerScaleMat;
		DirectX::XMMATRIX m_outlinerMat;
		DirectX::XMMATRIX m_rtMat;
		DirectX::XMMATRIX m_scMat;
		DirectX::XMMATRIX m_trMat;
		DirectX::XMMATRIX m_wMat;
		
		DirectX::XMMATRIX* m_instancedData;
		UINT m_nInstance;
		bool m_bInstanced;

		AABB* m_pAABB;
	};
}
#include "Import.h"
namespace wilson
{
	Importer::Importer()
	{
		m_vertexCount = 0;
		m_vertexCoordCount = 0;
		m_texCoordCount = 0;
		m_normalVectorCount = 0;
		m_indexCount = 0;
		m_objectCount = 0;
		m_texCount = 0;

		m_pModel = nullptr;
		m_pVertexCoord = nullptr;
		m_pNormalVector = nullptr;
		m_pTexCoord = nullptr;
		m_pVertexData = nullptr;
		m_pIndices = nullptr;
		m_pSRV = nullptr;

		m_fbxManager = FbxManager::Create();
		m_fbxIOsettings = FbxIOSettings::Create(m_fbxManager, IOSROOT);
		m_fbxManager->SetIOSettings(m_fbxIOsettings);
		m_fbxImporter = FbxImporter::Create(m_fbxManager, "");
	}

	bool Importer::LoadOBJ(LPCWSTR fileName)
	{

		std::ifstream fin;
		fin.open(fileName);

		if (fin.fail())
		{
			return false;
		}
		char ch;
		std::string line;

		while (!fin.eof())
		{
			std::getline(fin, line, ' ');
			if (line.compare("v") == 0)
			{
				++m_vertexCoordCount;
			}
			else if (line.compare("vt") == 0)
			{
				++m_texCoordCount;
			}

			else if (line.compare("vn") == 0)
			{
				++m_normalVectorCount;
			}

			else if (line.compare("f") == 0)
			{
				fin.get(ch);
				++m_vertexCount;
				while (ch != '\n')
				{
					if (ch == ' ')
					{
						++m_vertexCount;
					}
					fin.get(ch);

				}
				continue;
			}
			std::getline(fin, line);
		}
		fin.close();
		ch = ' ';

		m_pVertexCoord = new DirectX::XMFLOAT3[m_vertexCoordCount];
		m_pTexCoord = new DirectX::XMFLOAT2[m_texCoordCount];
		m_pNormalVector = new DirectX::XMFLOAT3[m_normalVectorCount];

		m_pVertexData = new VertexData[m_vertexCount];
		m_pIndices = new unsigned long[m_vertexCount];

		ZeroMemory(m_pVertexCoord, sizeof(DirectX::XMFLOAT3) * m_vertexCoordCount);
		ZeroMemory(m_pTexCoord, sizeof(DirectX::XMFLOAT2) * m_texCoordCount);
		ZeroMemory(m_pNormalVector, sizeof(DirectX::XMFLOAT3) * m_normalVectorCount);
		ZeroMemory(m_pVertexData, sizeof(VertexData) * m_vertexCount);

		fin.open(fileName);
		if (fin.fail())
		{
			return false;
		}

		char type;
		int vCnt = 0, vtCnt = 0, vnCnt = 0, i;
		while (!fin.eof())
		{
			fin.get(type);

			if (type == 'v')
			{
				fin.get(type);
				if (type == ' ')
				{
					fin >> m_pVertexCoord[vCnt].x
						>> m_pVertexCoord[vCnt].y
						>> m_pVertexCoord[vCnt].z;
					m_pVertexCoord[vCnt].z *= -1;
					++vCnt;
				}
				else if (type == 't')
				{
					fin >> m_pTexCoord[vtCnt].x
						>> m_pTexCoord[vtCnt].y;
					m_pTexCoord[vtCnt].y = 1 - m_pTexCoord[vtCnt].y;
					++vtCnt;
				}
				else if (type == 'n')
				{
					fin >> m_pNormalVector[vnCnt].x
						>> m_pNormalVector[vnCnt].y
						>> m_pNormalVector[vnCnt].z;
					m_pNormalVector[vnCnt].z *= -1;
					++vnCnt;
				}
			}

			else if (type == 'f')
			{
				fin.get(type);
				if (type == ' ')
				{
					int v, vt, vn;

					while (!fin.fail())
					{
						fin >> v >> ch;
						fin >> vt >> ch;
						fin >> vn;
						if (!fin.fail())
						{

							m_pVertexData[m_indexCount].position = m_pVertexCoord[v - 1];
							m_pVertexData[m_indexCount].tex = m_pTexCoord[vt - 1];
							m_pVertexData[m_indexCount].norm = m_pNormalVector[vn - 1];
							//¿À¸¥¼ÕÁÂÇ¥°è¿¡¼­ ¿Þ¼ÕÁÂÇ¥°è·Î
							m_pIndices[m_indexCount] = m_vertexCount - m_indexCount - 1;
							++m_indexCount;
						}

					}
					fin.clear();
				}
			}
		}

		fin.close();

		wchar_t* tok = TokenizeCWSTR(fileName);

		m_pModel = new Model(m_pVertexData, m_pIndices, m_vertexCount, m_indexCount, tok);
		++m_objectCount;
		return true;
	}

	wchar_t* Importer::TokenizeCWSTR(LPCWSTR fileName)
	{	
		wchar_t* tok = nullptr;
		wchar_t* ptr = nullptr;
		wchar_t* temp = wcstok((wchar_t*)fileName, (const wchar_t*)L"\\", &ptr);
		while (true)
		{
			temp = wcstok(nullptr, (const wchar_t*)L"\\", &ptr);
			if (temp == nullptr)
			{
				break;
			}
			else
			{
				tok = temp;
			}

		}

		return tok;
	}

	bool Importer::LoadTex(Model* model, LPCWSTR fileName, ID3D11Device* device)
	{
		HRESULT hr;
		hr = D3DX11CreateShaderResourceViewFromFileW(device, fileName, nullptr, nullptr, &m_pSRV, nullptr);
		if (FAILED(hr))
		{
			return false;
		}

		model->SetTex(m_pSRV);
		return true;
	}

	bool Importer::LoadFbx(LPCWSTR fileName)
	{	
		std::wstring fileName_w(fileName);
		std::string fileName_c = std::string(fileName_w.begin(), fileName_w.end());

		if (m_fbxImporter == nullptr)
		{
			return false;
		}

		bool result = m_fbxImporter->Initialize(fileName_c.c_str(), -1, m_fbxManager->GetIOSettings());
		if (!result)
		{
			return false;
		}
		
		FbxScene* scene = FbxScene::Create(m_fbxManager, "scene");
		m_fbxImporter->Import(scene);
		
		FbxNode* pFbxRootNode = scene->GetRootNode();
		if (pFbxRootNode)
		{	
			for (int i = 0; i < pFbxRootNode->GetChildCount(); ++i)
			{
				FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);

				if (pFbxChildNode->GetNodeAttribute() == nullptr)
				{
					continue;
				}

				FbxNodeAttribute::EType AttributType =
					pFbxChildNode->GetNodeAttribute()->GetAttributeType();

				if (AttributType != FbxNodeAttribute::eMesh)
				{
					continue;
				}

				FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
				FbxVector4* pVertices = pMesh->GetControlPoints();

				for (int cnt = 0; cnt < pMesh->GetPolygonCount(); ++cnt)
				{
					int verticesCnt = pMesh->GetPolygonSize(cnt);
					m_vertexCount += verticesCnt;
					if (verticesCnt == 3)
					{
						m_indexCount += 3;
					}
					else
					{
						m_indexCount += 6;
					}
				}
				m_pIndices = new unsigned long[m_indexCount];
				m_pVertexData = new VertexData[m_vertexCount];

				int idxCnt = 0;
				int vCnt = 0;
				for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
				{	
					int verticesCnt = pMesh->GetPolygonSize(j);

					for (int k = 0; k < verticesCnt; ++k)
					{	
						int controlPointIndex = pMesh->GetPolygonVertex(j, k);
						m_pIndices[idxCnt] = (unsigned long)controlPointIndex;
						++idxCnt;

						VertexData v;
						v.position = DirectX::XMFLOAT3(
							pVertices[controlPointIndex].mData[0],
							pVertices[controlPointIndex].mData[1],
							pVertices[controlPointIndex].mData[2]
						);

						m_pVertexData[vCnt] = v;
						++vCnt;
					}
				}
			}
		}

		wchar_t* tok = TokenizeCWSTR(fileName);
		m_pModel = new Model(m_pVertexData, m_pIndices, m_vertexCount, m_indexCount, tok);
		return true;
	}

	Importer::~Importer()
	{
		Clear();
		m_fbxIOsettings->Destroy();
		m_fbxManager->Destroy();
		m_fbxImporter->Destroy();
	}

	void Importer::Clear()
	{
		if (m_pVertexCoord != nullptr)
		{
			delete m_pVertexCoord;
			m_pVertexCoord = nullptr;
		}

		if (m_pTexCoord != nullptr)
		{
			delete m_pTexCoord;
			m_pTexCoord = nullptr;
		}

		if (m_pNormalVector != nullptr)
		{
			delete m_pNormalVector;
			m_pNormalVector = nullptr;
		}

		if (m_pVertexData != nullptr)
		{
			m_pVertexData = nullptr;
		}

		if (m_pIndices != nullptr)
		{
			m_pIndices = nullptr;
		}

		m_vertexCount = 0;
		m_indexCount = 0;
		m_vertexCoordCount = 0;
		m_texCoordCount = 0;
		m_normalVectorCount = 0;
	}
}

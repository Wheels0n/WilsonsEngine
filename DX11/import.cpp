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
		m_pVertices = nullptr;
		m_pIndices = nullptr;
		m_pSRV = nullptr;

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

		m_pVertices = new VertexData[m_vertexCount];
		m_pIndices = new unsigned long[m_vertexCount];

		ZeroMemory(m_pVertexCoord, sizeof(DirectX::XMFLOAT3) * m_vertexCoordCount);
		ZeroMemory(m_pTexCoord, sizeof(DirectX::XMFLOAT2) * m_texCoordCount);
		ZeroMemory(m_pNormalVector, sizeof(DirectX::XMFLOAT3) * m_normalVectorCount);
		ZeroMemory(m_pVertices, sizeof(VertexData) * m_vertexCount);

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

							m_pVertices[m_indexCount].position = m_pVertexCoord[v - 1];
							m_pVertices[m_indexCount].tex = m_pTexCoord[vt - 1];
							m_pVertices[m_indexCount].norm = m_pNormalVector[vn - 1];
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

		m_pModel = new Model(m_pVertices, m_pIndices, m_vertexCount, m_indexCount, tok);
		++m_objectCount;
		return true;
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

	Importer::~Importer()
	{
		Clear();
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

		if (m_pVertices != nullptr)
		{
			m_pVertices = nullptr;
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

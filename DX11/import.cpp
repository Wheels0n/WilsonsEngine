#include "import.h"

CImporter::CImporter()
{  
	m_vertexCount = 0;
	m_vertexCoordCount = 0;
	m_texCoordCount = 0;
	m_normalVectorCount = 0;
	m_indexCount = 0;
	m_objectCount = 0;
	m_texCount = 0;

	m_pCModel = nullptr;
	m_pVertexCoord = nullptr;
	m_pNormalVector = nullptr;
	m_pTexCoord = nullptr;
	m_pVertices = nullptr;
	m_pIndices = nullptr;
	m_pShaderResourceView = nullptr;

	m_plte = nullptr;
	m_pngData = nullptr;
}

bool CImporter::LoadOBJ(LPCWSTR fileName)
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

	m_pVertexCoord = new XMFLOAT3[m_vertexCoordCount];
	m_pTexCoord = new XMFLOAT2[m_texCoordCount];
	m_pNormalVector = new XMFLOAT3[m_normalVectorCount];

	m_pVertices= new VertexType[m_vertexCount];
	m_pIndices = new unsigned long[m_vertexCount];

	ZeroMemory(m_pVertexCoord, sizeof(XMFLOAT3) * m_vertexCoordCount);
	ZeroMemory(m_pTexCoord, sizeof(XMFLOAT2) * m_texCoordCount);
	ZeroMemory(m_pNormalVector, sizeof(XMFLOAT3) * m_normalVectorCount);
	ZeroMemory(m_pVertices, sizeof(VertexType) * m_vertexCount);

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
	
	m_pCModel = new CModel(m_pVertices, m_pIndices, m_vertexCount, m_indexCount);
	++m_objectCount;
	return true;
}


bool CImporter::LoadPNG(LPCWSTR fileName, unsigned int* width, unsigned int* height)
{
	std::ifstream fin;

	unsigned int len = 0;
	unsigned char buffer[4];
	char data[38];
	char chunkType[5];
	char bitsPerPixel, colorType, compressionMethod, filterMethod;
	chunkType[4] = '\0';

	fin.open(fileName, std::ifstream::binary);
	if (fin.fail())
	{
		return false;
	}

	fin.seekg(8, std::ios::cur);//discard PNG signature;

	while (!fin.fail())
	{
		fin.read(reinterpret_cast<char*>(buffer), 4);
		len = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];

		fin.read(chunkType, 4);
		if (strcmp("IHDR", chunkType) == 0)
		{
			fin.read(reinterpret_cast<char*>(buffer), 4);
			*width = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
			fin.read(reinterpret_cast<char*>(buffer), 4);
			*height = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
			fin.read(&bitsPerPixel, 1);
			fin.read(&colorType, 1);
			fin.read(&compressionMethod, 1);
			fin.read(&filterMethod, 1);

			fin.seekg(len - 12, std::ios::cur);
		}

		else if (strcmp("PLTE", chunkType) == 0)
		{
			m_plte = new char[len];
			fin.read(m_plte, len);
		}
		else if (strcmp("IDAT", chunkType) == 0)
		{
			fin.seekg(2, std::ios::cur);
			m_pngData = new char[len];
			fin.read(m_pngData, len);
		}
		else if (strcmp("IEND", chunkType) == 0)
		{
			break;
		}
		else
		{
			fin.seekg(len, std::ios::cur);
		}
		fin.seekg(4, std::ios::cur); // discard CRC;
	}

	fin.close();
	return true;
}

bool CImporter::LoadTex(CModel* model, LPCWSTR fileName, ID3D11Device* device)
{   
	HRESULT hr;
	hr = D3DX11CreateShaderResourceViewFromFileW(device, fileName, nullptr, nullptr, &m_pShaderResourceView, nullptr);
	if (FAILED(hr))
	{
		return false;
	}

	model->SetTex(m_pShaderResourceView);
	return true;
}

CImporter::~CImporter()
{  
	Clear();

	if (m_plte == nullptr)
	{
		delete[] m_plte;
		m_plte = nullptr;
	}

	if (m_pngData == nullptr)
	{
		delete[] m_pngData;
		m_pngData = nullptr;
	}
}

void CImporter::Clear()
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
	m_indexCount  = 0;
	m_vertexCoordCount = 0;
	m_texCoordCount = 0;
	m_normalVectorCount =0;
	
}

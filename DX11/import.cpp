#include "import.h"

CImporter::CImporter()
{  
	m_objectCount = 0;
	m_texCount = 0;

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

	m_vertexCoordCounts.reserve(m_objectCount + 1);
	m_vertexCoordCounts.push_back(0);
	m_texCoordCounts.reserve(m_objectCount + 1);
	m_texCoordCounts.push_back(0);
	m_normalVectorCounts.reserve(m_objectCount + 1);
	m_normalVectorCounts.push_back(0);
	m_vertexCounts.reserve(m_objectCount + 1);
	m_vertexCounts.push_back(0);

	while (!fin.eof())
	{
		std::getline(fin, line, ' ');
		if (line.compare("v") == 0)
		{
			++m_vertexCoordCounts[m_objectCount];
		}
		else if (line.compare("vt") == 0)
		{
			++m_texCoordCounts[m_objectCount];
		}

		else if (line.compare("vn") == 0)
		{
			++m_normalVectorCounts[m_objectCount];
		}

		else if (line.compare("f") == 0)
		{
			fin.get(ch);
			++m_vertexCounts[m_objectCount];
			while (ch != '\n')
			{
				if (ch == ' ')
				{
					++m_vertexCounts[m_objectCount];
				}
				fin.get(ch);

			}
			continue;
		}
		std::getline(fin, line);
	}
	fin.close();
	ch = ' ';

	m_pVertexCoordinates.reserve(m_objectCount + 1);
	m_pVertexCoordinates.push_back(nullptr);
	m_pVertexCoordinates[m_objectCount] = new XMFLOAT3[m_vertexCoordCounts[m_objectCount]];

	m_pTexCoordinates.reserve(m_objectCount + 1);
	m_pTexCoordinates.push_back(nullptr);
	m_pTexCoordinates[m_objectCount] = new XMFLOAT2[m_texCoordCounts[m_objectCount]];

	m_pNormalVectors.reserve(m_objectCount + 1);
	m_pNormalVectors.push_back(nullptr);
	m_pNormalVectors[m_objectCount] = new XMFLOAT3[m_normalVectorCounts[m_objectCount]];

	m_pVertices.reserve(m_objectCount + 1);
	m_pVertices.push_back(nullptr);
	m_pVertices[m_objectCount] = new VertexType[m_vertexCounts[m_objectCount]];

	m_pIndices.reserve(m_objectCount + 1);
	m_pIndices.push_back(nullptr);
	m_pIndices[m_objectCount] = new unsigned long[m_vertexCounts[m_objectCount]];

	m_indexCounts.reserve(m_objectCount + 1);
	m_indexCounts.push_back(0);


	ZeroMemory(m_pVertexCoordinates[m_objectCount], sizeof(D3DXVECTOR3) * m_vertexCoordCounts[m_objectCount]);
	ZeroMemory(m_pTexCoordinates[m_objectCount], sizeof(D3DXVECTOR2) * m_texCoordCounts[m_objectCount]);
	ZeroMemory(m_pNormalVectors[m_objectCount], sizeof(D3DXVECTOR3) * m_normalVectorCounts[m_objectCount]);
	ZeroMemory(m_pVertices[m_objectCount], sizeof(VertexType) * m_vertexCounts[m_objectCount]);

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
				fin >> m_pVertexCoordinates[m_objectCount][vCnt].x
					>> m_pVertexCoordinates[m_objectCount][vCnt].y
					>> m_pVertexCoordinates[m_objectCount][vCnt].z;
				m_pVertexCoordinates[m_objectCount][vCnt].z *= -1;
				++vCnt;
			}
			else if (type == 't')
			{
				fin >> m_pTexCoordinates[m_objectCount][vtCnt].x
					>> m_pTexCoordinates[m_objectCount][vtCnt].y;
				m_pTexCoordinates[m_objectCount][vtCnt].y = 1 - m_pTexCoordinates[m_objectCount][vtCnt].y;
				++vtCnt;
			}
			else if (type == 'n')
			{
				fin >> m_pNormalVectors[m_objectCount][vnCnt].x
					>> m_pNormalVectors[m_objectCount][vnCnt].y
					>> m_pNormalVectors[m_objectCount][vnCnt].z;
				m_pNormalVectors[m_objectCount][vnCnt].z *= -1;
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
						i = m_indexCounts[m_objectCount];
						m_pVertices[m_objectCount][i].position = m_pVertexCoordinates[m_objectCount][v - 1];
						m_pVertices[m_objectCount][i].tex = m_pTexCoordinates[m_objectCount][vt - 1];
						m_pVertices[m_objectCount][i].norm = m_pNormalVectors[m_objectCount][vn - 1];
						//¿À¸¥¼ÕÁÂÇ¥°è¿¡¼­ ¿Þ¼ÕÁÂÇ¥°è·Î
						m_pIndices[m_objectCount][i] = m_vertexCounts[m_objectCount] - i - 1;
						++m_indexCounts[m_objectCount];
					}

				}
				fin.clear();
			}
		}
	}

	fin.close();

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

CImporter::~CImporter()
{  
	for (int i = m_objectCount - 1; i >= 0; --i)
	{

		if (m_pVertexCoordinates[i] != nullptr)
		{
			delete[] m_pVertexCoordinates[i];
			m_pVertexCoordinates.pop_back();
		}

		if (m_pTexCoordinates[i] != nullptr)
		{
			delete[] m_pTexCoordinates[i];
			m_pTexCoordinates.pop_back();
		}

		if (m_pNormalVectors[i] != nullptr)
		{
			delete[] m_pNormalVectors[i];
			m_pNormalVectors.pop_back();
		}

		if (m_pVertices[i] != nullptr)
		{
			delete[] m_pVertices[i];
			m_pVertices.pop_back();
		}

		if (m_pIndices[i] != nullptr)
		{
			delete[] m_pIndices[i];
			m_pIndices.pop_back();
		}


	}


	if (m_vertexCounts.size() > 0)
	{
		m_vertexCounts.clear();
		m_vertexCounts.shrink_to_fit();
	}

	if (m_indexCounts.size() > 0)
	{
		m_indexCounts.clear();
		m_indexCounts.shrink_to_fit();
	}

	if (m_vertexCoordCounts.size() > 0)
	{
		m_vertexCoordCounts.clear();
		m_vertexCoordCounts.shrink_to_fit();
	}

	if (m_texCoordCounts.size() > 0)
	{
		m_texCoordCounts.clear();
		m_texCoordCounts.shrink_to_fit();
	}

	if (m_normalVectorCounts.size() > 0)
	{
		m_normalVectorCounts.clear();
		m_normalVectorCounts.shrink_to_fit();
	}


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

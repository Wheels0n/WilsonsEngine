
#include <cwchar>
#include <filesystem>

#include "Import.h"

ULONG g_vertexVecCount=0;
ULONG g_texVecCount=0;
ULONG g_normalVecCount=0;

namespace wilson
{
	Importer::Importer()
	{
		m_vertexCount = 0;
		m_vertexVecCount = 0;
		m_texVecCount = 0;
		m_normalVecCount = 0;
		m_indexCount = 0;
		m_objectCount = 0;

		m_pModel = nullptr;
		m_pVertexVecs = nullptr;
		m_pNormalVecs = nullptr;
		m_pTexVecs = nullptr;
		m_pVertexData = nullptr;
		m_pIndices = nullptr;
		m_pSRV = nullptr;

		m_texTypeHash["Kd"] = eTEX::Kd;
		m_texTypeHash["Ks"] = eTEX::Ks;
		m_texTypeHash["Bump"] = eTEX::Bump;
		m_texTypeHash["d"] = eTEX::d;

		m_curDir = nullptr;
		m_mtlPath = nullptr;
		m_fileName = nullptr;
		m_pModelGroup = nullptr;
		m_pTangentVecs = nullptr;

		m_fbxManager = FbxManager::Create();
		m_fbxIOsettings = FbxIOSettings::Create(m_fbxManager, IOSROOT);
		m_fbxManager->SetIOSettings(m_fbxIOsettings);
	}

	std::streampos Importer::GetCnts(LPCWSTR fileName, std::streampos pos, std::string& objName)
	{
		std::string line;
		std::ifstream fin;
		std::streampos lastPos;
		char ch;

		fin.open(fileName,std::ios_base::binary);
		if (fin.fail())
		{
			return pos;
		}

		fin.seekg(pos);
		std::getline(fin, line);
		objName = line;

		while (!fin.eof())
		{	
			std::getline(fin, line, ' ');

			if (line.compare("v") == 0)
			{
				++m_vertexVecCount;
			}
			else if (line.compare("vt") == 0)
			{
				++m_texVecCount;
			}

			else if (line.compare("vn") == 0)
			{
				++m_normalVecCount;
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
			else if (line.compare("o") == 0)
			{	
				lastPos = fin.tellg();
				lastPos -= 2;
				break;
			}

			std::getline(fin, line);
		}


		fin.close();
		return lastPos;
	}
	void Importer::LoadSubOBJ(LPCWSTR fileName, std::streampos pos, ID3D11Device* pDevice, std::string& objName)
	{	
		char ch = ' ';
		std::string line;
		std::ifstream fin;
		fin.open(fileName, std::ios_base::binary);
		if (fin.fail())
		{
			return;
		}

		std::vector<std::string> matNames;
		std::vector<unsigned int> vertexDataPos;
		std::vector<unsigned int> indicesPos;
		bool hasNormal = false;

		fin.seekg(pos);
		std::getline(fin, line);

		m_pVertexVecs = new DirectX::XMFLOAT3[m_vertexVecCount];
		m_pTexVecs = new DirectX::XMFLOAT2[m_texVecCount];
		m_pNormalVecs = new DirectX::XMFLOAT3[m_normalVecCount];

		m_pVertexData = new VertexData[m_vertexCount];
		m_pIndices = new unsigned long[m_vertexCount];

		ZeroMemory(m_pVertexVecs, sizeof(DirectX::XMFLOAT3) * m_vertexVecCount);
		ZeroMemory(m_pTexVecs, sizeof(DirectX::XMFLOAT2) * m_texVecCount);
		ZeroMemory(m_pNormalVecs, sizeof(DirectX::XMFLOAT3) * m_normalVecCount);
		ZeroMemory(m_pVertexData, sizeof(VertexData) * m_vertexCount);

		char type;
		int vCnt = 0, vtCnt = 0, vnCnt = 0;
		while (!fin.eof())
		{
			fin.get(type);
			if (type == 'o')//off면 안됨
			{
				break;
			}

			else if (type == 'v')
			{
				fin.get(type);
				if (type == ' ')
				{
					fin >> m_pVertexVecs[vCnt].x
						>> m_pVertexVecs[vCnt].y
						>> m_pVertexVecs[vCnt].z;
					m_pVertexVecs[vCnt].z *= -1;
					++vCnt;
				}
				else if (type == 't')
				{
					fin >> m_pTexVecs[vtCnt].x
						>> m_pTexVecs[vtCnt].y;
					m_pTexVecs[vtCnt].y = 1 - m_pTexVecs[vtCnt].y;
					++vtCnt;
				}
				else if (type == 'n')
				{
					fin >> m_pNormalVecs[vnCnt].x
						>> m_pNormalVecs[vnCnt].y
						>> m_pNormalVecs[vnCnt].z;
					m_pNormalVecs[vnCnt].z *= -1;
					++vnCnt;
				}
			}

			else if (type == 'f')
			{
				fin.get(type);
				if (type == ' ')
				{	
					while (!fin.fail())
					{
						int v, vt, vn;

						fin >> v >> ch;
						fin >> vt >> ch;
						fin >> vn;
						if (!fin.fail())
						{

							m_pVertexData[m_indexCount].position = m_pVertexVecs[v - g_vertexVecCount - 1];
							m_pVertexData[m_indexCount].uv = m_pTexVecs[vt - g_texVecCount - 1];
							m_pVertexData[m_indexCount].norm = m_pNormalVecs[vn - g_normalVecCount - 1];
							//오른손좌표계에서 왼손좌표계로
							m_pIndices[m_indexCount] = m_indexCount;
							++m_indexCount;
						}
					}
					if (hasNormal)
					{
						DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&m_pVertexData[m_indexCount - 3].position);
						DirectX::XMVECTOR uv1 = DirectX::XMLoadFloat2(&m_pVertexData[m_indexCount - 3].uv);
						DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&m_pVertexData[m_indexCount - 2].position);
						DirectX::XMVECTOR uv2 = DirectX::XMLoadFloat2(&m_pVertexData[m_indexCount - 2].uv);
						DirectX::XMVECTOR pos3 = DirectX::XMLoadFloat3(&m_pVertexData[m_indexCount-1].position);
						DirectX::XMVECTOR uv3 = DirectX::XMLoadFloat2(&m_pVertexData[m_indexCount-1].uv);

						DirectX::XMFLOAT3 e1;
						DirectX::XMStoreFloat3(&e1, DirectX::XMVectorSubtract(pos2, pos1));
						DirectX::XMFLOAT3 e2;
						DirectX::XMStoreFloat3(&e2, DirectX::XMVectorSubtract(pos3, pos1));
						DirectX::XMFLOAT2 dUV1;
						DirectX::XMStoreFloat2(&dUV1, DirectX::XMVectorSubtract(uv2, uv1));
						DirectX::XMFLOAT2 dUV2;
						DirectX::XMStoreFloat2(&dUV2, DirectX::XMVectorSubtract(uv3, uv1));

						float det = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);
						DirectX::XMFLOAT3 tangent;
						tangent.x = det * (dUV2.y * e1.x - dUV1.y * e2.x);
						tangent.y = det * (dUV2.y * e1.y - dUV1.y * e2.y);
						tangent.z = det * (dUV2.y * e1.z - dUV1.y * e2.z);

						m_pVertexData[m_indexCount - 1].tangent = tangent;
						m_pVertexData[m_indexCount - 2].tangent = tangent;
						m_pVertexData[m_indexCount - 3].tangent = tangent;
					}
					fin.clear();
				}
			
			}
			else if (type == 'u')
			{
				std::getline(fin, line,' ');
				std::getline(fin, line);
				matNames.push_back(line);

				int idx = m_matHash[line];
				hasNormal = !m_Materials[idx].normalMap.empty();
				vertexDataPos.push_back(m_indexCount);
				indicesPos.push_back(m_indexCount);
			}
			else if(type=='s')
			{
				std::getline(fin, line);
			}
		}

		g_vertexVecCount += m_vertexVecCount;
		g_texVecCount += m_texVecCount;
		g_normalVecCount += m_normalVecCount;

		vertexDataPos.push_back(m_vertexCount);
		indicesPos.push_back(m_indexCount);
		std::wstring wobjName = std::wstring(objName.begin(), objName.end());
		m_pModel = new Model(m_pVertexData, m_pIndices, vertexDataPos, indicesPos, (wchar_t*)wobjName.c_str(), matNames);
		m_pModels.push_back(m_pModel);
		++m_objectCount;

		fin.close();
		
	}
	bool Importer::LoadOBJ(LPCWSTR fileName, ID3D11Device* pDevice)
	{

		std::ifstream fin;
		fin.open(fileName,std::ios::binary);

		if (fin.fail())
		{
			return false;
		}
		
		LoadMTL(m_mtlPath, pDevice);

		std::string line;
		std::string objName;
		std::streampos curPos,nextPos;
		while (!fin.eof())
		{	
			std::getline(fin, line,' ');
			if (line.compare("o") == 0)
			{	
				curPos = fin.tellg();
				nextPos=GetCnts(fileName, curPos, objName);
				LoadSubOBJ(fileName, curPos, pDevice, objName);
				ClearModel();
				if (nextPos == 0)
				{
					break;
				}
				fin.seekg(nextPos);
			}
			else
			{
				std::getline(fin, line);
			}
			
			
		}
		fin.close();

		m_pModelGroup = new ModelGroup(m_pModels, m_Materials, m_pTexMaps,
			m_fileName, eFileType::OBJ, m_matHash, m_texHash);
		
		ClearModel();
		ClearModelGroup();
		g_vertexVecCount = 0;
		g_texVecCount    = 0;
		g_normalVecCount = 0;
		return true;
	}
	bool Importer::LoadModel(const char* extension, LPCWSTR fileName, ID3D11Device* pDevice)
	{
		GetCurDir(fileName);
		int len = wcslen(fileName);
		m_fileName = new wchar_t[len + 1];
		wcscpy(m_fileName, fileName);
		m_fileName[len] = L'\0';
		m_fileName = GetFileName(m_fileName);
		m_mtlPath = GetMTLPath(m_curDir, m_fileName);//파일명과 subMesh명이 반드시 일치x
		if (!strcmp(extension, "obj"))
		{
			return LoadOBJ(fileName, pDevice);
		}
		else if (!strcmp(extension, "fbx"))
		{
			return LoadFbx(fileName, pDevice);
		}
		delete[] m_fileName;
	}
	bool Importer::LoadMTL(wchar_t* fileName, ID3D11Device* pDevice)
	{
		std::ifstream fin;
		fin.open(fileName);
		if (fin.fail())
		{
			return false;
		}

		const char* delimiter = " ";
		std::string line;

		std::getline(fin, line);
		std::getline(fin, line, ':');//두 번째 행에 머티리얼 개수가 표시됨
		std::getline(fin, line);
		int matCnt = atoi(line.c_str());
		m_Materials.reserve(matCnt);
		std::getline(fin, line);


		while (!fin.eof())
		{
			std::getline(fin, line, ' ');

			if (line.compare("newmtl") == 0)
			{	
				MaterialInfo mat = { "","","","",{0,}};
				std::getline(fin, line);
				std::string matName = line;
				m_matHash   [matName] = m_Materials.size();
				{	
					float shininess = 1.0f;
					while (!fin.eof())
					{
						std::getline(fin, line);
						char* tok = strtok((char*)line.c_str(), delimiter);
						if (tok == nullptr)
						{
							break;
						}
						if (strcmp(tok, "Ns") == 0)
						{
							tok = strtok(nullptr, delimiter);
							shininess = std::stof(tok);
						}
						else if (strcmp(tok, "Ka") == 0)
						{
							tok = strtok(nullptr, delimiter);
							float x = std::stof(tok);
							tok = strtok(nullptr, delimiter);
							float y = std::stof(tok);
							tok = strtok(nullptr, delimiter);
							float z = std::stof(tok);

							DirectX::XMFLOAT4 ambient4(x, y, z, 1.0f);
							mat.material.ambient = DirectX::XMLoadFloat4(&ambient4);
						}
						else if (strcmp(tok, "Kd") == 0)
						{
							tok = strtok(nullptr, delimiter);
							float x = std::stof(tok);
							tok = strtok(nullptr, delimiter);
							float y = std::stof(tok);
							tok = strtok(nullptr, delimiter);
							float z = std::stof(tok);

							DirectX::XMFLOAT4 diffuse4(x, y, z, 1.0f);
							mat.material.diffuse = DirectX::XMLoadFloat4(&diffuse4);
						}
						else if (strcmp(tok, "Ks") == 0)
						{
							tok = strtok(nullptr, delimiter);
							float x = std::stof(tok);
							tok = strtok(nullptr, delimiter);
							float y = std::stof(tok);
							tok = strtok(nullptr, delimiter);
							float z = std::stof(tok);

							DirectX::XMFLOAT4 specular4(x, y, z, shininess);
							mat.material.specular = DirectX::XMLoadFloat4(&specular4);
						}
						else if (strncmp(tok, "map",3) == 0)
						{
							std::string texType(tok);
							int pos = texType.find_last_of('_')+1;
							texType = std::string(texType.begin() + pos, texType.end());

							tok = strtok(nullptr, delimiter);
							std::string texName(tok);
							
							m_texHash[texName] = m_pTexMaps.size();
							std::wstring wstr(texName.begin(), texName.end());
							pos = wstr.find_last_of(L'\\');
							wstr = wstr.substr(pos + 1, std::string::npos);


							int len = wcslen(m_curDir) + wstr.length() + 2;
							wchar_t* mapPath = new wchar_t[len];
							wcscpy(mapPath, m_curDir);
							mapPath = wcsncat(mapPath, L"\\", 2);
							mapPath = wcsncat(mapPath, wstr.c_str(), wstr.size());

							bool isDiffuse = false;
							switch (m_texTypeHash[texType])
							{
							case eTEX::Kd:
								isDiffuse = true;
								mat.diffuseMap = texName;
								break;
							case eTEX::Ks:
								mat.specularMap = texName;
								break;
							case eTEX::Bump:
								mat.normalMap = texName;
								break;
							case eTEX::d:
								mat.alphaMap = texName;
								break;
							}

							LoadTex(mapPath, pDevice,isDiffuse);
							delete[] mapPath;
						}

					}
					m_Materials.push_back(mat);
				}
			}
			else
			{
				while (line.compare("") != 0)
				{
					std::getline(fin, line);
				}
			}
		}

		fin.close();
	}
	
	void Importer::GetCurDir(LPCWSTR fileName)
	{
		std::wstring::size_type pos = std::wstring(fileName).find_last_of(L"\\");
		std::wstring curDirWStr = std::wstring(fileName).substr(0, pos);
		m_curDir = new wchar_t[curDirWStr.size()+1];
		m_curDir = wcscpy(m_curDir, curDirWStr.c_str());
		//널문자 만나면 끝나니까 직접 추가
		m_curDir[curDirWStr.size()] = L'\0';
	}
	wchar_t* Importer::GetMTLPath(LPCWSTR filePath, wchar_t* tok)
	{	
		int len = wcslen(filePath);
		len += wcslen(tok);
		len += 7; //.mtl\0 + // + 
		wchar_t* ptr = nullptr;
		wchar_t* tmp = new wchar_t[len];
		tmp = wcscpy(tmp, filePath);
		tmp = wcstok(tmp, L".", &ptr);	
		tmp = wcsncat(tmp, L"\\", 2);
		//strcat은 null문자 있는 곳에 그냥 이어쓰기! 메모리 초과 쓰기 주의
		wchar_t* mtlPath = wcsncat(tmp, tok, wcslen(tok));
		mtlPath = wcsncat(mtlPath, L".mtl\0", 5);
		return mtlPath;
	}
	wchar_t* Importer::GetFileName(LPCWSTR fileName)
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

		tok = wcstok(tok, L".", &ptr);
		return tok;
	}
	bool Importer::LoadFbxTex(std::string fileName, FbxSurfaceMaterial* pSurfaceMaterial,
		MaterialInfo* pMatInfo, std::vector<std::string>& matNames, ID3D11Device* pDevice)
	{	
		std::filesystem::path fbxPath = fileName.c_str();
		std::string texturesPath = fbxPath.parent_path().string() + "\\";
		
		const char* mapTypes[4] = { FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sNormalMap, FbxSurfaceMaterial::sSpecular, 
			FbxSurfaceMaterial::sSpecularFactor};//reflectioFactor==metallic
		for (int i = 0; i < 4; ++i)
		{	
			D3DX11_IMAGE_LOAD_INFO loadInfo = {};
			if (i==0)
			{
				loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
				loadInfo.Filter = D3DX11_FILTER_SRGB | D3DX11_FILTER_NONE;
			}

			//FBX는 C++멤버변수 보다는 FbxProperty를 이용한다.
			//텍스쳐는 대응 되는 속성(ex:디퓨즈)에 소스 오브젝트로 존재한다.
			FbxProperty property = pSurfaceMaterial->FindProperty(mapTypes[i]);
			std::string materialName(mapTypes[i]);

			int layeredTexCount = property.GetSrcObjectCount(FbxCriteria::ObjectType(FbxLayeredTexture::ClassId));
			if (layeredTexCount == 0)
			{
				FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject(FbxCriteria::ObjectType(FbxFileTexture::ClassId), 0));
				int texCount = property.GetSrcObjectCount(FbxCriteria::ObjectType(FbxTexture::ClassId));
				if (texCount > 0)
				{
					HRESULT hr;
					std::string relativePath(texture->GetRelativeFileName());
					std::string path = texturesPath + relativePath;
					std::string name = std::string(texture->GetName());
					
					if (m_texHash.find(name) == m_texHash.end())
					{	
						std::wstring wPath = std::wstring(path.begin(), path.end());
						hr = D3DX11CreateShaderResourceViewFromFileW(pDevice, wPath.c_str(), &loadInfo, nullptr, &m_pSRV, nullptr);
						if (FAILED(hr))
						{
							//return false;
						}
						m_pSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
							sizeof("Importer::m_pSRV") - 1, "Importer::m_pSRV");

						m_pTexMaps.push_back(m_pSRV);
						m_texHash[name] = m_pTexMaps.size() - 1;
						switch (i)
						{
						case 0:
							pMatInfo->diffuseMap = name;
							break;
						case 1:
							pMatInfo->normalMap = name;
							break;
						default:
							pMatInfo->specularMap = name;
							break;
						}
					}

				}
			}
		}
		m_Materials.push_back(*pMatInfo);
		FbxString nameFStr = pSurfaceMaterial->GetNameOnly();
		std::string name(nameFStr.Buffer());
		matNames.push_back(name);
		m_matHash[name] = m_Materials.size() - 1;
		return true;
	}
	Material Importer::LoadFbxMaterial(FbxSurfaceMaterial* pSurfaceMaterial)
	{
		Material mat = { 0, };
		const char* pProperties[4] = { FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sSpecular,  FbxSurfaceMaterial::sReflection };
		const char* pFactorProperties[4] = { FbxSurfaceMaterial::sAmbientFactor, FbxSurfaceMaterial::sDiffuseFactor, FbxSurfaceMaterial::sSpecularFactor, FbxSurfaceMaterial::sReflectionFactor };
		for (int i = 0; i < 4; ++i)
		{
			const FbxProperty property = pSurfaceMaterial->FindProperty(pProperties[i]);
			const FbxProperty factorProperty = pSurfaceMaterial->FindProperty(pFactorProperties[i]);
			if (property.IsValid() && factorProperty.IsValid())
			{
				FbxDouble3 propertyDouble = property.Get<FbxDouble3>();
				float factor = factorProperty.Get<float>();
				DirectX::XMVECTOR propertyVec = DirectX::XMVectorSet(propertyDouble[0] * factor, propertyDouble[1] * factor, propertyDouble[2] * factor, 1.0f);
				switch (i)
				{
				case 0:
					mat.ambient = propertyVec;
					break;
				case 1:
					mat.diffuse = propertyVec;
					break;
				case 2:
					mat.specular = propertyVec;
					break;
				default:
					mat.specular = propertyVec;
					break;
				}
			}
		
		}
		return mat;
	}
	bool Importer::LoadTex(LPCWSTR fileName, ID3D11Device* pDevice, bool isDiffuse)
	{  
		D3DX11_IMAGE_LOAD_INFO loadInfo = {};
		if (isDiffuse)
		{	
			loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			loadInfo.Filter = D3DX11_FILTER_SRGB|D3DX11_FILTER_NONE;
		}
		HRESULT hr = D3DX11CreateShaderResourceViewFromFileW(pDevice, fileName, &loadInfo, nullptr, &m_pSRV, nullptr);
		if (FAILED(hr))
		{
			return false;
		}
		m_pSRV->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Importer::m_pSRV") - 1, "Importer::m_pSRV");
		m_pTexMaps.push_back(m_pSRV);
		return true;
	}
	void Importer::LoadSubFbx(FbxMesh* pMesh, FbxVector4* pVertices, 
		std::vector<UINT>& vertexDataPos, std::vector<UINT>& indicesPos, std::vector<UINT>& submeshStride, std::vector<std::string>& matNames)
	{	
		//pos를 나눈 이유는 화분처럼 여러 재질로 이루어져있을 경우 나눠서 DrawCall을 하기 위함
		int submeshCount = 0;
		for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
		{
			if (submeshStride.size()>1&&submeshStride[submeshCount + 1] == j)
			{
				++submeshCount;
				vertexDataPos.push_back(m_vertexCount);
				indicesPos.push_back(m_indexCount);
			}

			int verticesCnt = pMesh->GetPolygonSize(j);
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
		vertexDataPos.push_back(m_vertexCount);
		indicesPos.push_back(m_indexCount);
		m_pIndices = new unsigned long[m_indexCount];
		m_pVertexData = new VertexData[m_vertexCount];

		int idx = -1;
		int idxCnt = 0;
		int vCnt = 0;
		for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
		{
			int verticesCnt = pMesh->GetPolygonSize(j);

			for (int k = 0; k < verticesCnt; ++k)
			{	
				//인덱스를 참조하여 나온 정점데이터를 순서대로 저장하니까, 인덱스버퍼는 오름차순이 된다. 
				int polygonV = pMesh->GetPolygonVertex(j, k);
				m_pIndices[idxCnt] = idxCnt;
				++idxCnt;

				VertexData v;
				v.position = DirectX::XMFLOAT3(
					static_cast<float>(pVertices[polygonV].mData[0]),
					static_cast<float>(pVertices[polygonV].mData[1]),
					static_cast<float>(pVertices[polygonV].mData[2])
				);

				FbxGeometryElementNormal* pNormal = pMesh->GetElementNormal();
				//vertex != controlPoint
				switch (pNormal->GetMappingMode())
				{
				case FbxGeometryElement::eByControlPoint:
					//참조 방식은 N번쨰 요소가 N번쨰 자리에 있으면 eDirect, 인덱스 배열을 통해 접근해야하면 eIndexToDirect
					switch (pNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:

						v.norm.x = static_cast<float>(
							pNormal->GetDirectArray().GetAt(polygonV).mData[0]);
						v.norm.y = static_cast<float>(
							pNormal->GetDirectArray().GetAt(polygonV).mData[1]);
						v.norm.z = static_cast<float>(
							pNormal->GetDirectArray().GetAt(polygonV).mData[2]);
						break;

					case FbxGeometryElement::eIndexToDirect:

						idx = pNormal->GetIndexArray().GetAt(polygonV);
						v.norm.x = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[0]);
						v.norm.y = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[1]);
						v.norm.z = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[2]);
						break;

					default:
						break;
					}
					break;
				case FbxGeometryElement::eByPolygonVertex:
					switch (pNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
					
						v.norm.x = static_cast<float>(
							pNormal->GetDirectArray().GetAt(vCnt).mData[0]);
						v.norm.y = static_cast<float>(
							pNormal->GetDirectArray().GetAt(vCnt).mData[1]);
						v.norm.z = static_cast<float>(
							pNormal->GetDirectArray().GetAt(vCnt).mData[2]);
						break;
					
					case FbxGeometryElement::eIndexToDirect:
					
						idx = pNormal->GetIndexArray().GetAt(vCnt);
						v.norm.x = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[0]);
						v.norm.y = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[1]);
						v.norm.z = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[2]);
						break;
					
					default:
						break;
					}
					break;
				default:
					break;
				}
				if (pMesh->GetElementTangentCount())
				{
					FbxGeometryElementTangent* pTangent = pMesh->GetElementTangent();
	
					switch (pTangent->GetMappingMode())
					{
					case FbxGeometryElement::eByControlPoint:
						switch (pTangent->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
							
								v.tangent.x = static_cast<float>(pTangent->GetDirectArray().GetAt(polygonV).mData[0]);
								v.tangent.y = static_cast<float>(pTangent->GetDirectArray().GetAt(polygonV).mData[1]);
								v.tangent.z = static_cast<float>(pTangent->GetDirectArray().GetAt(polygonV).mData[2]);
								break;
							
							case FbxGeometryElement::eIndexToDirect:
							
								idx = pTangent->GetIndexArray().GetAt(polygonV);
								v.tangent.x = static_cast<float>(pTangent->GetDirectArray().GetAt(idx).mData[0]);
								v.tangent.y = static_cast<float>(pTangent->GetDirectArray().GetAt(idx).mData[1]);
								v.tangent.z = static_cast<float>(pTangent->GetDirectArray().GetAt(idx).mData[2]);
								break;
							
							default:
							
								break;
							
							
						}
						break;
					case FbxGeometryElement::eByPolygonVertex:
						switch (pTangent->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
							
								v.tangent.x = static_cast<float>(pTangent->GetDirectArray().GetAt(vCnt).mData[0]);
								v.tangent.y = static_cast<float>(pTangent->GetDirectArray().GetAt(vCnt).mData[1]);
								v.tangent.z = static_cast<float>(pTangent->GetDirectArray().GetAt(vCnt).mData[2]);
								break;
							
							case FbxGeometryElement::eIndexToDirect:
							
								idx = pTangent->GetIndexArray().GetAt(vCnt);
								v.tangent.x = static_cast<float>(pTangent->GetDirectArray().GetAt(idx).mData[0]);
								v.tangent.y = static_cast<float>(pTangent->GetDirectArray().GetAt(idx).mData[1]);
								v.tangent.z = static_cast<float>(pTangent->GetDirectArray().GetAt(idx).mData[2]);
								break;
							
							default:
							
								break;
							
						}
					}
				}

				FbxGeometryElementUV* pUV = pMesh->GetElementUV();
				int uvIndx = pMesh->GetTextureUVIndex(j, k);
				switch (pUV->GetMappingMode())
				{
					case FbxGeometryElement::eByControlPoint:
						switch (pUV->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
							
								v.uv.x = static_cast<float>(pUV->GetDirectArray().GetAt(polygonV).mData[0]);
								v.uv.y = static_cast<float>(pUV->GetDirectArray().GetAt(polygonV).mData[1]);
								break;
							
							case FbxGeometryElement::eIndexToDirect:
							
								idx = pUV->GetIndexArray().GetAt(polygonV);
								v.uv.x = static_cast<float>(pUV->GetDirectArray().GetAt(idx).mData[0]);
								v.uv.y = static_cast<float>(pUV->GetDirectArray().GetAt(idx).mData[1]);
								break;
							
							default:
								break;
						}
						break;
					case FbxGeometryElement::eByPolygonVertex:
					{
						switch (pUV->GetReferenceMode())
						{
							case FbxGeometryElement::eDirect:
							
								v.uv.x = static_cast<float>(pUV->GetDirectArray().GetAt(uvIndx).mData[0]);
								v.uv.y = static_cast<float>(pUV->GetDirectArray().GetAt(uvIndx).mData[1]);
								break;
							
							case FbxGeometryElement::eIndexToDirect:
							
								v.uv.x = static_cast<float>(pUV->GetDirectArray().GetAt(uvIndx).mData[0]);
								v.uv.y = static_cast<float>(pUV->GetDirectArray().GetAt(uvIndx).mData[1]);
								break;
							
							default:
								break;
						}

					}
				}

				v.uv.y = 1 - v.uv.y;
				m_pVertexData[vCnt] = v;
				++vCnt;
			}
		}
		FbxString subMeshName = pMesh->GetNameOnly();
		std::string name(subMeshName.Buffer());
		std::string wName(name.begin(), name.end());
		m_pModel = new Model(m_pVertexData, m_pIndices, vertexDataPos, indicesPos,
			(wchar_t*)wName.c_str(), matNames);
		m_pModels.push_back(m_pModel);
	}
	bool Importer::LoadFbx(LPCWSTR fileName, ID3D11Device* pDevice)
	{	
		std::unordered_set<FbxSurfaceMaterial*> materialSet;


		std::wstring wfilePath(fileName);
		std::string filePath = std::string(wfilePath.begin(), wfilePath.end());
		wchar_t* wchFileName = GetFileName(fileName);
		std::wstring wfileName(wchFileName);
		
		m_fbxImporter = FbxImporter::Create(m_fbxManager, "");
		bool result = m_fbxImporter->Initialize(filePath.c_str(), -1, m_fbxManager->GetIOSettings());
		if (!result)
		{
			return false;
		}
		
		FbxScene* scene = FbxScene::Create(m_fbxManager, "scene");
		m_fbxImporter->Import(scene);
		m_fbxImporter->Destroy();

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

				std::vector<std::string> matNames;
				std::vector<unsigned int> submeshStride;
				std::vector<unsigned int> vertexDataPos;
				std::vector<unsigned int> indicesPos;

				FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
				FbxLayerElementMaterial* pMaterial = pMesh->GetLayer(0)->GetMaterials();
				if (pMaterial != nullptr)
				{
					switch (pMaterial->GetMappingMode())
					{
					case FbxLayerElement::eAllSame:
					{
						int matId = pMaterial->GetIndexArray().GetAt(0);
						FbxSurfaceMaterial* pSurfaceMaterial = pMesh->GetNode()->GetMaterial(matId);
					
						if (matId >= 0)
						{	
							materialSet.insert(pSurfaceMaterial);
							MaterialInfo matInfo;
							matInfo.material = LoadFbxMaterial(pSurfaceMaterial);
							LoadFbxTex(filePath, pSurfaceMaterial, &matInfo, matNames, pDevice);
						}
						break;
					}
					case FbxLayerElement::eByPolygon:
					{
						for (int i = 0; i < pMesh->GetPolygonCount(); ++i)
						{
							for (int j = 0; j < pMesh->GetLayerCount(); ++j)
							{	
								FbxLayerElementMaterial* layerMaterial = pMesh->GetLayer(j)->GetMaterials();
								if (layerMaterial != nullptr)
								{	
									FbxSurfaceMaterial* pSurfaceMaterial = nullptr;
									int matId = -1;

									pSurfaceMaterial = pMesh->GetNode()->GetMaterial(layerMaterial->GetIndexArray().GetAt(i));
									matId = layerMaterial->GetIndexArray().GetAt(i);

									if (matId >= 0)
									{	
										if (materialSet.find(pSurfaceMaterial) == materialSet.end())
										{	
											materialSet.insert(pSurfaceMaterial);
											MaterialInfo matInfo;
											matInfo.material = LoadFbxMaterial(pSurfaceMaterial);
											LoadFbxTex(filePath, pSurfaceMaterial, &matInfo, matNames,pDevice);
											//파츠마다 다른 머티리얼 쓰는 게 상식
											submeshStride.push_back(i);
										}		
									}
								}
							}
						}
						break;
					}
					default:
						break;
					}
				}

				//controlPoint는 면당 정점들이다. 
				FbxVector4* pVertices = pMesh->GetControlPoints();
				submeshStride.push_back(pMesh->GetPolygonCount());
				vertexDataPos.push_back(0);
				indicesPos.push_back(0);
				//FBX가 Map을 담고 있을 경우를 대비하여 LoadFBX는 Loop문으로 LoadSubFbx(subMesh)를 호출하고, ModelGroup을 만들도록한다.
				LoadSubFbx(pMesh, pVertices, vertexDataPos, indicesPos, submeshStride, matNames);
				ClearModel();
			}
			m_pModelGroup = new ModelGroup(m_pModels, m_Materials, m_pTexMaps, (wchar_t*)wfileName.c_str(),
				eFileType::FBX, m_matHash, m_texHash);
			ClearModelGroup();
		}

		return true;
	}

	Importer::~Importer()
	{
		ClearModel();
		if (m_curDir != nullptr)
		{
			delete m_curDir;
			m_curDir = nullptr;
		}

		m_fbxIOsettings->Destroy();
		m_fbxManager->Destroy();
	}
	void Importer::ClearModelGroup()
	{
		m_objectCount = 0;
		m_pModels.clear();
		m_Materials.clear();
		m_pTexMaps.clear();
		m_matHash.clear();
		m_texHash.clear();
	}
	void Importer::ClearModel()
	{
		if (m_pVertexVecs != nullptr)
		{
			delete m_pVertexVecs;
			m_pVertexVecs = nullptr;
		}

		if (m_pTexVecs != nullptr)
		{
			delete m_pTexVecs;
			m_pTexVecs = nullptr;
		}

		if (m_pNormalVecs != nullptr)
		{
			delete m_pNormalVecs;
			m_pNormalVecs = nullptr;
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
		m_vertexVecCount = 0;
		m_texVecCount = 0;
		m_normalVecCount = 0;

	}
}

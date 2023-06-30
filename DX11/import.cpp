#include "Import.h"
#include <cwchar>
#include <filesystem>

unsigned long g_vertexVecCount=0;
unsigned long g_texVecCount=0;
unsigned long g_normalVecCount=0;

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

		m_texTypeHash["Kd"] = ETEX::Kd;
		m_texTypeHash["Ks"] = ETEX::Ks;
		m_texTypeHash["bump"] = ETEX::bump;
		m_texTypeHash["d"] = ETEX::d;

		m_curDir = nullptr;
		m_mtlPath = nullptr;
		m_fileName = nullptr;
		m_pModelGroup = nullptr;

		m_fbxManager = FbxManager::Create();
		m_fbxIOsettings = FbxIOSettings::Create(m_fbxManager, IOSROOT);
		m_fbxManager->SetIOSettings(m_fbxIOsettings);
		m_fbxImporter = FbxImporter::Create(m_fbxManager, "");
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
		std::string line, matName;
		std::ifstream fin;
		fin.open(fileName, std::ios_base::binary);
		if (fin.fail())
		{
			return;
		}
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
					int v, vt, vn;

					while (!fin.fail())
					{
						fin >> v >> ch;
						fin >> vt >> ch;
						fin >> vn;
						if (!fin.fail())
						{

							m_pVertexData[m_indexCount].position = m_pVertexVecs[v -g_vertexVecCount- 1];
							m_pVertexData[m_indexCount].UV = m_pTexVecs[vt - g_texVecCount -1];
							m_pVertexData[m_indexCount].norm = m_pNormalVecs[vn -g_normalVecCount- 1];
							//오른손좌표계에서 왼손좌표계로
							m_pIndices[m_indexCount] = m_indexCount;
							++m_indexCount;
						}

					}
					fin.clear();
				}
			
			}
			else if (type == 'u')
			{
				std::getline(fin, line,' ');
				std::getline(fin, line);
				matName = line;
			}
			else if(type=='s')
			{
				std::getline(fin, line);
			}
		}
		g_vertexVecCount += m_vertexVecCount;
		g_texVecCount += m_texVecCount;
		g_normalVecCount += m_normalVecCount;

		std::wstring wobjName = std::wstring(objName.begin(), objName.end());
		m_pModel = new Model(m_pVertexData, m_pIndices, m_vertexCount, m_indexCount, (wchar_t*)wobjName.c_str(),matName);
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

		LoadMTL(m_mtlPath, pDevice);
		m_pModelGroup = new ModelGroup(m_pModels, m_Materials, m_pTexMaps,
			m_fileName, EFileType::OBJ, m_matHash, m_texHash);
		
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

							
							switch (m_texTypeHash[texType])
							{
							case ETEX::Kd:
								mat.diffuseMap = texName;
								break;
							case ETEX::Ks:
								mat.specularMap = texName;
								break;
							case ETEX::bump:
								mat.normalMap = texName;
								break;
							case ETEX::d:
								mat.alphaMap = texName;
								break;
							}

							LoadTex(mapPath, pDevice);
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
		std::unordered_set<std::string>& texSet ,std::vector<TextureData>& texVec, ID3D11Device* pDevice)
	{	
		std::filesystem::path fbxPath = fileName.c_str();
		std::string texturesPath = fbxPath.parent_path().string() + "\\";
		

		FbxProperty fbxProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

		ID3D11ShaderResourceView* tex = nullptr;
		std::string materialName(FbxSurfaceMaterial::sDiffuse);

		int layeredTexCount = fbxProperty.GetSrcObjectCount(FbxCriteria::ObjectType(FbxLayeredTexture::ClassId));
		if (layeredTexCount ==0)
		{
			FbxFileTexture* texture = FbxCast<FbxFileTexture>(fbxProperty.GetSrcObject(FbxCriteria::ObjectType(FbxFileTexture::ClassId), 0));
			int texCount = fbxProperty.GetSrcObjectCount(FbxCriteria::ObjectType(FbxTexture::ClassId));
			if (texCount >= 0)
			{
				HRESULT hr;
				TextureData textureData;
				textureData.path = texturesPath + std::string(texture->GetRelativeFileName());
				textureData.name = std::string(texture->GetName());
				textureData.texture = nullptr;

				if (texSet.find(textureData.name) == texSet.end())
				{	
					std::wstring wPath = std::wstring(textureData.path.begin(), textureData.path.end());
					hr = D3DX11CreateShaderResourceViewFromFileW(pDevice, wPath.c_str(), nullptr, nullptr, &textureData.texture, nullptr);
					if (FAILED(hr))
					{
						return false;
					}
					
					texVec.push_back(textureData);
					texSet.insert(textureData.name);
				}
				else
				{
					return false;
				}
				

			}
		}
		return true;
	}
	MaterialInfo Importer::LoadFbxMaterial(FbxSurfaceMaterial* pSurfaceMaterial)
	{
		MaterialInfo mat = { 0, };
		const FbxProperty AmbientProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
		const FbxProperty AmbientFactorProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
		if (AmbientProperty.IsValid() && AmbientFactorProperty.IsValid())
		{
			FbxDouble3 ambientDouble = AmbientProperty.Get<FbxDouble3>();
			DirectX::XMVECTOR ambientVec = DirectX::XMVectorSet(ambientDouble[0], ambientDouble[1], ambientDouble[2], 1.0f);

			float factor = AmbientFactorProperty.Get<float>();
			ambientVec = DirectX::XMVectorMultiply(ambientVec, DirectX::XMVectorSet(factor, factor, factor, factor));
			mat.material.ambient = ambientVec;
		}
		const FbxProperty diffuseProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		const FbxProperty diffuseFactorProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
		if (diffuseProperty.IsValid() && diffuseFactorProperty.IsValid())
		{
			FbxDouble3 diffuseDouble = diffuseProperty.Get<FbxDouble3>();
			DirectX::XMVECTOR diffuseVec = DirectX::XMVectorSet(diffuseDouble[0], diffuseDouble[1], diffuseDouble[2], 1.0f);

			float factor = diffuseFactorProperty.Get<float>();
			diffuseVec = DirectX::XMVectorMultiply(diffuseVec, DirectX::XMVectorSet(factor, factor, factor, factor));
			mat.material.diffuse = diffuseVec;
		}
		const FbxProperty specularProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
		const FbxProperty specularFactorProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
		if (specularProperty.IsValid() && specularFactorProperty.IsValid())
		{
			FbxDouble3 specularDouble = specularProperty.Get<FbxDouble3>();
			DirectX::XMVECTOR specularVec = DirectX::XMVectorSet(specularDouble[0], specularDouble[1], specularDouble[2], 1.0f);

			float factor = specularFactorProperty.Get<float>();
			specularVec = DirectX::XMVectorMultiply(specularVec, DirectX::XMVectorSet(factor, factor, factor, factor));
			mat.material.specular = specularVec;
		};
		
		return mat;
	}
	bool Importer::LoadTex(LPCWSTR fileName, ID3D11Device* pDevice)
	{
		HRESULT hr;
		hr = D3DX11CreateShaderResourceViewFromFileW(pDevice, fileName, nullptr, nullptr, &m_pSRV, nullptr);
		if (FAILED(hr))
		{
			return false;
		}

		m_pTexMaps.push_back(m_pSRV);
		return true;
	}
	bool Importer::LoadFbx(LPCWSTR fileName, ID3D11Device* pDevice)
	{	
		std::unordered_set<std::string> texSet;
		std::unordered_set<FbxSurfaceMaterial*> materialSet;
		std::vector<TextureData> texVec;
		std::vector<MaterialInfo> materialVec;

		std::vector<unsigned int> submeshStride;
		std::vector<unsigned int> vertexDataPos;
		std::vector<unsigned int> indicesPos;

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
							materialVec.push_back(LoadFbxMaterial(pSurfaceMaterial));
							LoadFbxTex(fileName_c, pSurfaceMaterial, texSet, texVec, pDevice);
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
											MaterialInfo material = LoadFbxMaterial(pSurfaceMaterial);
											materialVec.push_back(material);
											materialSet.insert(pSurfaceMaterial);
											LoadFbxTex(fileName_c, pSurfaceMaterial, texSet, texVec, pDevice);

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

				submeshStride.push_back(pMesh->GetPolygonCount());
				vertexDataPos.push_back(0);
				indicesPos.push_back(0);

				int submeshCount = 0;
				for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
				{	
					if (submeshStride[submeshCount + 1] == j)
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

				int idxCnt = 0;
				int vCnt = 0;
				for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
				{	
					int verticesCnt = pMesh->GetPolygonSize(j);

					for (int k = 0; k < verticesCnt; ++k)
					{	
						int controlPointIndex = pMesh->GetPolygonVertex(j, k);
						m_pIndices[idxCnt] = idxCnt;
						++idxCnt;

						VertexData v;
						v.position = DirectX::XMFLOAT3(
							static_cast<float>(pVertices[controlPointIndex].mData[0]),
							static_cast<float>(pVertices[controlPointIndex].mData[1]),
							static_cast<float>(pVertices[controlPointIndex].mData[2])
						);

						FbxGeometryElementNormal* pNormal = pMesh->GetElementNormal();
					    //vertex != controlPoint
						switch (pNormal->GetMappingMode())
						{
						case FbxGeometryElement::eByControlPoint:
							switch (pNormal->GetReferenceMode())
							{
							 case FbxGeometryElement::eDirect:
							 {
								 v.norm.x = static_cast<float>(
									 pNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
								 v.norm.y = static_cast<float>(
									 pNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
								 v.norm.z = static_cast<float>(
									 pNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
								 break;
							 }
							 case FbxGeometryElement::eIndexToDirect:
							 {
								 int idx = pNormal->GetIndexArray().GetAt(controlPointIndex);
								 v.norm.x = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[0]);
								 v.norm.y = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[1]);
								 v.norm.z = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[2]);
							 }
							default:
								break;
							}
							break;
						case FbxGeometryElement::eByPolygonVertex:
							switch (pNormal->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
							{
								v.norm.x = static_cast<float>(
									pNormal->GetDirectArray().GetAt(vCnt).mData[0]);
								v.norm.y = static_cast<float>(
									pNormal->GetDirectArray().GetAt(vCnt).mData[1]);
								v.norm.z = static_cast<float>(
									pNormal->GetDirectArray().GetAt(vCnt).mData[2]);
								break;
							}
							case FbxGeometryElement::eIndexToDirect:
							{
								int idx = pNormal->GetIndexArray().GetAt(vCnt);
								v.norm.x = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[0]);
								v.norm.y = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[1]);
								v.norm.z = static_cast<float>(pNormal->GetDirectArray().GetAt(idx).mData[2]);
								break;
							}
							default:
								break;
							}
							break;
						default:
							break;
						}

						FbxGeometryElementUV* pUV = pMesh->GetElementUV();
						int uvIndx = pMesh->GetTextureUVIndex(j, k);
						switch (pUV->GetMappingMode())
						{
						 case FbxGeometryElement::eByControlPoint:
							switch (pUV->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
							{
								v.UV.x = static_cast<float>(pUV->GetDirectArray().GetAt(controlPointIndex).mData[0]);
								v.UV.y = static_cast<float>(pUV->GetDirectArray().GetAt(controlPointIndex).mData[1]);
								break;
							}
							case FbxGeometryElement::eIndexToDirect:
							{
								int idx = pUV->GetIndexArray().GetAt(controlPointIndex);
								v.UV.x = static_cast<float>(pUV->GetDirectArray().GetAt(idx).mData[0]);
								v.UV.y = static_cast<float>(pUV->GetDirectArray().GetAt(idx).mData[1]);
								break;
							}
							default:
								break;
							}
							break;
						 case FbxGeometryElement::eByPolygonVertex:
						 {
							switch (pUV->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
							{
								v.UV.x = static_cast<float>(pUV->GetDirectArray().GetAt(uvIndx).mData[0]);
								v.UV.y = static_cast<float>(pUV->GetDirectArray().GetAt(uvIndx).mData[1]);
								break;
							}
							case FbxGeometryElement::eIndexToDirect:
							{
								int idx = pUV->GetIndexArray().GetAt(uvIndx);
								v.UV.x = static_cast<float>(pUV->GetDirectArray().GetAt(uvIndx).mData[0]);
								v.UV.y = static_cast<float>(pUV->GetDirectArray().GetAt(uvIndx).mData[1]);
								break;
							}
							default:
								break;
							}

						 }
						}

						v.UV.y = 1 - v.UV.y;
						m_pVertexData[vCnt] = v;
						++vCnt;
					}
				}

			}
		}

		wchar_t* tok = GetFileName(fileName);
		m_pModel = new Model(m_pVertexData, m_pIndices, vertexDataPos, indicesPos, materialVec, texVec, tok);
		m_pModels.push_back(m_pModel);
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
		//m_fbxImporter->Destroy();
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

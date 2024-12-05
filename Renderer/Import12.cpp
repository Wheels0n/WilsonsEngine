#pragma once
#include <d3d12.h>
#include <DirectXTex.h>
#include "D3D12.h"
#include "Import12.h"
#include "HeapManager.h"

namespace wilson
{
	ULONG Importer12::g_nVertexVec = 0;
	ULONG Importer12::g_nTexVec = 0;
	ULONG Importer12::g_nNormalVec = 0;

	Importer12::Importer12(D3D12*const pD3D12)
	{
		m_nVertex = 0;
		m_nVertexVec = 0;
		m_nTexVec = 0;
		m_nNormalVec = 0;
		m_nIndex = 0;
		m_nObject = 0;

		m_pMesh = nullptr;
		m_pVertexVecs = nullptr;
		m_pNormalVecs = nullptr;
		m_pTexVecs = nullptr;
		m_pVertexData = nullptr;
		m_pIndices = nullptr;

		m_texTypeHash["Kd"] = eTEX::Kd;
		m_texTypeHash["Ks"] = eTEX::Ks;
		m_texTypeHash["Bump"] = eTEX::Bump;
		m_texTypeHash["d"] = eTEX::d;

		m_pCurDir = nullptr;
		m_pMtlPath = nullptr;
		m_pFileName = nullptr;
		m_pObject = nullptr;
		m_pTangentVecs = nullptr;

		m_fbxManager = FbxManager::Create();
		m_fbxIOsettings = FbxIOSettings::Create(m_fbxManager, IOSROOT);
		m_fbxManager->SetIOSettings(m_fbxIOsettings);

		m_pD3D12 = pD3D12;
		m_pDevice = m_pD3D12->GetDevice();
		m_pHeapManager= m_pD3D12->GetHeapManager();
		m_pCommandList = m_pD3D12->GetCommandList();

		
		HRESULT hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pImporterCommandAllocator.GetAddressOf()));
		assert(SUCCEEDED(hr));
		m_pImporterCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Importer12::m_pImporterCommandAllocator") - 1, "Importer12::m_pImporterCommandAllocator");

		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pImporterCommandAllocator.Get(), nullptr, IID_PPV_ARGS(m_pImporterCommandList.GetAddressOf()));
		assert(SUCCEEDED(hr));
		m_pImporterCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pImporterCommandList") - 1, "D3D12::m_pImporterCommandList");
	}
	void Importer12::GetExtension(char* pDst, const char* pSrc)
	{
		int i = 0;
		while (true)
		{
			++i;
			if (pSrc[i] == '.')
			{
				++i;
				break;
			}
		}

		for (int j = 0; j < 3; ++j, ++i)
		{
			pDst[j] = pSrc[i];
		}
		pDst[3] = '\0';
		return;
	}
	std::streampos Importer12::GetCnts(LPCWSTR pFileName, const std::streampos pos, std::string& objName)
	{
		std::string line;
		std::ifstream fin;
		std::streampos lastPos;
		char ch;

		fin.open(pFileName, std::ios_base::binary);
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
				++m_nVertexVec;
			}
			else if (line.compare("vt") == 0)
			{
				++m_nTexVec;
			}

			else if (line.compare("vn") == 0)
			{
				++m_nNormalVec;
			}

			else if (line.compare("f") == 0)
			{
				fin.get(ch);
				++m_nVertex;
				while (ch != '\n')
				{
					if (ch == ' ')
					{
						++m_nVertex;
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
	void Importer12::LoadSubOBJ(LPCWSTR pFileName, const std::streampos pos, ID3D12Device* const pDevice, const std::string& objName)
	{
		char ch = ' ';
		std::string line;
		std::ifstream fin;
		fin.open(pFileName, std::ios_base::binary);
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

		m_pVertexVecs = new DirectX::XMFLOAT3[m_nVertexVec];
		m_pTexVecs = new DirectX::XMFLOAT2[m_nTexVec];
		m_pNormalVecs = new DirectX::XMFLOAT3[m_nNormalVec];

		m_pVertexData = new VertexData[m_nVertex];
		m_pIndices = new unsigned long[m_nVertex];

		ZeroMemory(m_pVertexVecs, sizeof(DirectX::XMFLOAT3) * m_nVertexVec);
		ZeroMemory(m_pTexVecs, sizeof(DirectX::XMFLOAT2) * m_nTexVec);
		ZeroMemory(m_pNormalVecs, sizeof(DirectX::XMFLOAT3) * m_nNormalVec);
		ZeroMemory(m_pVertexData, sizeof(VertexData) * m_nVertex);

		char type;
		UINT vCnt = 0, vtCnt = 0, vnCnt = 0;
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

							m_pVertexData[m_nIndex].position = m_pVertexVecs[v - g_nVertexVec - 1];
							m_pVertexData[m_nIndex].uv = m_pTexVecs[vt - g_nTexVec - 1];
							m_pVertexData[m_nIndex].norm = m_pNormalVecs[vn - g_nNormalVec - 1];
							//오른손좌표계에서 왼손좌표계로
							m_pIndices[m_nIndex] = m_nIndex;
							++m_nIndex;
						}
					}
					if (hasNormal)
					{
						DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&m_pVertexData[m_nIndex - 3].position);
						DirectX::XMVECTOR uv1 = DirectX::XMLoadFloat2(&m_pVertexData[m_nIndex - 3].uv);
						DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&m_pVertexData[m_nIndex - 2].position);
						DirectX::XMVECTOR uv2 = DirectX::XMLoadFloat2(&m_pVertexData[m_nIndex - 2].uv);
						DirectX::XMVECTOR pos3 = DirectX::XMLoadFloat3(&m_pVertexData[m_nIndex - 1].position);
						DirectX::XMVECTOR uv3 = DirectX::XMLoadFloat2(&m_pVertexData[m_nIndex - 1].uv);

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

						m_pVertexData[m_nIndex - 1].tangent = tangent;
						m_pVertexData[m_nIndex - 2].tangent = tangent;
						m_pVertexData[m_nIndex - 3].tangent = tangent;
					}
					fin.clear();
				}

			}
			else if (type == 'u')
			{
				std::getline(fin, line, ' ');
				std::getline(fin, line);
				matNames.push_back(line);

				int idx = m_matHash[line];
				hasNormal = !m_materialInfos[idx].normalMap.empty();
				vertexDataPos.push_back(m_nIndex);
				indicesPos.push_back(m_nIndex);
			}
			else if (type == 's')
			{
				std::getline(fin, line);
			}
		}

		g_nVertexVec += m_nVertexVec;
		g_nTexVec += m_nTexVec;
		g_nNormalVec += m_nNormalVec;

		vertexDataPos.push_back(m_nVertex);
		indicesPos.push_back(m_nIndex);
		std::wstring wobjName = std::wstring(objName.begin(), objName.end());
		DirectX::XMVECTOR zeroV = DirectX::XMVectorZero();
	/*	m_pMesh = new Mesh12(m_pDevice, m_pCommandList, m_pHeapManager, 
			m_pVertexData, m_pIndices, vertexDataPos, indicesPos, (wchar_t*)wobjName.c_str(), matNames);*/
		m_pMeshes.push_back(m_pMesh);
		++m_nObject;

		fin.close();

	}
	bool Importer12::LoadOBJ(LPCWSTR pFileName, ID3D12Device* const pDevice)
	{
		std::ifstream fin;
		fin.open(pFileName, std::ios::binary);

		if (fin.fail())
		{
			return false;
		}

		LoadMTL(m_pMtlPath, pDevice);

		std::string line;
		std::string objName;
		std::streampos curPos, nextPos;
		while (!fin.eof())
		{
			std::getline(fin, line, ' ');
			if (line.compare("o") == 0)
			{
				curPos = fin.tellg();
				nextPos = GetCnts(pFileName, curPos, objName);
				LoadSubOBJ(pFileName, curPos, pDevice, objName);
				ClearMesh();
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


		//Gen SRV
		for (int i = 0; i < m_pTexs.size(); ++i)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = m_pTexs[i]->GetDesc().Format;
			srvDesc.Texture2D.MipLevels = m_pTexs[i]->GetDesc().MipLevels;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			
			m_Srv = m_pHeapManager->GetSrv(srvDesc, m_pTexs[i], pDevice);
			m_pTexSrvs.push_back(m_Srv);

		}
		m_pObject = new Object12(m_pMeshes, m_materialInfos, m_pTexSrvs, m_pTexs, 
			m_pFileName, eFileType::OBJ, m_matHash, m_texHash);

		ClearMesh();
		ClearObject();
		g_nVertexVec = 0;
		g_nTexVec = 0;
		g_nNormalVec = 0;
		return true;
	}
	bool Importer12::LoadObject(const char* pExtension, LPCWSTR pFileName, ID3D12Device* const pDevice)
	{
		GetCurDir(pFileName);
		int len = wcslen(pFileName);
		m_pFileName = new wchar_t[len + 1];
		wcscpy(m_pFileName, pFileName);
		m_pFileName[len] = L'\0';
		m_pFileName = GetFileName(m_pFileName);
		m_pMtlPath = GetMTLPath(m_pCurDir, m_pFileName);//파일명과 subMesh명이 반드시 일치x
		if (!strcmp(pExtension, "obj"))
		{
			return LoadOBJ(pFileName, pDevice);
		}
		else if (!strcmp(pExtension, "fbx"))
		{
			return LoadFbx(pFileName, pDevice);
		}
		delete[] m_pFileName;
	}
	bool Importer12::LoadMTL(wchar_t* pFileName, ID3D12Device* const pDevice)
	{
		std::ifstream fin;
		fin.open(pFileName);
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
		m_materialInfos.reserve(matCnt);
		std::getline(fin, line);


		while (!fin.eof())
		{
			std::getline(fin, line, ' ');

			if (line.compare("newmtl") == 0)
			{
				MaterialInfo mat = { "","","","",{0,} };
				std::getline(fin, line);
				std::string matName = line;
				m_matHash[matName] = m_materialInfos.size();
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
						else if (strncmp(tok, "map", 3) == 0)
						{
							std::string texType(tok);
							int pos = texType.find_last_of('_') + 1;
							texType = std::string(texType.begin() + pos, texType.end());

							tok = strtok(nullptr, delimiter);
							std::string texName(tok);

							m_texHash[texName] = m_pTexSrvs.size();
							std::wstring wstr(texName.begin(), texName.end());
							pos = wstr.find_last_of(L'\\');
							wstr = wstr.substr(pos + 1, std::string::npos);


							int len = wcslen(m_pCurDir) + wstr.length() + 2;
							wchar_t* mapPath = new wchar_t[len];
							wcscpy(mapPath, m_pCurDir);
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

							LoadTex(mapPath, pDevice, isDiffuse);
							delete[] mapPath;
						}

					}
					m_materialInfos.push_back(mat);
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

	FbxAMatrix Importer12::GetNodeTransfrom(FbxNode* const pNode)
	{
		FbxAMatrix trM, scM, scPM, scOM,
			rtPM, rtOM, preRM, rtM, postRtM, wM;

		FbxAMatrix parentM, globalTM, globalRSM;
		if (!pNode)
		{
			wM.SetIdentity();
			return wM;
		}

		FbxVector4 translation = pNode->LclTranslation.Get();
		trM.SetT(translation);

		FbxVector4 rotation = pNode->LclRotation.Get();
		FbxVector4 preRot = pNode->PreRotation.Get();
		FbxVector4 postRot = pNode->PostRotation.Get();
		rtM.SetR(rotation);
		preRM.SetR(preRot);
		postRtM.SetR(postRot);

		FbxVector4 scaling = pNode->LclScaling.Get();
		scM.SetS(scaling);

		FbxVector4 scaleOffset = pNode->ScalingOffset.Get();
		FbxVector4 scalePivot = pNode->ScalingPivot.Get();
		FbxVector4 rotationOffset = pNode->RotationOffset.Get();
		FbxVector4 rotationPivot = pNode->RotationPivot.Get();
		scOM.SetT(scaleOffset);
		scPM.SetT(scalePivot);
		rtOM.SetT(rotationOffset);
		rtPM.SetT(rotationPivot);

		FbxNode* pParentNode = pNode->GetParent();
		if (pParentNode)
		{
			parentM = GetNodeTransfrom(pParentNode);
		}
		else
		{
			parentM.SetIdentity();
		}

		FbxAMatrix localRM, parentGRM;
		FbxVector4 parentGlobalRotation = parentM.GetR();
		parentGRM.SetR(parentGlobalRotation);
		localRM = preRM * rtM * postRtM;

		FbxAMatrix localSM, parentGSM, parentGRSM, parentTM;
		FbxVector4 parentGT = parentM.GetT();
		parentTM.SetT(parentGT);
		parentGRSM = parentTM.Inverse() * parentM;
		parentGSM = parentGRM.Inverse() * parentGRSM;
		localSM = scM;

		FbxTransform::EInheritType inheritType = pNode->InheritType.Get();
		if (inheritType == FbxTransform::eInheritRrSs)
		{
			globalRSM = parentGRM * localRM * parentGSM * localSM;
		}
		else if (inheritType == FbxTransform::eInheritRSrs)
		{
			globalRSM = parentGRM * parentGSM * localRM * localSM;
		}
		else if (inheritType == FbxTransform::eInheritRrs)
		{
			FbxAMatrix parentLSM;
			FbxVector4 parentLS = pParentNode->LclScaling.Get();
			parentLSM.SetS(parentLS);

			FbxAMatrix parentGSM_noLocal = parentGSM * parentLSM.Inverse();
			globalRSM = parentGRM * localRM * parentGSM_noLocal * localSM;
		}
		else
		{
			FBXSDK_printf("error, unknown inherit type! \n");
		}

		// Construct translation matrix
		// Calculate the local transform matrix
		wM = trM * rtOM * rtPM * preRM * rtM * postRtM * rtPM.Inverse()
			* scOM * scPM * scM * scPM.Inverse();
		FbxVector4 localTWithAllPivotAndOffsetInfo = wM.GetT();
		// Calculate global translation vector according to: 
		// GlobalTranslation = ParentGlobalTransform * LocalTranslationWithPivotAndOffsetInfo
		FbxVector4 globalTranslation = parentM.MultT(localTWithAllPivotAndOffsetInfo);
		globalTM.SetT(globalTranslation);

		//Construct the whole global transform
		wM = globalTM * globalRSM;


		return wM;
	}

	void Importer12::GetCurDir(LPCWSTR pFileName)
	{
		std::wstring::size_type pos = std::wstring(pFileName).find_last_of(L"\\");
		std::wstring curDirWStr = std::wstring(pFileName).substr(0, pos);
		m_pCurDir = new wchar_t[curDirWStr.size() + 1];
		m_pCurDir = wcscpy(m_pCurDir, curDirWStr.c_str());
		//널문자 만나면 끝나니까 직접 추가
		m_pCurDir[curDirWStr.size()] = L'\0';
	}
	wchar_t* Importer12::GetMTLPath(LPCWSTR pFilePath, wchar_t* pTok)
	{
		UINT len = wcslen(pFilePath);
		len += wcslen(pTok);
		len += 7; //.mtl\0 + // + 
		wchar_t* ptr = nullptr;
		wchar_t* pTmp = new wchar_t[len];
		pTmp = wcscpy(pTmp, pFilePath);
		pTmp = wcstok(pTmp, L".", &ptr);
		pTmp = wcsncat(pTmp, L"\\", 2);
		//strcat은 null문자 있는 곳에 그냥 이어쓰기! 메모리 초과 쓰기 주의
		wchar_t* pMtlPath = wcsncat(pTmp, pTok, wcslen(pTok));
		pMtlPath = wcsncat(pMtlPath, L".mtl\0", 5);
		return pMtlPath;
	}
	wchar_t* Importer12::GetFileName(LPCWSTR pFileName)
	{
		wchar_t* pTok = nullptr;
		wchar_t* ptr = nullptr;
		wchar_t* pTemp = wcstok((wchar_t*)pFileName, (const wchar_t*)L"\\", &ptr);
		while (true)
		{
			pTemp = wcstok(nullptr, (const wchar_t*)L"\\", &ptr);
			if (pTemp == nullptr)
			{
				break;
			}
			else
			{
				pTok = pTemp;
			}

		}

		pTok = wcstok(pTok, L".", &ptr);
		return pTok;
	}
	bool Importer12::LoadFbxTex(const std::string fileName, FbxSurfaceMaterial* const pSurfaceMaterial,
		MaterialInfo* const pMatInfo, std::vector<std::string>& matNames, ID3D12Device* const pDevice)
	{
		std::filesystem::path fbxPath = fileName.c_str();
		std::string texturesPath = fbxPath.parent_path().string() + "\\";

		const char* mapTypes[5] = { FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sNormalMap, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sSpecular,
			FbxSurfaceMaterial::sSpecularFactor };//reflectioFactor==metallic
		for (int i = 0; i < 5; ++i)
		{

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
					char extension[4];
					GetExtension(extension, path.c_str());
					if (m_texHash.find(name) == m_texHash.end())
					{
						std::wstring wPath = std::wstring(path.begin(), path.end());
						DirectX::ScratchImage image;
						DirectX::ScratchImage decompressdImage;
						DirectX::ScratchImage resizedImage;
						DirectX::ScratchImage dstImage;
						DirectX::TexMetadata metadata;
						ID3D12Resource* pTex=nullptr;
						UINT8* pData=nullptr;
						if (strcmp(extension, "dds") == 0)
						{ 

							hr = DirectX::LoadFromDDSFile(wPath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
							metadata = image.GetMetadata();
							pData = image.GetPixels();
							if (DirectX::IsCompressed(metadata.format) &&
								(metadata.width < 4 || metadata.height < 4))
							{
								//Decompress
								
								hr = DirectX::Decompress(image.GetImages(), image.GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, decompressdImage);
								metadata = decompressdImage.GetMetadata();
								//Resize
								
								hr = DirectX::Resize(decompressdImage.GetImages(), decompressdImage.GetImageCount(), metadata,
									4, 4, DirectX::TEX_FILTER_DEFAULT, resizedImage);
								metadata = resizedImage.GetMetadata();
								//Compress
								
								hr = DirectX::Compress(resizedImage.GetImages(), resizedImage.GetImageCount(), metadata, image.GetMetadata().format,
									DirectX::TEX_COMPRESS_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT, dstImage);
								metadata = dstImage.GetMetadata();
								pData = dstImage.GetPixels();
							
							}

						}
						else
						{
							hr = DirectX::LoadFromWICFile(wPath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);
							pData = image.GetPixels();
							metadata = image.GetMetadata();
						}

						{//
							size_t rowPitch;
							size_t slidePitch;
							ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slidePitch);
							
							D3D12_RESOURCE_DESC	texDesc= {};
							//4byte배수 제한
							texDesc.Width =metadata.width;
							texDesc.Height = metadata.height;
							texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
							texDesc.Alignment = 0;
							texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
							texDesc.Format = metadata.format;
							texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
							texDesc.DepthOrArraySize = 1;
							texDesc.MipLevels = metadata.mipLevels;
							texDesc.SampleDesc.Count = 1;
							texDesc.SampleDesc.Quality = 0;

							m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &pTex, m_pDevice);
							pTex->SetPrivateData(WKPDID_D3DDebugObjectName,
								sizeof("Importer12:: pTex") - 1, "Importer12:: pTex");
							
							ID3D12Resource* pUploadCB = nullptr;
							m_pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, pTex, &pUploadCB, m_pImporterCommandList.Get());
							
							D3D12_RESOURCE_BARRIER copyDstToSrv = {};
							copyDstToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
							copyDstToSrv.Transition.pResource = pTex;
							copyDstToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
							copyDstToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
							copyDstToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
							

							m_pImporterCommandList->ResourceBarrier(1, &copyDstToSrv);
							m_pImporterCommandList->Close();
							m_pD3D12->ExecuteCommandLists(m_pImporterCommandList.GetAddressOf(), 1);
							pUploadCB->Release();
							m_pImporterCommandAllocator->Reset();
							m_pImporterCommandList->Reset(m_pImporterCommandAllocator.Get(), nullptr);
						}
						
						m_pTexs.push_back(pTex);
						m_texHash[name] = m_pTexs.size() - 1;
						switch (i)
						{
						case 0:
							pMatInfo->diffuseMap = name;
							break;
						case 1:
							pMatInfo->normalMap = name;
							break;
						case 2:
							pMatInfo->emissiveMap = name;
							break;
						default:
							pMatInfo->specularMap = name;
							break;
						}
					}
					
				}
			}
		}
		m_materialInfos.push_back(*pMatInfo);
		FbxString nameFStr = pSurfaceMaterial->GetNameOnly();
		std::string name(nameFStr.Buffer());
		matNames.push_back(name);
		m_matHash[name] = m_materialInfos.size() - 1;
		return true;
	}
	Material Importer12::LoadFbxMaterial(FbxSurfaceMaterial* const pSurfaceMaterial)
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
	bool Importer12::LoadTex(LPCWSTR pFileName, ID3D12Device* const pDevice, bool bDiffuse)
	{
		
		ID3D12Resource* pTex;
		HRESULT hr;
		//Gen Tex
		{
			//Gen IconTexFromFile 

			DirectX::ScratchImage image;
			hr = DirectX::LoadFromWICFile(pFileName, DirectX::WIC_FLAGS_NONE, nullptr, image);
			D3D12_RESOURCE_DESC	texDesc = {};
			texDesc.Width = image.GetMetadata().width;
			texDesc.Height = image.GetMetadata().height;
			texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			texDesc.Alignment = 0;
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Format = image.GetMetadata().format;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;

			m_pHeapManager->CreateTexture(texDesc, D3D12_RESOURCE_STATE_GENERIC_READ, &pTex, m_pDevice);
			pTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Importer12:: pTex") - 1, "Importer12:: pTex");
		}
		m_pTexs.push_back(pTex);
		return true;
	}
	void Importer12::LoadSubFbx(FbxMesh* const pMesh, FbxVector4* const pVertices,
		std::vector<UINT>& vertexDataPos, std::vector<UINT>& indicesPos,
		const std::vector<UINT>& submeshStride, const std::vector<std::string>& matNames, const std::string& name,
		const FbxAMatrix& wMat)
	{
		//pos를 나눈 이유는 화분처럼 여러 재질로 이루어져있을 경우 나눠서 DrawCall을 하기 위함
		int submeshCount = 0;
		for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
		{
			if (submeshStride.size() > 1 && submeshStride[submeshCount + 1] == j)
			{
				++submeshCount;
				vertexDataPos.push_back(m_nVertex);
				indicesPos.push_back(m_nIndex);
			}

			int verticesCnt = pMesh->GetPolygonSize(j);
			m_nVertex += verticesCnt;
			if (verticesCnt == 3)
			{
				m_nIndex += 3;
			}
			else
			{
				m_nIndex += 6;
			}

		}
		vertexDataPos.push_back(m_nVertex);
		indicesPos.push_back(m_nIndex);
		
		m_pIndices = new unsigned long[m_nIndex];
		m_pVertexData = new VertexData[m_nVertex];
		FbxVector4 tan;
		
		
		int idx = -1;
		int idxCnt = 0;
		int vCnt = 0;
		std::unordered_map<int, std::vector<int>> vToTri;
		std::vector<std::vector<int>> triInfo;
		//정점 데이터 파싱
		//FBX데이터에서는 정점 번호로 바로 가져옴. 근데 저장할떄는 정점이 겹쳐도  
		//TBN계산떄문에 그냥 VCnt로 마구잡이로 넣었었음
		//삼각형idx+offset으로 index 설정. 정점데이터는 나오는 순서대로 그냥 집어넣기
		for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
		{
			int verticesCnt = pMesh->GetPolygonSize(j);
			std::vector<int> v;
			for (int k = 0; k < verticesCnt; ++k)
			{ 
				//인덱스를 참조하여 나온 정점데이터를 순서대로 저장하니까, 인덱스버퍼는 오름차순이 된다. 
				int polygonV = pMesh->GetPolygonVertex(j, k);
				vToTri[polygonV].push_back(j);
				v.push_back(polygonV);

				m_pIndices[idxCnt] = idxCnt;
				++idxCnt;

				VertexData v;
				FbxVector4 pos = pVertices[polygonV].mData;
				pos = wMat.MultT(pos);

				v.position = DirectX::XMFLOAT3(
					static_cast<float>(pos[0]),
					static_cast<float>(pos[1]),
					static_cast<float>(pos[2])
				);

				FbxGeometryElementNormal* pNormal = pMesh->GetElementNormal();
				FbxVector4 norm;
				//vertex != controlPoint
				switch (pNormal->GetMappingMode())
				{
				case FbxGeometryElement::eByControlPoint:
					//참조 방식은 N번쨰 요소가 N번쨰 자리에 있으면 eDirect, 인덱스 배열을 통해 접근해야하면 eIndexToDirect
					switch (pNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						norm = pNormal->GetDirectArray().GetAt(polygonV);

						break;

					case FbxGeometryElement::eIndexToDirect:

						idx = pNormal->GetIndexArray().GetAt(polygonV);
						norm = pNormal->GetDirectArray().GetAt(idx);

						break;

					default:
						break;
					}
					break;
				case FbxGeometryElement::eByPolygonVertex:
					switch (pNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						norm = pNormal->GetDirectArray().GetAt(vCnt);
						break;

					case FbxGeometryElement::eIndexToDirect:

						idx = pNormal->GetIndexArray().GetAt(vCnt);
						norm = pNormal->GetDirectArray().GetAt(idx);
						break;

					default:
						break;
					}
					break;
				default:
					break;
				}
				norm.Normalize();
				norm = wMat.MultT(norm);
				v.norm.x = norm.mData[0];
				v.norm.y = norm.mData[1];
				v.norm.z = norm.mData[2];

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

				if (vCnt && vCnt % 3 == 0)
				{

					if (pMesh->GetElementTangentCount())
					{
						FbxGeometryElementTangent* pTangent = pMesh->GetElementTangent();
						switch (pTangent->GetMappingMode())
						{
						case FbxGeometryElement::eByControlPoint:
							switch (pTangent->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:
								tan = pTangent->GetDirectArray().GetAt(polygonV);
								break;

							case FbxGeometryElement::eIndexToDirect:

								idx = pTangent->GetIndexArray().GetAt(polygonV);
								tan = pTangent->GetDirectArray().GetAt(idx);

								break;

							default:

								break;


							}
							break;
						case FbxGeometryElement::eByPolygonVertex:
							switch (pTangent->GetReferenceMode())
							{
							case FbxGeometryElement::eDirect:

								tan = pTangent->GetDirectArray().GetAt(vCnt);

								break;

							case FbxGeometryElement::eIndexToDirect:

								idx = pTangent->GetIndexArray().GetAt(vCnt);
								tan = pTangent->GetDirectArray().GetAt(idx);

								break;

							default:

								break;

							}
						}

					}
					else
					{
						DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&m_pVertexData[vCnt - 3].position);
						DirectX::XMVECTOR uv1 = DirectX::XMLoadFloat2(&m_pVertexData[vCnt - 3].uv);
						DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&m_pVertexData[vCnt - 2].position);
						DirectX::XMVECTOR uv2 = DirectX::XMLoadFloat2(&m_pVertexData[vCnt - 2].uv);
						DirectX::XMVECTOR pos3 = DirectX::XMLoadFloat3(&m_pVertexData[vCnt - 1].position);
						DirectX::XMVECTOR uv3 = DirectX::XMLoadFloat2(&m_pVertexData[vCnt - 1].uv);

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

						tan = FbxVector4(tangent.x, tangent.y, tangent.z);
					}
					tan.Normalize();
					for (int i = 1; i < 4; ++i)
					{
						m_pVertexData[vCnt - i].tangent.x = tan.mData[0];
						m_pVertexData[vCnt - i].tangent.y = tan.mData[1];
						m_pVertexData[vCnt - i].tangent.z = tan.mData[2];
					}
				}
				}

				v.uv.y = 1 - v.uv.y;
				m_pVertexData[vCnt] = v;
				++vCnt;


			}

			{
				DirectX::XMVECTOR pos1 = DirectX::XMLoadFloat3(&m_pVertexData[vCnt - 3].position);
				DirectX::XMVECTOR uv1 = DirectX::XMLoadFloat2(&m_pVertexData[vCnt - 3].uv);
				DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&m_pVertexData[vCnt - 2].position);
				DirectX::XMVECTOR uv2 = DirectX::XMLoadFloat2(&m_pVertexData[vCnt - 2].uv);
				DirectX::XMVECTOR pos3 = DirectX::XMLoadFloat3(&m_pVertexData[vCnt - 1].position);
				DirectX::XMVECTOR uv3 = DirectX::XMLoadFloat2(&m_pVertexData[vCnt - 1].uv);

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

				tan = FbxVector4(tangent.x, tangent.y, tangent.z);

				tan.Normalize();
				for (int i = 1; i < 4; ++i)
				{
					m_pVertexData[vCnt - i].tangent.x = tan.mData[0];
					m_pVertexData[vCnt - i].tangent.y = tan.mData[1];
					m_pVertexData[vCnt - i].tangent.z = tan.mData[2];
				}
			}
			triInfo.push_back(v);
		}
		//Meshlet 생성
		//1.정점을 q에서 pop.
		//2.소속된 삼각형 배열을 돌며 방문한적 없다면
		//현 MeshLet에 추가하고 나머지 정점들을 q에 push
		//3. MeshLet이 가득차면 재시작
		//4. 모든 삼각형이 처리될떄까지 반복
		//당장은 머티리얼 무시.오버드로우를 막기위해서라도 
		//머티리얼을 실시간으로 합치고 UV좌표를 다시 계산해야하는 기능을 추가해야한다.
		//삼각형 단위로 사용 머티리얼이 달라 질수도 있기 때문
		
		std::queue<int> q;
		std::vector<Meshlet*> meshLets;
		std::unordered_set<int> visited;
		Meshlet* mshlt=new Meshlet;
		int tri = -1;

		for (int i = 0; i < pMesh->GetPolygonCount(); ++i)
		{
			if(visited.find(i) != visited.end())
			{
				continue;
			}

			q.push(triInfo[i][0]);
			q.push(triInfo[i][1]);
			q.push(triInfo[i][2]);
			while (!q.empty())
			{
				int cur = q.front();
				q.pop();

				for (int j = 0; j < vToTri[cur].size(); ++j)
				{
					tri = vToTri[cur][j];
					if (visited.find(tri) == visited.end())
					{
						visited.insert(tri);
						if (mshlt->GetNumOfTri() == 85)
						{
							meshLets.push_back(mshlt);
							mshlt = new Meshlet;
							q = std::queue<int>();
						}

						mshlt->AddTris(tri);
						for (int k = 0; k < 3; ++k)
						{
							int next = triInfo[tri][k];
							if (cur != next)
							{
								q.push(next);
							}

						}
					}
				}
			}
			if (mshlt->GetNumOfTri())
			{
				meshLets.push_back(mshlt);
				mshlt = new Meshlet;
			}
		}

		int sum = 0;
		for (int i = 0; i < meshLets.size(); ++i)
		{
			sum += meshLets[i]->GetNumOfTri();
		}

		std::wstring wName(name.begin(), name.end());
		m_pMesh = new Mesh12(m_pDevice, m_pCommandList, m_pHeapManager,
			meshLets, m_pVertexData, m_pIndices, vertexDataPos, indicesPos,
			(wchar_t*)wName.c_str(), matNames);
		m_pMeshes.push_back(m_pMesh);
	}
	bool Importer12::LoadFbx(LPCWSTR pFileName, ID3D12Device* const pDevice)
	{
		std::unordered_set<FbxSurfaceMaterial*> objectMatSet;

		std::wstring wfilePath(pFileName);
		std::string filePath = std::string(wfilePath.begin(), wfilePath.end());
		wchar_t* pWchFileName = GetFileName(pFileName);
		std::wstring wfileName(pWchFileName);

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
		FbxAxisSystem axis = scene->GetGlobalSettings().GetAxisSystem();
		if (pFbxRootNode)
		{

			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(0);
			FbxNodeAttribute::EType AttributType =
				pFbxChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributType == FbxNodeAttribute::eNull)
			{

				TraverseNode(pFbxChildNode, pDevice, filePath, objectMatSet);
			}
			else
			{
				TraverseNode(pFbxRootNode, pDevice, filePath, objectMatSet);
			}
			
			m_pTexSrvs.reserve(m_pTexs.size());


			//Gen SRV
			for(int i=0;i<m_pTexs.size();++i)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Format = m_pTexs[i]->GetDesc().Format;
				srvDesc.Texture2D.MipLevels = m_pTexs[i]->GetDesc().MipLevels;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;


				D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = m_pHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = m_pHeapManager->GetCurCbvSrvGpuHandle();
				pDevice->CreateShaderResourceView(m_pTexs[i], &srvDesc, srvCpuHandle);
				m_Srv = srvGpuHandle;
				m_pTexSrvs.push_back(m_Srv);
				m_pHeapManager->IncreaseCbvSrvHandleOffset();
			}
			m_pObject = new Object12(m_pMeshes, m_materialInfos, m_pTexSrvs, m_pTexs,
				(wchar_t*)wfileName.c_str(),
				eFileType::FBX, m_matHash, m_texHash);
			ClearObject();
		}

		return true;
	}

	bool Importer12::TraverseNode(FbxNode* const pFbxNode, ID3D12Device* const pDevice, const std::string& filePath, 
		std::unordered_set<FbxSurfaceMaterial*>& objectMatSet)
	{
		for (int i = 0; i < pFbxNode->GetChildCount(); ++i)
		{
			FbxNode* pFbxChildNode = pFbxNode->GetChild(i);

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

			std::string name(pFbxChildNode->GetName());
			FbxAMatrix wMat = pFbxChildNode->EvaluateGlobalTransform();

			FbxVector4 translation = pFbxChildNode->GeometricTranslation.Get();
			FbxVector4 rotation = pFbxChildNode->GeometricRotation.Get();
			FbxVector4 scailing = pFbxChildNode->GeometricScaling.Get();
			FbxAMatrix geoMat;
			geoMat.SetT(translation);
			geoMat.SetR(rotation);
			geoMat.SetS(scailing);

			wMat = wMat * geoMat;


			std::unordered_set<FbxSurfaceMaterial*> localMatSet;
			std::vector<std::string> matNames;
			std::vector<unsigned int> submeshStride;
			std::vector<unsigned int> vertexDataPos;
			std::vector<unsigned int> indicesPos;

			FbxMesh* pMesh = reinterpret_cast<FbxMesh*>(pFbxChildNode->GetNodeAttribute());
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
						if (objectMatSet.find(pSurfaceMaterial) == objectMatSet.end())
						{
							objectMatSet.insert(pSurfaceMaterial);
							MaterialInfo matInfo;
							matInfo.material = LoadFbxMaterial(pSurfaceMaterial);
							LoadFbxTex(filePath, pSurfaceMaterial, &matInfo, matNames, pDevice);
						}
						else
						{
							FbxString matNameFStr = pSurfaceMaterial->GetNameOnly();
							std::string matName(matNameFStr.Buffer());
							matNames.push_back(matName);
						}
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
									//group에서 처음 보는 머티리얼 발견시
									if (objectMatSet.find(pSurfaceMaterial) == objectMatSet.end())
									{
										MaterialInfo matInfo;
										objectMatSet.insert(pSurfaceMaterial);
										localMatSet.insert(pSurfaceMaterial);
										matInfo.material = LoadFbxMaterial(pSurfaceMaterial);
										LoadFbxTex(filePath, pSurfaceMaterial, &matInfo, matNames, pDevice);
										submeshStride.push_back(i);

									}
									//현 mesh에서 처음 보는 머티리얼 발견시
									else if (localMatSet.find(pSurfaceMaterial) == localMatSet.end())
									{
										localMatSet.insert(pSurfaceMaterial);
										FbxString matNameFStr = pSurfaceMaterial->GetNameOnly();
										std::string matName(matNameFStr.Buffer());
										matNames.push_back(matName);
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
			//FBX가 Map을 담고 있을 경우를 대비하여 LoadFBX는 Loop문으로 LoadSubFbx(subMesh)를 호출하고, Object을 만들도록한다.
			LoadSubFbx(pMesh, pVertices, vertexDataPos, indicesPos, submeshStride, matNames, name, wMat);
			ClearMesh();
		}
		return true;
	}

	Importer12::~Importer12()
	{
		ClearMesh();
		if (m_pCurDir != nullptr)
		{
			delete m_pCurDir;
			m_pCurDir = nullptr;
		}
		
		m_fbxIOsettings->Destroy();
		m_fbxManager->Destroy();
	}
	void Importer12::ClearObject()
	{
		m_nObject = 0;
		m_pMeshes.clear();
		m_materialInfos.clear();
		m_pTexSrvs.clear();
		m_pTexs.clear();
		m_matHash.clear();
		m_texHash.clear();
	}
	void Importer12::ClearMesh()
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

		m_nVertex = 0;
		m_nIndex = 0;
		m_nVertexVec = 0;
		m_nTexVec = 0;
		m_nNormalVec = 0;

	}
}

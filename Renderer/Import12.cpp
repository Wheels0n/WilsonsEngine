#pragma once
#include <cwchar>
#include <filesystem>
#include <d3d12.h>
#include <DirectXTex.h>
#include "D3D12.h"
#include "Import12.h"
#include "DescriptorHeapManager.h"

namespace wilson
{
	ULONG Importer12::g_vertexVecCount = 0;
	ULONG Importer12::g_texVecCount = 0;
	ULONG Importer12::g_normalVecCount = 0;

	Importer12::Importer12(D3D12* pD3D12)
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

		m_pD3D12 = pD3D12;
		m_pDevice = m_pD3D12->GetDevice();
		m_pDescriptorHeapManager= m_pD3D12->GetDescriptorHeapManager();
		m_pCommandList = m_pD3D12->GetCommandList();
		m_pImporterCommandList = nullptr;
		m_pImporterCommandAllocator = nullptr;

		
		HRESULT hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pImporterCommandAllocator));
		assert(SUCCEEDED(hr));
		m_pImporterCommandAllocator->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("Importer12::m_pImporterCommandAllocator") - 1, "Importer12::m_pImporterCommandAllocator");

		hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pImporterCommandAllocator, nullptr, IID_PPV_ARGS(&m_pImporterCommandList));
		assert(SUCCEEDED(hr));
		m_pImporterCommandList->SetPrivateData(WKPDID_D3DDebugObjectName,
			sizeof("D3D12::m_pImporterCommandList") - 1, "D3D12::m_pImporterCommandList");
	}
	void Importer12::GetExtension(char* dst, const char* src)
	{
		int i = 0;
		while (true)
		{
			++i;
			if (src[i] == '.')
			{
				++i;
				break;
			}
		}

		for (int j = 0; j < 3; ++j, ++i)
		{
			dst[j] = src[i];
		}
		dst[3] = '\0';
		return;
	}
	std::streampos Importer12::GetCnts(LPCWSTR fileName, std::streampos pos, std::string& objName)
	{
		std::string line;
		std::ifstream fin;
		std::streampos lastPos;
		char ch;

		fin.open(fileName, std::ios_base::binary);
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
	void Importer12::LoadSubOBJ(LPCWSTR fileName, std::streampos pos, ID3D12Device* pDevice, std::string& objName)
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
			if (type == 'o')//off�� �ȵ�
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
							//��������ǥ�迡�� �޼���ǥ���
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
						DirectX::XMVECTOR pos3 = DirectX::XMLoadFloat3(&m_pVertexData[m_indexCount - 1].position);
						DirectX::XMVECTOR uv3 = DirectX::XMLoadFloat2(&m_pVertexData[m_indexCount - 1].uv);

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
				std::getline(fin, line, ' ');
				std::getline(fin, line);
				matNames.push_back(line);

				int idx = m_matHash[line];
				hasNormal = !m_MaterialInfoV[idx].normalMap.empty();
				vertexDataPos.push_back(m_indexCount);
				indicesPos.push_back(m_indexCount);
			}
			else if (type == 's')
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
		DirectX::XMVECTOR zeroV = DirectX::XMVectorZero();
		m_pModel = new Model12(m_pDevice, m_pCommandList, m_pDescriptorHeapManager, 
			m_pVertexData, m_pIndices, vertexDataPos, indicesPos, (wchar_t*)wobjName.c_str(), matNames);
		m_pModels.push_back(m_pModel);
		++m_objectCount;

		fin.close();

	}
	bool Importer12::LoadOBJ(LPCWSTR fileName, ID3D12Device* pDevice)
	{
		std::ifstream fin;
		fin.open(fileName, std::ios::binary);

		if (fin.fail())
		{
			return false;
		}

		LoadMTL(m_mtlPath, pDevice);

		std::string line;
		std::string objName;
		std::streampos curPos, nextPos;
		while (!fin.eof())
		{
			std::getline(fin, line, ' ');
			if (line.compare("o") == 0)
			{
				curPos = fin.tellg();
				nextPos = GetCnts(fileName, curPos, objName);
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

			D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
			D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();

			pDevice->CreateShaderResourceView(m_pTexs[i], &srvDesc, srvCpuHandle);
			m_SRV = srvGpuHandle;
			m_pTexSrvs.push_back(m_SRV);
			m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
		}
		m_pModelGroup = new ModelGroup12(m_pModels, m_MaterialInfoV, m_pTexSrvs, m_pTexs, 
			m_fileName, eFileType::OBJ, m_matHash, m_texHash);

		ClearModel();
		ClearModelGroup();
		g_vertexVecCount = 0;
		g_texVecCount = 0;
		g_normalVecCount = 0;
		return true;
	}
	bool Importer12::LoadModel(const char* extension, LPCWSTR fileName, ID3D12Device* pDevice)
	{
		GetCurDir(fileName);
		int len = wcslen(fileName);
		m_fileName = new wchar_t[len + 1];
		wcscpy(m_fileName, fileName);
		m_fileName[len] = L'\0';
		m_fileName = GetFileName(m_fileName);
		m_mtlPath = GetMTLPath(m_curDir, m_fileName);//���ϸ��� subMesh���� �ݵ�� ��ġx
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
	bool Importer12::LoadMTL(wchar_t* fileName, ID3D12Device* pDevice)
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
		std::getline(fin, line, ':');//�� ��° �࿡ ��Ƽ���� ������ ǥ�õ�
		std::getline(fin, line);
		int matCnt = atoi(line.c_str());
		m_MaterialInfoV.reserve(matCnt);
		std::getline(fin, line);


		while (!fin.eof())
		{
			std::getline(fin, line, ' ');

			if (line.compare("newmtl") == 0)
			{
				MaterialInfo mat = { "","","","",{0,} };
				std::getline(fin, line);
				std::string matName = line;
				m_matHash[matName] = m_MaterialInfoV.size();
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

							LoadTex(mapPath, pDevice, isDiffuse);
							delete[] mapPath;
						}

					}
					m_MaterialInfoV.push_back(mat);
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

	FbxAMatrix Importer12::GetNodeTransfrom(FbxNode* pNode)
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

	void Importer12::GetCurDir(LPCWSTR fileName)
	{
		std::wstring::size_type pos = std::wstring(fileName).find_last_of(L"\\");
		std::wstring curDirWStr = std::wstring(fileName).substr(0, pos);
		m_curDir = new wchar_t[curDirWStr.size() + 1];
		m_curDir = wcscpy(m_curDir, curDirWStr.c_str());
		//�ι��� ������ �����ϱ� ���� �߰�
		m_curDir[curDirWStr.size()] = L'\0';
	}
	wchar_t* Importer12::GetMTLPath(LPCWSTR filePath, wchar_t* tok)
	{
		int len = wcslen(filePath);
		len += wcslen(tok);
		len += 7; //.mtl\0 + // + 
		wchar_t* ptr = nullptr;
		wchar_t* tmp = new wchar_t[len];
		tmp = wcscpy(tmp, filePath);
		tmp = wcstok(tmp, L".", &ptr);
		tmp = wcsncat(tmp, L"\\", 2);
		//strcat�� null���� �ִ� ���� �׳� �̾��! �޸� �ʰ� ���� ����
		wchar_t* mtlPath = wcsncat(tmp, tok, wcslen(tok));
		mtlPath = wcsncat(mtlPath, L".mtl\0", 5);
		return mtlPath;
	}
	wchar_t* Importer12::GetFileName(LPCWSTR fileName)
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
	bool Importer12::LoadFbxTex(std::string fileName, FbxSurfaceMaterial* pSurfaceMaterial,
		MaterialInfo* pMatInfo, std::vector<std::string>& matNames, ID3D12Device* pDevice)
	{
		std::filesystem::path fbxPath = fileName.c_str();
		std::string texturesPath = fbxPath.parent_path().string() + "\\";

		const char* mapTypes[5] = { FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sNormalMap, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sSpecular,
			FbxSurfaceMaterial::sSpecularFactor };//reflectioFactor==metallic
		for (int i = 0; i < 5; ++i)
		{

			//FBX�� C++������� ���ٴ� FbxProperty�� �̿��Ѵ�.
			//�ؽ��Ĵ� ���� �Ǵ� �Ӽ�(ex:��ǻ��)�� �ҽ� ������Ʈ�� �����Ѵ�.
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
							
							D3D12_RESOURCE_DESC	texDesc = {};
							//4byte��� ����
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

							D3D12_HEAP_PROPERTIES heapProps = {};
							heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
							heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
							heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
							heapProps.CreationNodeMask = 1;
							heapProps.VisibleNodeMask = 1;

							hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
								nullptr, IID_PPV_ARGS(&pTex));
							assert(SUCCEEDED(hr));
							pTex->SetPrivateData(WKPDID_D3DDebugObjectName,
								sizeof("Importer12:: pTex") - 1, "Importer12:: pTex");
							
							ID3D12Resource* pUploadCB = nullptr;
							m_pD3D12->UploadTexThroughCB(texDesc, rowPitch, pData, pTex, &pUploadCB, m_pImporterCommandList);
							
							D3D12_RESOURCE_BARRIER copyDstToSrv = {};
							copyDstToSrv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
							copyDstToSrv.Transition.pResource = pTex;
							copyDstToSrv.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
							copyDstToSrv.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
							copyDstToSrv.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
							

							m_pImporterCommandList->ResourceBarrier(1, &copyDstToSrv);
							m_pImporterCommandList->Close();
							m_pD3D12->ExecuteCommandLists(&m_pImporterCommandList, 1);
							pUploadCB->Release();
							m_pImporterCommandAllocator->Reset();
							m_pImporterCommandList->Reset(m_pImporterCommandAllocator, nullptr);
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
		m_MaterialInfoV.push_back(*pMatInfo);
		FbxString nameFStr = pSurfaceMaterial->GetNameOnly();
		std::string name(nameFStr.Buffer());
		matNames.push_back(name);
		m_matHash[name] = m_MaterialInfoV.size() - 1;
		return true;
	}
	Material Importer12::LoadFbxMaterial(FbxSurfaceMaterial* pSurfaceMaterial)
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
	bool Importer12::LoadTex(LPCWSTR fileName, ID3D12Device* pDevice, bool isDiffuse)
	{
		
		ID3D12Resource* pTex;
		HRESULT hr;
		//Gen Tex
		{
			//Gen IconTexFromFile 

			DirectX::ScratchImage image;
			hr = DirectX::LoadFromWICFile(fileName, DirectX::WIC_FLAGS_NONE, nullptr, image);
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

			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			hr = pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(&pTex));
			assert(SUCCEEDED(hr));
			pTex->SetPrivateData(WKPDID_D3DDebugObjectName,
				sizeof("Importer12:: pTex") - 1, "Importer12:: pTex");
		}
		m_pTexs.push_back(pTex);
		return true;
	}
	void Importer12::LoadSubFbx(FbxMesh* pMesh, FbxVector4* pVertices,
		std::vector<UINT>& vertexDataPos, std::vector<UINT>& indicesPos, std::vector<UINT>& submeshStride, std::vector<std::string>& matNames, std::string& name,
		FbxAMatrix& wMat)
	{
		//pos�� ���� ������ ȭ��ó�� ���� ������ �̷�������� ��� ������ DrawCall�� �ϱ� ����
		int submeshCount = 0;
		for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
		{
			if (submeshStride.size() > 1 && submeshStride[submeshCount + 1] == j)
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
		FbxVector4 tan;

		int idx = -1;
		int idxCnt = 0;
		int vCnt = 0;
		for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
		{
			int verticesCnt = pMesh->GetPolygonSize(j);

			for (int k = 0; k < verticesCnt; ++k)
			{
				//�ε����� �����Ͽ� ���� ���������͸� ������� �����ϴϱ�, �ε������۴� ���������� �ȴ�. 
				int polygonV = pMesh->GetPolygonVertex(j, k);
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
					//���� ����� N���� ��Ұ� N���� �ڸ��� ������ eDirect, �ε��� �迭�� ���� �����ؾ��ϸ� eIndexToDirect
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
		}


		std::wstring wName(name.begin(), name.end());
		m_pModel = new Model12(m_pDevice, m_pCommandList, m_pDescriptorHeapManager, 
			m_pVertexData, m_pIndices, vertexDataPos, indicesPos,
			(wchar_t*)wName.c_str(), matNames);
		m_pModels.push_back(m_pModel);
	}
	bool Importer12::LoadFbx(LPCWSTR fileName, ID3D12Device* pDevice)
	{
		std::unordered_set<FbxSurfaceMaterial*> groupMatSet;

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
		FbxAxisSystem axis = scene->GetGlobalSettings().GetAxisSystem();
		if (pFbxRootNode)
		{

			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(0);
			FbxNodeAttribute::EType AttributType =
				pFbxChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributType == FbxNodeAttribute::eNull)
			{

				TraverseNode(pFbxChildNode, pDevice, filePath, groupMatSet);
			}
			else
			{
				TraverseNode(pFbxRootNode, pDevice, filePath, groupMatSet);
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


				D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvCpuHandle();
				D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = m_pDescriptorHeapManager->GetCurCbvSrvGpuHandle();
				pDevice->CreateShaderResourceView(m_pTexs[i], &srvDesc, srvCpuHandle);
				m_SRV = srvGpuHandle;
				m_pTexSrvs.push_back(m_SRV);
				m_pDescriptorHeapManager->IncreaseCbvSrvHandleOffset();
			}
			m_pModelGroup = new ModelGroup12(m_pModels, m_MaterialInfoV, m_pTexSrvs, m_pTexs,
				(wchar_t*)wfileName.c_str(),
				eFileType::FBX, m_matHash, m_texHash);
			ClearModelGroup();
		}

		return true;
	}

	bool Importer12::TraverseNode(FbxNode* pFbxNode, ID3D12Device* pDevice, std::string& filePath, std::unordered_set<FbxSurfaceMaterial*>& groupMatSet)
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
			FbxAMatrix wMat = pFbxChildNode->EvaluateGlobalTransform();//GetNodeTransfrom(pFbxChildNode);

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
						if (groupMatSet.find(pSurfaceMaterial) == groupMatSet.end())
						{
							groupMatSet.insert(pSurfaceMaterial);
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
									//group���� ó�� ���� ��Ƽ���� �߽߰�
									if (groupMatSet.find(pSurfaceMaterial) == groupMatSet.end())
									{
										MaterialInfo matInfo;
										groupMatSet.insert(pSurfaceMaterial);
										localMatSet.insert(pSurfaceMaterial);
										matInfo.material = LoadFbxMaterial(pSurfaceMaterial);
										LoadFbxTex(filePath, pSurfaceMaterial, &matInfo, matNames, pDevice);
										submeshStride.push_back(i);

									}
									//�� mesh���� ó�� ���� ��Ƽ���� �߽߰�
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

			//controlPoint�� ��� �������̴�. 
			FbxVector4* pVertices = pMesh->GetControlPoints();
			submeshStride.push_back(pMesh->GetPolygonCount());
			vertexDataPos.push_back(0);
			indicesPos.push_back(0);
			//FBX�� Map�� ��� ���� ��츦 ����Ͽ� LoadFBX�� Loop������ LoadSubFbx(subMesh)�� ȣ���ϰ�, ModelGroup�� ���鵵���Ѵ�.
			LoadSubFbx(pMesh, pVertices, vertexDataPos, indicesPos, submeshStride, matNames, name, wMat);
			ClearModel();
		}
		return true;
	}

	Importer12::~Importer12()
	{
		ClearModel();
		if (m_curDir != nullptr)
		{
			delete m_curDir;
			m_curDir = nullptr;
		}

		if (m_pImporterCommandAllocator != nullptr)
		{
			m_pImporterCommandAllocator->Release();
			m_pImporterCommandAllocator = nullptr;
		}

		if (m_pImporterCommandList != nullptr)
		{	
			m_pImporterCommandList->Release();
			m_pImporterCommandList = nullptr;
		}
		
		m_fbxIOsettings->Destroy();
		m_fbxManager->Destroy();
	}
	void Importer12::ClearModelGroup()
	{
		m_objectCount = 0;
		m_pModels.clear();
		m_MaterialInfoV.clear();
		m_pTexSrvs.clear();
		m_pTexs.clear();
		m_matHash.clear();
		m_texHash.clear();
	}
	void Importer12::ClearModel()
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
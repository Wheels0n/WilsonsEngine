#pragma once
#include <d3d12.h>
#include <DirectXTex.h>
#include "ResourceBarrierHandler.h"
#include "CommandListWrapperPool.h"
#include "D3D12.h"
#include "Import12.h"
#include "HeapManager.h"

namespace wilson
{
	ULONG Importer12::g_nVertexVec = 0;
	ULONG Importer12::g_nTexVec = 0;
	ULONG Importer12::g_nNormalVec = 0;

	Importer12::Importer12()
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

		m_pCurDir = nullptr;
		m_pFileName = L"";
		m_pObject = nullptr;
		m_pTangentVecs = nullptr;

		m_fbxManager = FbxManager::Create();
		m_fbxIOsettings = FbxIOSettings::Create(m_fbxManager, IOSROOT);
		m_fbxManager->SetIOSettings(m_fbxIOsettings);

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
	bool Importer12::LoadObject(const char* pExtension, const wchar_t* pAbsPath, ComPtr<ID3D12Device> pDevice)
	{
		GetCurDir(pAbsPath);
		int len = wcslen(pAbsPath);
		vector<wchar_t> pFileName(len+1);
		wcscpy(&pFileName[0], pAbsPath);
		pFileName[len] = L'\0';
		
		GetFileName(pFileName);
		if (!strcmp(pExtension, "fbx"))
		{
			return LoadFbx(pAbsPath);
		}
		else
		{
			return false;
		}
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

	void Importer12::GetCurDir(const wchar_t* pAbsPath)
	{
		wstring::size_type pos = wstring(pAbsPath).find_last_of(L"\\");
		wstring curDirWStr = wstring(pAbsPath).substr(0, pos);
		m_pCurDir = std::shared_ptr<wchar_t[]>(new wchar_t[curDirWStr.size() + 1]);
		wcscpy(m_pCurDir.get(), curDirWStr.c_str());
		m_pCurDir[curDirWStr.size()] = L'\0';
	}
	void Importer12::GetFileName(vector<wchar_t>& pFileName)
	{
		wchar_t* pTok = nullptr;
		wchar_t* ptr = nullptr;
		wchar_t* pTempRaw = wcstok(&pFileName[0], (const wchar_t*)L"\\", &ptr);
		while (true)
		{
			pTempRaw = wcstok(nullptr, (const wchar_t*)L"\\", &ptr);
			if (pTempRaw == nullptr)
			{
				break;
			}
			else
			{
				pTok = pTempRaw;
			}

		}

		pTempRaw = wcstok(pTok, L".", &ptr);
		m_pFileName = wstring(pTempRaw);
	}
	bool Importer12::LoadFbxTex(const string fileName, FbxSurfaceMaterial* const pSurfaceMaterial,
		MaterialInfo* const pMatInfo, vector<string>& matNames)
	{
		filesystem::path fbxPath = fileName.c_str();
		string texturesPath = fbxPath.parent_path().string() + "\\";

		const char* mapTypes[5] = { FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sNormalMap, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sSpecular,
			FbxSurfaceMaterial::sSpecularFactor };//reflectioFactor==metallic
		for (int i = 0; i < 5; ++i)
		{

			//FBX는 C++멤버변수 보다는 FbxProperty를 이용한다.
			//텍스쳐는 대응 되는 속성(ex:디퓨즈)에 소스 오브젝트로 존재한다.
			FbxProperty property = pSurfaceMaterial->FindProperty(mapTypes[i]);
			string materialName(mapTypes[i]);

			int layeredTexCount = property.GetSrcObjectCount(FbxCriteria::ObjectType(FbxLayeredTexture::ClassId));
			if (layeredTexCount == 0)
			{
				FbxFileTexture* texture = FbxCast<FbxFileTexture>(property.GetSrcObject(FbxCriteria::ObjectType(FbxFileTexture::ClassId), 0));
				int texCount = property.GetSrcObjectCount(FbxCriteria::ObjectType(FbxTexture::ClassId));
				if (texCount > 0)
				{
					HRESULT hr;
					string relativePath(texture->GetRelativeFileName());
					string path = texturesPath + relativePath;
					string name = string(texture->GetName());
					char extension[4];
					GetExtension(extension, path.c_str());
					if (m_texHash.find(name) == m_texHash.end())
					{
						wstring wPath = wstring(path.begin(), path.end());

						shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
						ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();
						shared_ptr<TextureWrapper> pTex=nullptr;
						if (strcmp(extension, "dds") == 0)
						{ 
							pTex = g_pTextureManager->LoadImageDDSFile(wPath.c_str(), pCmdList);

						}
						else
						{
							pTex = g_pTextureManager->LoadImageWICFile(wPath.c_str(), pCmdList);
						}
						SET_PRIVATE_DATA(pTex->tex);
						g_pResourceBarrierHandler->Push(
							g_pTextureManager->CreateBarrier(pTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

						g_pResourceBarrierHandler->Flush(pCmdList);
							

						g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
						g_pGraphicsCmdListPool->Push(pCmdListWrapper);
						
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
		string name(nameFStr.Buffer());
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
				XMVECTOR propertyVec = XMVectorSet(propertyDouble[0] * factor, propertyDouble[1] * factor, propertyDouble[2] * factor, 1.0f);
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
	void Importer12::LoadSubFbx(FbxMesh* const pMesh, FbxVector4* const pVertices,
		vector<UINT>& vertexDataPos, vector<UINT>& indicesPos,
		const vector<UINT>& submeshStride, const vector<string>& matNames, const string& name,
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
		
		m_pIndices = shared_ptr<ULONG[]>(new ULONG[m_nIndex]);
		m_pVertexData = shared_ptr<VertexInfo[]>(new VertexInfo[m_nVertex]);
		FbxVector4 tan;
		
		
		int idx = -1;
		int idxCnt = 0;
		int vCnt = 0;
		unordered_map<int, vector<int>> vToTri;
		vector<vector<int>> triInfo;
		//정점 데이터 파싱
		//FBX데이터에서는 정점 번호로 바로 가져옴. 근데 저장할떄는 정점이 겹쳐도  
		//TBN계산떄문에 그냥 VCnt로 마구잡이로 넣었었음
		//삼각형idx+offset으로 index 설정. 정점데이터는 나오는 순서대로 그냥 집어넣기
		for (int j = 0; j < pMesh->GetPolygonCount(); ++j)
		{
			int verticesCnt = pMesh->GetPolygonSize(j);
			vector<int> v;
			for (int k = 0; k < verticesCnt; ++k)
			{ 
				//인덱스를 참조하여 나온 정점데이터를 순서대로 저장하니까, 인덱스버퍼는 오름차순이 된다. 
				int polygonV = pMesh->GetPolygonVertex(j, k);
				vToTri[polygonV].push_back(j);
				v.push_back(polygonV);

				m_pIndices[idxCnt] = idxCnt;
				++idxCnt;

				VertexInfo v;
				FbxVector4 pos = pVertices[polygonV].mData;
				pos = wMat.MultT(pos);

				v.position = XMFLOAT3(
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
						XMVECTOR pos1 = XMLoadFloat3(&m_pVertexData[vCnt - 3].position);
						XMVECTOR uv1 = XMLoadFloat2(&m_pVertexData[vCnt - 3].uv);
						XMVECTOR pos2 = XMLoadFloat3(&m_pVertexData[vCnt - 2].position);
						XMVECTOR uv2 = XMLoadFloat2(&m_pVertexData[vCnt - 2].uv);
						XMVECTOR pos3 = XMLoadFloat3(&m_pVertexData[vCnt - 1].position);
						XMVECTOR uv3 = XMLoadFloat2(&m_pVertexData[vCnt - 1].uv);

						XMFLOAT3 e1;
						XMStoreFloat3(&e1, XMVectorSubtract(pos2, pos1));
						XMFLOAT3 e2;
						XMStoreFloat3(&e2, XMVectorSubtract(pos3, pos1));
						XMFLOAT2 dUV1;
						XMStoreFloat2(&dUV1, XMVectorSubtract(uv2, uv1));
						XMFLOAT2 dUV2;
						XMStoreFloat2(&dUV2, XMVectorSubtract(uv3, uv1));

						float det = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);
						XMFLOAT3 tangent;
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
				XMVECTOR pos1 = XMLoadFloat3(&m_pVertexData[vCnt - 3].position);
				XMVECTOR uv1 = XMLoadFloat2(&m_pVertexData[vCnt - 3].uv);
				XMVECTOR pos2 = XMLoadFloat3(&m_pVertexData[vCnt - 2].position);
				XMVECTOR uv2 = XMLoadFloat2(&m_pVertexData[vCnt - 2].uv);
				XMVECTOR pos3 = XMLoadFloat3(&m_pVertexData[vCnt - 1].position);
				XMVECTOR uv3 = XMLoadFloat2(&m_pVertexData[vCnt - 1].uv);

				XMFLOAT3 e1;
				XMStoreFloat3(&e1, XMVectorSubtract(pos2, pos1));
				XMFLOAT3 e2;
				XMStoreFloat3(&e2, XMVectorSubtract(pos3, pos1));
				XMFLOAT2 dUV1;
				XMStoreFloat2(&dUV1, XMVectorSubtract(uv2, uv1));
				XMFLOAT2 dUV2;
				XMStoreFloat2(&dUV2, XMVectorSubtract(uv3, uv1));

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
		
		queue<int> q;
		vector<shared_ptr<Meshlet>> meshLets;
		unordered_set<int> visited;
		shared_ptr<Meshlet> mshlt= make_shared<Meshlet>();
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
							mshlt = make_shared<Meshlet>();
							q = queue<int>();
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
				mshlt = std::make_shared<Meshlet>();
			}
		}

		int sum = 0;
		for (int i = 0; i < meshLets.size(); ++i)
		{
			sum += meshLets[i]->GetNumOfTri();
		}

		wstring wName(name.begin(), name.end());
		m_pMesh = make_shared<Mesh12>(D3D12::GetDevice(),
			meshLets, m_pVertexData, m_pIndices, vertexDataPos, indicesPos,
			(wchar_t*)wName.c_str(), matNames);
		m_pMeshes.push_back(m_pMesh);
	}
	bool Importer12::LoadFbx(const wchar_t* pAbsPath)
	{
		unordered_set<FbxSurfaceMaterial*> objectMatSet;
		wstring wfilePath(pAbsPath);
		string filePath =string(wfilePath.begin(), wfilePath.end());
	
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

				TraverseNode(pFbxChildNode, filePath, objectMatSet);
			}
			else
			{
				TraverseNode(pFbxRootNode, filePath, objectMatSet);
			}

			m_pObject = make_shared<MeshGroup12>(m_pMeshes, m_materialInfos,  m_pTexs,
				m_pFileName.c_str(),
				eFileType::FBX, m_matHash, m_texHash);
			ClearObject();
		}

		return true;
	}

	bool Importer12::TraverseNode(FbxNode* const pFbxNode,  const string& filePath, 
		unordered_set<FbxSurfaceMaterial*>& objectMatSet)
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


			unordered_set<FbxSurfaceMaterial*> localMatSet;
			vector<string> matNames;
			vector<unsigned int> submeshStride;
			vector<unsigned int> vertexDataPos;
			vector<unsigned int> indicesPos;

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
							LoadFbxTex(filePath, pSurfaceMaterial, &matInfo, matNames);
						}
						else
						{
							FbxString matNameFStr = pSurfaceMaterial->GetNameOnly();
							string matName(matNameFStr.Buffer());
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
										LoadFbxTex(filePath, pSurfaceMaterial, &matInfo, matNames);
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
		m_pCurDir = nullptr;
		
		m_fbxIOsettings->Destroy();
		m_fbxManager->Destroy();
	}
	void Importer12::ClearObject()
	{
		m_nObject = 0;
		m_pMeshes.clear();
		m_materialInfos.clear();
		m_pTexs.clear();
		m_matHash.clear();
		m_texHash.clear();
	}
	void Importer12::ClearMesh()
	{
		
		m_pVertexVecs = nullptr;
		m_pTexVecs = nullptr;
		m_pNormalVecs = nullptr;
		m_pVertexData = nullptr;
		m_pIndices = nullptr;

		m_nVertex = 0;
		m_nIndex = 0;
		m_nVertexVec = 0;
		m_nTexVec = 0;
		m_nNormalVec = 0;

	}
}

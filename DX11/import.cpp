#include "Import.h"
#include <filesystem>
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
							m_pVertexData[m_indexCount].UV = m_pTexCoord[vt - 1];
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

		m_pModel = new Model(m_pVertexData, m_pIndices, m_vertexCount, m_indexCount,tok);
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

	Material Importer::LoadFbxMaterial(FbxSurfaceMaterial* pSurfaceMaterial)
	{
		Material mat = { 0, };
		const FbxProperty AmbientProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
		const FbxProperty AmbientFactorProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
		if (AmbientProperty.IsValid() && AmbientFactorProperty.IsValid())
		{
			FbxDouble3 ambientDouble = AmbientProperty.Get<FbxDouble3>();
			DirectX::XMVECTOR ambientVec = DirectX::XMVectorSet(ambientDouble[0], ambientDouble[1], ambientDouble[2], 1.0f);

			float factor = AmbientFactorProperty.Get<float>();
			ambientVec = DirectX::XMVectorMultiply(ambientVec, DirectX::XMVectorSet(factor, factor, factor, factor));
			mat.ambient = ambientVec;
		}
		const FbxProperty diffuseProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		const FbxProperty diffuseFactorProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
		if (diffuseProperty.IsValid() && diffuseFactorProperty.IsValid())
		{
			FbxDouble3 diffuseDouble = diffuseProperty.Get<FbxDouble3>();
			DirectX::XMVECTOR diffuseVec = DirectX::XMVectorSet(diffuseDouble[0], diffuseDouble[1], diffuseDouble[2], 1.0f);

			float factor = diffuseFactorProperty.Get<float>();
			diffuseVec = DirectX::XMVectorMultiply(diffuseVec, DirectX::XMVectorSet(factor, factor, factor, factor));
			mat.diffuse = diffuseVec;
		}
		const FbxProperty specularProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
		const FbxProperty specularFactorProperty = pSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
		if (specularProperty.IsValid() && specularFactorProperty.IsValid())
		{
			FbxDouble3 specularDouble = specularProperty.Get<FbxDouble3>();
			DirectX::XMVECTOR specularVec = DirectX::XMVectorSet(specularDouble[0], specularDouble[1], specularDouble[2], 1.0f);

			float factor = specularFactorProperty.Get<float>();
			specularVec = DirectX::XMVectorMultiply(specularVec, DirectX::XMVectorSet(factor, factor, factor, factor));
			mat.specular = specularVec;
		};
		
		return mat;
	}
	bool Importer::LoadTex(Model* model, LPCWSTR fileName, ID3D11Device* pDevice)
	{
		HRESULT hr;
		hr = D3DX11CreateShaderResourceViewFromFileW(pDevice, fileName, nullptr, nullptr, &m_pSRV, nullptr);
		if (FAILED(hr))
		{
			return false;
		}

		model->SetTex(m_pSRV);
		return true;
	}

	bool Importer::LoadFbx(LPCWSTR fileName, ID3D11Device* pDevice)
	{	
		std::unordered_set<std::string> texSet;
		std::unordered_set<FbxSurfaceMaterial*> materialSet;
		std::vector<TextureData> texVec;
		std::vector<Material> materialVec;

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
											Material material = LoadFbxMaterial(pSurfaceMaterial);
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

		wchar_t* tok = TokenizeCWSTR(fileName);
		m_pModel = new Model(m_pVertexData, m_pIndices, vertexDataPos, indicesPos, materialVec, texVec, tok);
		return true;
	}

	Importer::~Importer()
	{
		Clear();
		m_fbxIOsettings->Destroy();
		m_fbxManager->Destroy();
	}

	void Importer::Clear()
	{
		//m_fbxImporter->Destroy();
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

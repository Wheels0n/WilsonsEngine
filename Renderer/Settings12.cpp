#include<DirectXTex.h>
#include "CommandListWrapperPool.h"
#include "Settings12.h"
#include "SSAOResourceManager.h"
#include "PostProcess.h"
#include "ImGuiManager.h"
#include "../ImGui/imgui.h"
namespace wilson
{
	const static float _ICON_SZ = 16.0f;
	const static float _PAD = 32.0f;
	Settings12::Settings12(shared_ptr<D3D12> pD3D12)
	{
		//D3D12
		m_FPS.Init();
		m_pD3D12 = pD3D12;
		m_pCam = m_pD3D12->GetCam();


		shared_ptr<CommandListWrapper> pCmdListWrapper = g_pGraphicsCmdListPool->Pop();
		ComPtr<ID3D12GraphicsCommandList> pCmdList = pCmdListWrapper->GetCmdList();

		shared_ptr<HeapManager> pHeapManager = m_pD3D12->GetHeapManager();

		HRESULT hr;
		//Gen DirLightIcon
		{
			//Gen DirIconTex
			{
				//Gen IconTexFromFile 
				m_pDirLitIconTex = g_pTextureManager->LoadImageWICFile(L"./Assets/Icons/sun-color-icon.png", pCmdList);
				SET_PRIVATE_DATA(m_pDirLitIconTex->tex);
			}
		}

		//Gen CubeLightIcon
		{
			{
				//Gen IconTexFromFile 
				m_pCubeLitIconTex = g_pTextureManager->LoadImageWICFile(L"./Assets/Icons/sun-color-icon.png", pCmdList);
				SET_PRIVATE_DATA(m_pCubeLitIconTex->tex);
			}
		}

		//Gen SpotLightIcon
		{
			{
				m_pSpotLitIconTex = g_pTextureManager->LoadImageWICFile(L"./Assets/Icons/flashlight-icon.png", pCmdList);
				SET_PRIVATE_DATA(m_pSpotLitIconTex->tex);
				//Gen IconTexFromFile 
			}
		}
		g_pGraphicsCmdListPool->Execute(pCmdListWrapper);
		g_pGraphicsCmdListPool->Push(pCmdListWrapper);

	}

	void Settings12::Draw()
	{
		if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_MenuBar))
		{
			std::shared_ptr<Frustum12> pFrustum = m_pCam->GetFrustum();
			m_FPS.Frame();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS:%d", m_FPS.GetFps());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "DrawCall:%d", m_pD3D12->GetNumDrawCall());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "InScene: %d", pFrustum->GetNumOfMeshletInScene());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "InFrustum: %d", pFrustum->GetNumOfMeshletInFrustum());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "HiZPassed: %d", m_pD3D12->GetNumSubMeshHiZPassed());
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "NotOccluded: %d", m_pD3D12->GetNumSubMeshNotOccluded());
			ImGui::Separator();

			ImGuiIO io = ImGui::GetIO();
			ImGui::Text("Mouse Pos: %g, %g", io.MousePos.x, io.MousePos.y);
			ImGui::Separator();
			if (ImGui::TreeNode("Camera"))
			{
				using namespace DirectX;
				static float DragFactor = 0.1f;

				BOOL bDirty = FALSE;

				ImGui::DragFloat("TranslationSpeed",	m_pCam->GetTranslationSpeed(), 0.1f, 10.0f);
				ImGui::DragFloat("RotationSpeed",		m_pCam->GetRotationSpeed(), 0.1f, 10.0f);
				
				if (ImGui::DragFloat("NearZ", m_pCam->GetNearZ(), 0.001f, 0.1f))
				{
					bDirty = TRUE;
				}
				if (ImGui::DragFloat("FarZ", m_pCam->GetFarZ(), 100.f, 100.f))
				{
					bDirty = TRUE;
				}

				XMVECTOR* curPosVec = m_pCam->GetPosition();
				XMFLOAT4 posFloat;
				XMStoreFloat4(&posFloat, *curPosVec);

				if (ImGuiManager::DrawFloat4(posFloat, "Position", *m_pCam->GetTranslationSpeed()))
				{
					bDirty = true;
				}
				
				posFloat.w=1.0f;
				*curPosVec = XMLoadFloat4(&posFloat);
				ImGui::PushID(0);
				if (ImGui::Button("Reset"))
				{
					m_pCam->ResetTranslation();
					bDirty = true;
				}

				ImGui::PopID();

				XMVECTOR* angleVec = m_pCam->GetRotation();
				XMFLOAT3 angleFloat;
				XMStoreFloat3(&angleFloat, *angleVec);
				//Rad to Degree
				//angleFloat.x = 57.2958f;
				//angleFloat.y = 57.2958f;

				if (ImGuiManager::DrawFloat3(angleFloat, "Rotation", *m_pCam->GetRotationSpeed()))
				{
					bDirty = true;
				}
				//Degree to Rad
				//angleFloat.x /= 57.2958f;
				//angleFloat.y /= 57.2958f;
				*angleVec = XMLoadFloat3(&angleFloat);
				ImGui::PushID(1);
				if (ImGui::Button("Reset"))
				{
					m_pCam->ResetRotation();
					bDirty = true;
				}

				if (bDirty)
				{	
					m_pCam->UpdateMatrices();
					m_pCam->SetDirty(true);
				}

				ImGui::Separator();
				ImGui::PopID();

				ImGui::TreePop();
			}

		}
		ImGui::End();

		if (ImGui::Begin("Light"))
		{
			ImGui::Image((ImTextureID)m_pDirLitIconTex->srv.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("DirectionalLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("dir", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image((ImTextureID)m_pCubeLitIconTex->srv.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("CubeLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("pnt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			ImGui::Image((ImTextureID)m_pSpotLitIconTex->srv.ptr, ImVec2(_ICON_SZ, _ICON_SZ));
			ImGui::SameLine(_PAD);
			ImGui::Text("SpotLight");
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("spt", nullptr, 0, ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}


		}
		ImGui::End();
		
		if (ImGui::Begin("PostProcess"))
		{
			shared_ptr<PostProcess> pPostProcess = m_pD3D12->GetPostProcess();
			if (ImGui::DragFloat("Exposure", pPostProcess->GetExposure(), 0.01f, 0.1f, 5.0f))
			{
				pPostProcess->UpdateExposureParemeters();
			}
		}
		ImGui::End();
		if (ImGui::Begin("SSAO"))
		{
			shared_ptr<SSAOResourceManager>pSSAOResourceManager = m_pD3D12->GetSSAOResourceManager();

			BOOL bDirty = false;
			if (ImGui::DragInt("SampleCount", reinterpret_cast<INT*>(pSSAOResourceManager->GetNumSsaoSample()), 1, 1, _NUM_SSAO_SAMPLE))
			{
				bDirty = true;
			}
			if (ImGui::DragFloat("Bias", pSSAOResourceManager->GetSsaoBias(), 0.01f, 0.1f, _SSAO_BIAS))
			{
				bDirty = true;
			}
			if (ImGui::DragFloat("Radius", pSSAOResourceManager->GetSsaoRadius(), 0.01f, 0.1f, _SSAO_RADIUS))
			{
				bDirty = true;
			}
			if (bDirty)
			{
				pSSAOResourceManager->UpdateSSAOParemeters();
			}
			
		}
		ImGui::End();
	}

}
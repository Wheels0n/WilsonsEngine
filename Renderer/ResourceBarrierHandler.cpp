#include "ResourceBarrierHandler.h"
namespace wilson
{
	shared_ptr<ResourceBarrierHandler> g_pResourceBarrierHandler = nullptr;

	void ResourceBarrierHandler::Push(D3D12_RESOURCE_BARRIER barrier)
	{
		HRESULT hr;
		hr = WaitForSingleObject(m_lock, INFINITE);
		assert(hr == WAIT_OBJECT_0);
		m_queue.push(barrier);
		ReleaseMutex(m_lock);
	}
	void ResourceBarrierHandler::Flush(ComPtr<ID3D12GraphicsCommandList> pCmdList)
	{
		HRESULT hr;
		hr = WaitForSingleObject(m_lock, INFINITE);
		assert(hr == WAIT_OBJECT_0);
		UINT n = m_queue.size();
		if (n)
		{
			vector<D3D12_RESOURCE_BARRIER> vec(n);
			for (int i = 0; i < n; ++i)
			{
				vec[i] = m_queue.front();
				m_queue.pop();
			}
			pCmdList->ResourceBarrier(n, &vec[0]);
		}
		
		ReleaseMutex(m_lock);
	}

	D3D12_RESOURCE_BARRIER ResourceBarrierHandler::CreateResourceBarrier(ComPtr<ID3D12Resource> pResource,
		const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = pResource.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = beforeState;
		barrier.Transition.StateAfter = afterState;
		return barrier;
	}
	D3D12_RESOURCE_BARRIER ResourceBarrierHandler::CreateResourceBarrier(ComPtr<ID3D12Resource> pResource,
		const D3D12_RESOURCE_STATES beforeState, const D3D12_RESOURCE_STATES afterState, const UINT mipLevel)
	{
		D3D12_RESOURCE_BARRIER barrier = CreateResourceBarrier(pResource, beforeState, afterState);
		barrier.Transition.Subresource = mipLevel;
		return barrier;
	}

	ResourceBarrierHandler::ResourceBarrierHandler()
	{
		m_lock = CreateMutex(NULL, FALSE, NULL);
	}
}
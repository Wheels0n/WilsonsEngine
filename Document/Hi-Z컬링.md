# Hi-Z 컬링

## 개요

Hierarchical-Z는 현대 GPU에서 칩 메모리에 있는 축소되고 압축된 깊이버퍼를  
이용하여 다수의 입력 픽셀 집합들을 걸러내면서 깊이테스트를 가속하는 기능이다.  
이 방식을 통해 기존의 전통적인 occlusion 쿼리 사용시 불가피했던 cpu 개입없이도  
gs를 통해 다수의 오브젝트들에 대해 occlusion culling을 할 수가 있다.

## H/W 컬링

오클루전 컬링은 뷰 프러스텀 내에는 존재하지만 다른 물체에 가려저 안 보이는 물체들을  
판단하는 가시성 판단 알고리즘이다. 이제는 GPU가 H/W에서 이러한 오클루전 쿼리를 지원  
한다. 방식은 간단하다.

> 쿼리가 활성화된 상태에서 오브젝트를 그리면 깊이 테스트를 통과한 샘플 수를 반환한다.

가시성 판단한답시고 한번 더 그린다는 게 우스울수도 있으나 실제로는 GPU의 일을 많이  
덜어 줄 수 있다. 만약 수 천개의 삼각형으로 이루어진 오브젝트가 있는데 가시성을 판단  
하고 싶다면 오브젝트 경계박스를 그리고 그것이 보인다면 그 오브젝트는 높은 확률로  
보인다는 뜻이다.

허나 이러한 쿼리방식은 경계박스를 이용하기떄문에 단지 '높은 확률'일뿐이지 완전히  
정확한 결과는 아니다. 그래서 어느 오클루전 알고리즘을 적용할떄는 효율과 효과사이에  
타협을 해야할 필요가 있다. 이제 적용할 방식은 좀 더 conservative하지만 큰 데이터들  
에대해서는 오클루전 컬링이 가능케한다.

HW방식은 유용하긴하나 App측에게 쿼리 관리와 쿼리가 가능할 때만 그리는 것까지 맡긴다.  
가장 막연한 방식은 오브젝트를 그리전에 쿼리를 실행하는 것이다. 그럴싸하지만 쿼리  
결과가 나올떄까지(available) CPU는 멈추며, 당연히 그사이 GPU 사이클은 낭비된다.  
이를 해결하기위해 App측에서 쿼리 실행과 결과를 기반으로한 오브젝트 그리기사이의 낭비되는  
시간을 채워야한다. 여러 방법이 있지만 구현이 더 복잡해진다. 또 다른 문제점으로는  
쿼리 사용시에 가시성 판단을 위해 여전히 CPU의 개입이 요구 된다는 것이다. 현대의 HW는  
배칭이 렌더링에 매우 중요한데 이러한 방법은 다소 비효율적이다.

우리가 사용할 방식은 GPU에게 오로지 가시성 판단을 맞기면서도 구현은 쉽다.

## 쿼리 힙

D3D12에서 쿼리는 쿼리 힙이라는 쿼리 배열로 그룹지어져 있다. 쿼리힙은 어떤 타입의 쿼리들을  
담는 지 정의하는 타입이 정해져있다. D3D12_QUERY_TYPE_BINARY_OCCLUSION 라는 것도 추가  
되었는데 D3D12_QUERY_TYPE_OCCLUSION와 동작은 같지만 1개라도 깊이 테스트를 성공한 샘플이  
있다면 1, 아니면 0을 반환한다고 한다.

힙 내의 각 원소는 개별로 시작/정지 될 수 있다. 쿼리로 부터 데이터를 추출하기 위해 ResolveQueryData를  
 사용해서 8바이트 배수로 버퍼에 저장하면 된다.  
근데 Default-Heap에서의 버퍼만 인자로 전달 가능하며 Map()을 쓸수는 없어서 ReadBack힙을  
만들어서 복사 후 결과를 가져옴.

DrawObject함수를 적당히 수정해서 HW Occlusion함수를 만들것. 쿼리시작과 정지사이에  
드로우콜을 꼭해야되서 이것도 워커쓰레드를 이용해서 최대한 빨리 그려줘야한다.  
통과한 것만 큐에 넣어서 PbrGeoPass에 넘긴다. 깊이 테스트만 필요하니 유의

참고로 SetPredication()은 드로우콜 직전에 쓰여서 결과에 따라 진짜 그릴지 말지를 설정하는  
명령어이다. 텍스쳐 아틀라스 적용전까지는 실제로 가려지는 파트가 있더라도 다른 파트가 그려지면  
일단 통과로 처리 함.

![1차 시도](HW오클루전%201차도입.JPG)  
기존에 12-13겨우 찎었던게 머티리얼 상관없이 1오브젝트 1드로우콜로 줄이고 오클루전 컬링으로  
그릴 오브젝트 수를 반으로 줄였음에도 불구하고 7-8로 줄었다.

![1차 시도-1](HW오클루전%201차도입-1.JPG)  
Pixe로 살펴보니 Resolve쿼리와 리소스배리어가 프러스텀 컬링을 통과한 개체마다 호출되어서 느려진다.  
이것들도 \_OBJECT_PER_THREAD만큼 작게 배칭시켜서 개선해야겠다. 쿼리 결과를 담는 버퍼를 크게 잡고  
포인터연산으로 결과 참조

![2차 시도](HW오클루전%202차도입.JPG)  
다시 11-12fps 정도로 복구되었다.

![2차시도-1](HW오클루전%202차도입-1.JPG)  
프레임 하나에 160ms에서 72ms로 줄긴 줄었다. 허나 여전히 듬성듬성 gpu가 놀고있어서 좀 걸린다.

## Hi-Z맵 생성

대부분의 GPU기반 컬링 알고리즘이 그렇듯, Hi-Z 맵 기반의 오클루전 컬링은 GS를 사용하여  
가려지는 primitive들의 출력을 막는다. 알고리즘 자체는 현대 GPU에 구현 된 계층적 Z 테스팅과  
궤를 함꼐 한다. Scene에서 가리는 물체들을 모두 그린뒤 깊이버퍼로 부터 계층적 깊이 버퍼를  
생성한다. 밉맵으로, 각 레벨i에서 텍셀은 i-1의 대응되는 텍셀들 중 최대 깊이를 담는다.

이후 각 오브젝트의 바운딩 볼륨의 깊이와 Hi-Z맵의 깊이 값을 비교하여 컬링을 실시한다.  
더 적은 텍스처 페칭으로 특정 밉레벨로부터 샘플링하여 깊이 비교가 가능하다. 이게 우리가  
최대 깊이 값을 사용하는 이유이다. reverse Z버퍼라면 최소값을 사용한다.

단일 샘플링 렌더링이라면 Hi-Z맵을 Scene을 그리는데에 깊이 버퍼로서 쓸 수 있다. 다중 샘플링  
의 경우에도 적용 가능하지만 다중 샘플 깊이 버퍼의 각 샘플의 최대 깊이를 계산하기위해 별도의  
풀스크린 패스가 요구되며 결과를 단일 샘플 Hi-Z맵에 저장한다. 다수의 프레임버퍼를 이용하여  
풀스크린 패스로 이전 밉레벨의 것을 입력으로 하고 현재 밉레벨을 출력으로 한다. 정사각형이 아닌  
텍스쳐들은 반올림 문제로 인해 뷰포트 크기가 최소 1x1이 되도록 하는 걸 잊어서는 안된다.  
PS는 한 가지만 뺴면 직관적인데 화면 비율이 떄문에 깊이 텍스쳐로 정사각형을 쓰지 않을 것이며  
정사각형이 아니면 다음 밉레벨의 크기를 결정하는 데에 floor가 기본으로 쓰이기 때문에 홀수 크기  
의 밉레벨에서 축소된 경우 가장자리의 텍셀을 반영해줘야한다.

```C
textureOffset(LastMip, TexCoord, ivec2(-1, 0))
```

uv좌표가 아닌 텍셀단위로 offset해주는 함수

Hi-z맵 생성 자체는 pre-z패스에서 나온 깊이 버퍼로부터 밉맵을 만들면 됨

```C++
{
    D3D12_RESOURCE_BARRIER copyZ[] =
    {
        CreateResourceBarrier(m_pSceneDepthTex, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE, 0),
        CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST, 0)
    };
    m_pMainCommandList->ResourceBarrier(sizeof(copyZ) / sizeof(D3D12_RESOURCE_BARRIER), copyZ);

    UINT mipLevels = 1 + (UINT)floorf(log2f(fmaxf(m_clientWidth, m_clientHeight)));
    UINT curWidth = m_clientWidth;
    UINT curHeight = m_clientHeight;

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource = m_pHiZTempTex;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource = m_pSceneDepthTex;
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = 0;

    m_pMainCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    D3D12_RESOURCE_BARRIER afterCopyZ[] =
    {
        CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET,0),
        CreateResourceBarrier(m_pSceneDepthTex, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE, 0)
    };

    m_pMainCommandList->ResourceBarrier(sizeof(afterCopyZ) / sizeof(D3D12_RESOURCE_BARRIER), afterCopyZ);

    for (int i = 1; i < mipLevels; ++i)
    {
        m_pMainCommandList->SetDescriptorHeaps(sizeof(ppHeaps) / sizeof(ID3D12DescriptorHeap*), ppHeaps);
        m_pMainCommandList->SetPipelineState(m_pGenHiZPassPso);
        m_pMainCommandList->SetGraphicsRootSignature(m_pShader->GetGenHiZpassRootSignature());
        m_pMainCommandList->SetGraphicsRootDescriptorTable(eGenHiZRP_ePs_sampler, m_ClampSSV);
        m_pMainCommandList->SetGraphicsRootDescriptorTable(eGenHiZRP_ePs_lastResoltion, m_ResolutionCBV);

        //텍스쳐 자체는 밉레벨 여러개로 구성하고 rtv와 srv만 resourceBarrier로 바꿔가며 렌더링
        D3D12_RESOURCE_BARRIER mipPrep[] =
        {
            CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, i-1),
            CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET, i)
        };
        m_pMainCommandList->ResourceBarrier(sizeof(mipPrep) / sizeof(D3D12_RESOURCE_BARRIER), mipPrep);
        m_pMainCommandList->SetGraphicsRootDescriptorTable(eGenHiZRP_ePs_lastMip, m_hiZTempSrvs[i-1]);

        UINT resolution[] = { curWidth, curHeight };
        memcpy(m_pResolutionCbBegin, resolution, sizeof(UINT) * 2);


        //viewport도 for문에서 동적으로 생성.
        D3D12_VIEWPORT viewPort = {};
        viewPort.Width = curWidth/2;
        viewPort.Height = curHeight/2;

        D3D12_RECT scissorRect = {};
        scissorRect.right = curWidth/2;
        scissorRect.bottom = curHeight/2;

        m_pMainCommandList->IASetVertexBuffers(0, 1, &m_QuadVBV);
        m_pMainCommandList->IASetIndexBuffer(&m_QuadIBV);
        m_pMainCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pMainCommandList->RSSetViewports(1, &viewPort);
        m_pMainCommandList->RSSetScissorRects(1, &scissorRect);
        m_pMainCommandList->OMSetRenderTargets(1, &m_hiZTempRtvs[i], FALSE, nullptr);
        m_pMainCommandList->DrawIndexedInstanced(_QUAD_IDX_COUNT, 1, 0, 0, 0);


        m_pMainCommandList->Close();
        m_pMainCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pMainCommandList);
        UINT fenceValue = m_fenceValue++;
        m_pMainCommandQueue->Signal(m_pFence, fenceValue);
        if (m_pFence->GetCompletedValue() < fenceValue)
        {
            m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
        m_pMainCommandAllocator->Reset();
        m_pMainCommandList->Reset(m_pMainCommandAllocator, nullptr);
        curWidth /= 2;
        curHeight /= 2;
    }
    D3D12_RESOURCE_BARRIER mipPrep=
    CreateResourceBarrier(m_pHiZTempTex, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, mipLevels - 1);
    m_pMainCommandList->ResourceBarrier(1, &mipPrep);
}
```

1. 먼저 깊이 버퍼 (mip0)에 대응되는 rtv텍스쳐에 복붙
2. rtv를 1x1까지 밉맵에 붙여 넣기 용으로 렌더(그 만큼 rendering..)
3. 다시 깊이버퍼 밉맵에 CopyTextureRegion(subresource지정가능)
4. 끝!

실행중에 화면 크기가 어떻게 될 지 모르니 vector로 서술자 핸들을 담음.  
miplevel만 설정해주면 하위 밉자체는 생성됨. 거기다 그려 넣는 건 app책임.

## Hi-Z맵 컬링

Hi-z맵 생성 후 2x2 크기의, 바운딩 볼륨이 차지하는 스크린 공간 주변의 텍셀들을 이용한다.  
먼저 바운딩 볼륨을 클립 공간으로 전환 해야한다. 이는 두 가지 용도가 있는 데

1. Hi-Z 맵에 쓸 텍스쳐 좌표
2. Hi-Z 맵의 LOD 판정

LOD 계산을 위해 바운딩 볼륨의 스크린 공간 크기를 계산 해야한다. 그 후 이 결과를 이용하여
LOD를 쉽게 도출 할 수가 있다.

스크린 공간 크기는 다음과 같이 계산한다.(바운딩 볼륨은 클립공간이라고 가정)

```C++
float ViewSizeX = (BoundingRect[1].x-BoundingRect[0].x) * Transform.Viewport.y;
float ViewSizeY = (BoundingRect[1].y-BoundingRect[0].y) * Transform.Viewport.z;
```

```C++
float LOD = ceil(log2(max(ViewSizeX, ViewSizeY) / 2.0));
```

끝으로 방금 구한 인자들을 통해 Hi-Z맵을 참조하여 네 텍셀들의 최댓값을 구하고 이 결과를
오브젝트의 깊이값과 비교하면 끝!

왜 다음 밉레벨을 안쓰고 굳이 2x2텍셀을 쓰는 지 의아 할 수도 있다. 4개의 텍셀을 쓰는 경우  
LOD판정이 쉬울뿐만 아니라 실제 물체를 더 잘 포함한다. 한 개짜리 텍셀의 경우 LOD판정이 더  
복잡하고 비쌀뿐만아니라 더 큰 LOD가 페치 되어야한다. 그리고 LOD가 4개의 텍셀+1텍셀로  
페치되는 걸로 결정이 안될 수가 있다. 극단적인 경우 만약 화면 정중앙에 있는 경우 가장 큰  
LOD를 가져와야 할 수도 있다.

![뷰포트 행렬](https://learn.microsoft.com/en-us/windows/win32/dxtecharts/images/d3dxfrm68.gif)

뷰포트 X = (Xn W+W)/2, 뷰포트 Y = (-Yn h+h)/2

### 기타 변경 사항

-모델 클래스에 부분이 아닌 전체 인덱스를 담는 IBV 변수 선언  
-pix를 통한 디버깅을 용이하게 하기 위해 #include <pix3.h>선언  
-PbrGeo패스아니면 머티리얼마다 따로 그리지말고 통째로 그리도록 변경  
-깊이버퍼를 hi-z컬링용으로 쓰기 위해 mipLevel을 10개정도 늘림.
-hi-z패스용 border샘플러 추가

#### 참조

[Hi-Z맵 설명](https://www.rastergrid.com/blog/2010/10/hierarchical-z-map-based-occlusion-culling/)  
[DX12 오클루전 쿼리-유영천](https://megayuchi.com/2016/03/16/d3d12%EC%97%94%EC%A7%84%EA%B0%9C%EB%B0%9C-d3dquery%EB%A5%BC-%EC%9D%B4%EC%9A%A9%ED%95%9C-occlusion-culling/)  
[DX12-오클루전 쿼리 - DirectXSpecs](https://microsoft.github.io/DirectX-Specs/d3d/CountersAndQueries.html#queries)  
[DX12-쿼리 예제 - MSDN](https://learn.microsoft.com/en-us/windows/win32/direct3d12/predication-queries)  
[DX12-Hi-z 컬링 - 유영천](https://www.slideshare.net/dgtman/hierachical-z-map-occlusion-culling)
[밉맵 설명](https://www.3dgep.com/learning-directx-12-4/#Compute_Shaders)  
[hi-z맵 CS+구](https://www.nickdarnell.com/hierarchical-z-buffer-occlusion-culling/)

# GPU 컨텍스트 최적화

렌더링 관련 글을 보면 불필요한 드로우 콜을 줄이기 위해서 batch시켜라는 말을 자주 언급된다.  
왜 그런지 알아보자. 지금 할 이야기는 모두 AMD의 GCN 아키텍쳐에 한하여 적용된다.

## GPU단에서의 컨텍스트란?

GPU프로그래밍을 하면 보통 컨텍스트는 GPU에 명령을 내려주는 API 메커니즘의 한 인스턴스를 가리킨다.  
이러한 컨텍스트에서 이는 최상위 소프트웨어 구조(construct)이며 게임이나 앱에서 GPU사용시에 오직  
하나만을 가지고 있다.(DX11의 그 컨텍스트를 뜻하는 듯하다.)

허나 이는 AMD GPU 컨텍스트 롤을 이야기 할 때의 그 컨텐스트가 아니다. 여기서의 GPU 하드웨어가  
정상적으로 그리기 위해 있어야할 현 상태에 대해 알고 있는 것을 뜻한다.

우리는 GPU상에서 각 그리기 상태의 각부분에 대해 조정이 가능하다. 예를 들어 어떤 순서로 정점을  
처리할지와 어떤 모드로 blend 작업을 할 것있지 등이다. 이러한 가변적인 GPU상태들은 파이프라인  
전체에 적용된다.

이제부터 컨텍스트라하면 '무언가를 제대로 그리기위해 요구되는 GPU 파이프라인의 상태'를 뜻한다고
보면된다.

## AMD GPU roll 메커니즘

이제 GPU 하드웨어 관려해서 알아보겠다. GCN구조에서 하드웨어는 동작 상태를 저장하기 위해서 8개의
컨텍스트 레지스터 뱅크를 유지한다. 하지만 1개는 clear용도고 나머지 7개도 이론적인 구조(construct)
이다. 실제 하드웨어 구현은 훨씬 복잡하다.

이러한 컨텍스트 레지스터들의 사본이 banked된 RAM들에 저장되는 되지만 단순히 하나의 물리적인 RAM에
다같이 저장된 게아니라 GPU 디자인에 걸쳐서 분산되어 있으며 이들을 필요로 하는 블록들에 저장되어 있다.
새 값 쓰기를 관리하고 칩 전반에 걸쳐 알아야 할 내용을 전파하며 컨텍스트 레지스터를 가진 각 블록들과  
소통하는 하드웨어 블록이 있다.

그러면 각 블록은 이 사본들을 관리할 책임이 있다. 그 관리 블록이 어떻게 구동되는 지는 모두 드라이버에 의해  
구동되는 Command Processor(CP)에 달려있다.

roll은 CP가 관리 블록에 제대로 그리기 위해 파이프라인 상태가 바뀌어야하니 새로운 레지스터 집합에 대해 작업을
개시하라는 요청을 하는 순간을 뜻한다. 그리고 이는 각 블록에 현 레지스터들의 사본을 빈(free) 뱅크에 넣도록 요청하여 새로운 레지스터 값을 받아들일 준비를 시킨다.

## 드라이버 롤 메커니즘

CP는 자동으로 상태를 관리하는 게 아니라 드라이버가 시키는 대로만 작동할 뿐이다. 다행인건 8개월 전과 달리
지금 이 포스트를 작성하는 시점에서는 Vulkan 드라이버와 이것의 기반이 되는 저수준 플랫폼 추상화 계층(PAL)이
오픈 소스화 되었다는 것이다.

PAL은 CP에게 GPU를 제대로 그릴 수 있는 상태로 만들 것을 명령할 것을 담당한다. 그리고 패킷화 된 명령어 포맷
PM4라는 것을 이용하여 그렇게 한다. PM4가 여기서 언급하기에는 워낙 불투명한지라 궁금하다면 소스코드를 읽어보라

컨텍스트 롤을 이해하기 위해 직접 가서 볼 필요까지는 없다만 궁금하면 깃헙에서 찾아봐라. 예를 들어 EOP나 'CMD
Util::BuildLoadContextRegs()같은 걸 확인해보아라. 이 함수가 바로 컨텍스트 레지스터를 프로그램하는 'LOAD_CONTEXT_REG4' opcode를 사용하는 PM4 패킷을 만드는 함수이다.

그래서 GPU 파이프라인 상태에 변화가 생기면 이러한 요청들을 받아들이는 드라이버의 유저모드에서 PAL로 요청을  
보낸다. 그러면 PAL이 PM4 패킷을 올바른 레지스터 범위와 함께 올바른 시점에 컨텍스트 상태 갱신이 이루어지게끔
만든다.

## 성능 영향

이제 GPU가 블록 단위로 상태를 관리하고 CP가 어떻게 명령을 받고 새로운 그리기를 개시 하는 지 알았다.
그러면 성능에는 무슨 영향을 끼치는 가?

새로운 컨텍스트를 롤 할 여분의 논리적 컨텍스트 레지스터가 있다면 성능의 영향은 없다. 허나 7개 모두 이미  
그리기 작업 중이라면? 만약 그리기가 충분한 작업을 담고 있다면 여전히 영향은 없다. 그리기가 GPU파이프라인을
쥐어 짜는 동안에 GPU는 아마 아직도 바쁠 것이고 관련된 EOP 작업이 발생하면 논리 컨텍스트가 비게 되고 그러므로  
CP가 새로운 그리기를 위해 새로운 컨텍스트를 롤한다.

하지만 그리기에 작업이 충분하지 않다면 GPU가 충분히 바쁘지 않아서 stall로 인한 오버헤드에 빠지게 되고
결국 새로운 빈 논리 컨텍스트가 없어지게 된다. 잘만든 게임이라면 그런 일이 없다.

이러한 컨텍스트 기아 현상을 피하기 위해서는 GPU에 어떻게 명령을 내리는 지에 대해 민감해져야한다. 주로  
배치를 통한다. 같은 재질 이나 렌더 상태를 통해 최대한 그리기를 배치 시킨뒤 그리기를 제출 해야한다.

## 테스트

지금은 RDNA가 더 최신이라 GCN관련 툴들은 공식 홈페이지에서 안보인다. 그래서 구글링을 해서 구버전을  
다운 받았다.

![컨텍스트 설정 중복](AMD%20GPU프로파일러%20컨텍스트%20롤.JPG)

드로우 콜 과 컨텍스트 수는 물론이고 PSO 설정 관련하여 중복되어 불필요한 동작이였는 지여부까지 퍼센트로  
표기해준다. 죄다 90퍼 이상 불필요했다고 나타났다. 한번 갈아 엎어보자.

## 최적화 작업 - 1차시도

lastParentMesh 변수를 통해 이전과 차이가 없다면 VB/IB만 변경한 뒤 그린다. 근데 문제는 이 그리기가  
멀티 스레딩을 통하기 떄문에 어떻게 해결할지가 문제다. 해당 변수 관리는 그렇다 치고 만약 중간에 다른  
메시를 그리게 될 떄가 문제다. 이러면 이전 컨텍스트로 명령내리는 스레드와 새 컨텍스트로 명령 내리는 스  
레드가 번갈아 작업을 내리는 불상사가 생길 수 있다.

그래서 다음과 같이 그리기 작업 방식을 변경하기로 했다.

1. 같은 컨텍스트 단위로 커맨드 리스트를 빠르게 기록 한다.
2. 마지막에 끝난 스레드가 모든 커맨드 리스트를 모아서 Execute한다.
3. 그전에 작업이 끝난 다른 스레드들은 기다리지 않고 다른 커맨드 리스트  
   에다가 새로운 컨텍스트로 작업을 기록한다.
4. 남은 메시에 1-3의 작업을 반복한다.

스레드당 커맨드리스트를 7개로 바꾸었다. 당장은 전보다 GPU에 일이 쏠리는 건 감안하고 하겠다.

매 패스 실행 직전에 메시가 바뀌는 색인을 벡터로 기록 했다.

```c++
	void D3D12::UpdateContextOffset(std::vector<Meshlet*>& meshQueue)
	{
		if (meshQueue.empty())
		{
			return;
		}
		m_contextOffsets = std::vector<UINT>();
		sort(meshQueue.begin(), meshQueue.end());
		Mesh12* pLastMesh = meshQueue[0]->GetParentMesh();
		for (int i = 1; i < meshQueue.size(); ++i)
		{
			Mesh12* pCurMesh= meshQueue[i]->GetParentMesh();
			if (pLastMesh != pCurMesh)
			{
				m_contextOffsets.push_back(i);
				pLastMesh = pCurMesh;
			}
		}

		m_contextOffsets.push_back(meshQueue.size());
	}
```

다음 offset보다 안 커질 만큼만 일감 알아서 가져 가도록 하였다. 가져올수 있는 메시렛이 0개면  
그떄 offset을 증가 시키고 현재 conext에 대한 작업들을 모두 Execute한다. 그리고 다음 컨텍스트로  
넘어간다. 물론 이방법은 잘못되었다. 아직 다 기록 했는 지도 모르는 데 임의로 커맨드 리스트를 닫았다.

```c++
UINT nMeshes =min(m_contextOffsets[m_curContextOffset] - m_lastMeshletIdx, _OBJECT_PER_THREAD);

if (nMeshes==0)
{
   newContext = true;
   m_curContextOffset++;
   nMeshes = min(m_contextOffsets[m_curContextOffset] - m_lastMeshletIdx, _OBJECT_PER_THREAD);
   ID3D12CommandList* workerCommandLists[_WORKER_THREAD_COUNT];
   for (int k = 0; k < _WORKER_THREAD_COUNT; ++k)
   {
      workerCommandLists[k] = m_pWorkerCommandLists[k][m_curContext].Get();
      m_pWorkerCommandLists[k][m_curContext]->Close();
   }

   m_pMainCommandQueue->ExecuteCommandLists(_WORKER_THREAD_COUNT, workerCommandLists);
   for (int k = 0; k < _WORKER_THREAD_COUNT; ++k)
   {
      UINT fenceValue = m_workerFenceValues[k][m_curContext]++;
      m_pMainCommandQueue->Signal(m_pWorkerFences[k][m_curContext].Get(), fenceValue);
   }

   m_curContext = (m_curContext + 1) % _CONTEXT_COUNT;
}
```

다른 스레드에서 컨텍스트가 변경된걸 확인하면 다른 커맨드리스트를 써야하는 데 아직 실행중이라면
대기한다음 Reset 및 PSO 설정을 한다.

```c++
if (m_workerThreadContext[threadIndex] != m_curContext)
{
   m_workerThreadContext[threadIndex]++;
   UINT fenceValue = m_workerFenceValues[threadIndex][m_curContext];
   if (m_pWorkerFences[threadIndex][m_curContext]->GetCompletedValue() < fenceValue)
   {
      m_pWorkerFences[threadIndex][m_curContext]->SetEventOnCompletion(fenceValue,
         m_workerFenceEvents[threadIndex][m_curContext]);
      WaitForSingleObject(m_workerFenceEvents[threadIndex][m_curContext], INFINITE);
   }
   HRESULT hr = m_pWorkerCommandAllocators[threadIndex][m_curContext]->Reset();
   assert(SUCCEEDED(hr));
   m_pWorkerCommandLists[threadIndex][m_curContext]->Reset(m_pWorkerCommandAllocators[threadIndex][m_curContext].Get(), nullptr);

   {
      //컨텍스트 재설정
   }
}
```

새 컨텍스트 설정은 새 Offset을 먼저 찾은 스레드가 한다.

```c++
UINT nMeshes =min(m_contextOffsets[m_curContextOffset] - m_lastMeshletIdx, _OBJECT_PER_THREAD);

if (nMeshes==0)
{
//컨텍스트 재설정
}
```

![시간측정_컨텍스트 최적화](시간측정_컨텍스트%20최적화.JPG)
예상은 했지만 메시마다 Execute하기 때문에 기다리는 cpu가 듬성 듬성띄어진 부분은 거의 안보이지만  
전체적으로 노는 시간도 많고 사전 작업이 많아져서 인지 더 느려졌다.

## 최적화 작업 - 2차시도

안그래도 프러스텀 컬링 기다린다고 늦어졌는 데 정렬까지 해버리니 gpu의 그리기 작업까지 5ms나 걸린다.  
 고작 구 하나 그리는 데 말이다.

프러스텀 컬링과 다르게 HI_Z와 HW컬링이 멀티 스레딩 방식이라서 메시렛이 같은 부모인지에 따라 정렬되었다고  
 보장을 못 한다. 그래서 직접 정렬을 했고 이 것때문에 컬링과 다음 패스 사이의 대기 시간이 불가피 해 졌다.

고안한 방식은 다음과 같다.

1. 컬링 후보를 담은 렌더큐와 같은 길이의 패스 여부를 담는 큐를 만든다. 이러면 기록할 떄 동시성 문제를  
   고려 안해도 된다.
2. 그리고 메인스레드에서 불리안 큐에 따라 다음 패스의 렌더큐를 생성한다. 이떄 오프셋 갱신도 같이한다.  
   어차피 패스를 통과한 큐를 다시 합쳐주는 작업을 하고 있었기에 2번 작업 떄문에 더 나빠지지는 않을 것이다.

3. 컬링을 시도하는 스레드에서 일정 개수만 큼 일감이 쌓일 때까지 워커스레드들은 대기하다가
   일감이 생기면 워커스레드가 앞다퉈 일감을 가져간다.

4. 만약 일감이 일 스레드 마다 배정하기로 약속 한 것 보다 작다면 컨텍스트가 변경 된 것이므로. 이때
   해당 일감을 가져온 스레드가 책임지고 컨텍스트를 변경하고 락을 내려 놓음

5. 컨텍스트 단위로 하면 메시하나가 건물처럼 엄청 크다면 문제가된다. 그냥 작성되는 대로 바로
   Excute 해야한다.

이벤트로 컬링후 일감이있다는 걸 알리고,
event로 처리하자니 마지막 일처리후가 문제가 됨. 스레드개수만큼 setSignal할수도 없는 노릇
setEvent하면 하나의 대기 스레드가 풀려나면 nun-signal이됨
오토리셋 이벤트로 처리
--
HW직후에는 결과를 하나씩 돌면서 프로스텀 컬링떄와 같은 방식으로 워커에 통지

한번에 reserve해야한다. 안그러면 진짜 원했던것보다 작다.

프러스텀 컬링과, HI_Z는 INDIRECT를 통할것
프러스텀-Zpaasss, hi_z-hw 짝을 지을것

### 기타 변경 사항

- 11 관련 소스코드 삭제. 추후 11복습겸 따로 버전을 만들도록.
- shared_ptr 뜩칠
- 안쓰던 obj 포맷 관련 함수 삭제
- 커맨드 관리 클래스 따로

- 각 클래스에서 rawPtr와 CBV를 관리하던걸 Heap매니저에서 해쉬로 총괄하고 개체들은 해시에 대한
  색인만 가지고 있는다.

- 행렬은 구조체로 묶어서 선언. transform(tr,rt, sc), wvp(w,v,p), composite(wv, vp, wvp)
- 행렬연산 matrixHelper를 없애고 구조체 포인터를 받아서 바로 접근

- ECS로 관련 리팩터링. 패널을 더추가해서 존재한다면 메시/라이트 컴포넌트에 접근 가능하게 함
- dirty 비트 광원에 추가할. 프로퍼티는 포인터로 바로 설정, 매트릭스만 덜티로 처리
- 광원 클래스에 멤버변수 중복 되 던것 수정. 속성 패널도 분기문을 통하도록 하여 함수를 하나로 줄임
- 어차피 스폿광이 모든 속성을 가지니까 스폿광 속성을 모든 광원에 대해 사용하기로 함
- ImGui에서 값이 바뀌면 true인걸 이제 알게됨.. if문안에 넣고 바로 dirty비트 켜기로함
- DrawFloat()만 써서 따로 배열을 만들고 원래 값 할당하고 다시 원래 값에 덮어씌는 과정을 없앰

- scene에서 그룹을 하나의 엔티티로 취급하던걸 그룹 내의 메시별로 엔티티를 추가하기로함.
- 오브젝트에서 Entity Group으로 이름변경. 엔티티랑 헷갈린다. 게다가 fbx에는 라이팅과 카메라도 들어 있으니까  
  엔티티그룹을 한꺼번에 지우는 버튼은 fbx에서 광원이랑 카메라까지 파싱하는 기능을 추가하면 할 예정

- 파이프라인은 오로지 D3D12클래스를 통해 조율한다. 조율하는 상세한 방식은 힙매니저와 같은 하위 클래스로  
   가려서 코드 변경을 편하게 한다.

- 구조체에 텍스쳐, rtv, dsv, srv, uav 다 넣고 불리안으로 존재 여부만 파악
- 함수로 크기, 포맷, 배열인자 단일인지 , 밉맵 개수, 초기 상태까지 받는다.
- 생성 함수만 좀 까다롭지, 그 후 사용은 배리어와 루트 인자로 뷰설정이 고작이라 쉽다.

- ReadBack힙도 미리 할당
- 엔티티 지우는 방식을 hash를 이용하도록 변경하여 원소 작업을 없앰
- D3D12의 Device만 static으로 변경
- scene에 종속적인 광원, 그림자, 메쉬 배열을 Scene 개체에 포함하고 D3D12에서는 현재 Scene으로  
  부터 가져오게 함
- 창 크기는 Engine클래스 자체에서 가져오도록 함
- BackBuffer, SSAO, PBR(SKYBOX포함), PSO, Sampler, postprocess 모두 별개 클래스로 분리하여  
  D3D12는 오로지 흐름 제어만 하도록 변경
- 커맨드 리스트, 커맨드 할당자, 펜스를 하나의 개체로 관리하고 풀링
- parallax 매핑은 모델별로 설정하도록 함. 고로 메시 클래스로 이동
- 코드 길이 단축을 위한 enum->UINT 캐스팅 매크로 정의
- 번들 제거
- 리소스 배리어 매니저로 플러쉬. 각 자원 클래스에서는 자원마다 함수를 생성하는 게 아닌, pass별로 함수를 만들어서 흐름을  
   명확하게 한다.

- 코드 변경후 WIC함수 사용을 위해 COM을 직접 미리 초기화
- 엔진에서 win32부분 별개 클래스로 분리

- 다운 샘플링 CS의 sampler를 border(1.0f)로 변경.
- brdf쉐이더 div by 0 버그 수정

- 스레드 루프문 탈출 이벤트 추가. CloseHandle()은 핸들만 닫고 스레드는 안 건드림.
- PSO와 Shader 관리 코드를 하나로 합침. 어차피 rs와 pso짝 맞추는 데 실수가 잦았었다.

#### 참조

- [AMD : Understanding GPU context rolls](https://gpuopen.com/learn/understanding-gpu-context-rolls/)
- [DirectXTK : WIC requires COM to be Initialized](https://github.com/microsoft/DirectXTex/wiki/DirectXTex)
- [MSDN : CoInitializeEx ](https://learn.microsoft.com/en-us/windows/win32/learnwin32/initializing-the-com-library)
- [MSDN : Terminating a Thread](https://learn.microsoft.com/en-us/windows/win32/procthread/terminating-a-thread)

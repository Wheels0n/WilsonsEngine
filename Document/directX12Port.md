DX12 추가
=======

Renderer클래스에서 dxgi팩토리를 임시생성 후 QueryInterface()로 dx12지원 여부 확인후 지원시 12로, 안되면 11로(선택 기능은 추후에 넣기로함).  
D3D12클래스를 새로 생성하고 11의 파이프라인을 12로 재구성.  
그림자 맵, 셰이더 같은 사용자 정의 클래스들은 API에따라 11/12클래스를 따로 만들고, 가능하면 함수 오버라이딩으로 처리. 12멀티쓰레딩과 해상도 선택은 파이프라인 구축 후에 하기로함 이참에 폴더명과 프로젝트 명도 Renderer로 바꾸었다.  
특정 API에 종속적이지 않게 추상화 레이어를 도입해야한다. 나중에 VULKAN도 넣을 예정이라 이번처럼 손을 많이타면 시간이 촉박하다.

pso개체 생성과 그에따른 11에서의 코드 흐름 변화
-----------------------------

그래픽카드와 (어댑터) 모니터(디스플레이)가 하나라는 가정하에 작성하였음.  
미리 PSO개체들을 생성해야해서 11의 Init()의 흐름과 12의 그것의 흐름이 달라질 수밖에 없다. 일단 큰 흐름은 msdn의 샘플대로 따라가기로 한다.  
팩토리 생성 - 디바이스 생성 - 커맨드 큐 생성 - 스왑채인 생성 - 디스크립터 힙 생성 - 루트 시그내쳐 생성 - 셰이더 컴파일 - 인풋 데스크 생성 - 사용자 정의 클래스 dss, rss, bs 생성 - pso 생성 - 텍스쳐 및 vb, ib, cb 자원 생성  
일일이 하나의 클래스에서 11인지 12인지 확인하면서 쓰면 쓸데 없는 IF문을 계속 거쳐야하니 당장은 함수오버로딩으로 떄움 pso에서 RTV개수만큼 Format을 지정해주고 나머지는 꼭 Unknown으로 해야함.

루트 시그내처
-------

어차피 shader바이트 코드들을 pso생성시밖에 안 사용하지만, 당장은 11의 흐름대로 작성하기로함.  
The inlined root descriptors should contain descriptors that are accessed most often, though is limited to CBVs, and raw or structured UAV or SRV buffers.  
테이블은 2개이상의 연속되는 위치에 같은 타입의 서술자들이 있으면 가장 첫 번째 서술자 핸들만 세팅하는 데에 쓴다. 고로 같은 종류의 cbv를 연속으로 쉐이더에 선언해도 offset이다르면 말짱도로묵.  
루트시그내쳐이용시 인덱스로 ENUM이용하기로함.

텍스쳐자원 관리의 차이로인한 Model과 Importer클래스 리팩터링
---------------------------------------

서술자힙의 서술자 변수개수는 tex를 모두 가져온 뒤에 파악할 수 있어서 texture를 다 생성한뒤에 몰아서 생성하기로 함.  
텍스쳐들 메모리 관리도 ModelGroup책임이니 넘겨줘야한다.  
빈 SRV는 for문으로 nullSrv를 넘겨줘야한다.

서술자 힙 관리
--------

명렁어 리스트당 한 개 타입에 하나의 서술자힙만 할당해야한다  
각 클래스에 heap등록, 곧바로 Dx12클래스의 메인 힙에다가 다 복붙해서 옮기거나 offset을 통해서 힙에다가 서술자를 생성해야한다. (CopyDescriptorsSimple)단 src힙은 non-shader-visible해야함.  
당장은 D3D12에 서술자 힙을 종류 별로만들고 각 힙별로 curOffset 핸들을 만들어서 다른 클래스에서도 공유하도록 했다. 그러면 거의 모든 사용자정의 클래스의 생성자에다가 D3D12클래스에대한 참조나 포인터를 넘겨야한다. 그렇게 해서 리소스 생성직후에 해당 핸들을 각 클래스의 멤버변수에다가 복사한다.

스왑체인, DWM, flip모델
-----------------

present를 하면 내 app측에서 그린게 dwm의 리디렉션 서피스에 복사(blt) 또는 공유(flip)한뒤 이러한 각 앱으로부터의 서피스들을 재구성해서 스크린에 렌더링한다고 한다.  
단, 풀스크린에서는 그리는 즉시 스크린에 나타난다고 한다. 그릴게 하나 밖에 없으니 어쨰보면 당연하다.  
버퍼카운트는 풀스크린모드에서는 항상 프론트버퍼용 카운트를 1+해서 스왑채인을 생성하라고 한다.

자원 바이트 정렬
---------

모든 자원은 64KB단위로 나뉘어야한다.  
CB읽기는 255바이트 단위. cbvDesc에는 실제 자원크기 그대로

텍스쳐 생성
------

Upload힙으로써 tex2d생성불가. Default로하면 upload힙을 따로 만들어서 이를 통해서 CopyTextureRegion()같은 함수로 업데이트해야한다.  
11에서는 꼭 default가 아니더라도 가능  
D3D12\_TEXTURE\_COPY\_LOCATION이라는 구조체를 통해 그냥 텍스쳐를 그대로 쓰거나 아니면 PlacedFootPrint라는 구조체를 추가로 이용해서 버퍼 자원의 특정 부분을 어떻게 다차원 텍스쳐로 해석할지 지정이 가능하다고 한다. 상당히 구체적으로 변하였다.  
상태전이시에 D3D12\_RESOURCE\_BARRIER\_ALL\_SUBRESOURCES플래그를 넣어야지 하위리소스까지 모두 전이된다.

명령어리스트 관리
---------

무작정 하나로만 했는 데 당장 초기화 할떄만 보아도, 또는 자원 재생성이 필요할 떄, 명령어 리스트를 열고 닫는 데 굉장히 번거로워서 실수할 여지가 크다. 리소스 초기화용 리스트를 한 두개 더 둬야겠다. 명령어 할당자는 한번에 하나의 명령어 리스트와 연결가능해서 따로 하나 더 두었다.

밉맵생성과 계산쉐이더
-----------

계산쉐이더는 그래픽스 파이프라인의 일부가 아니라 계산(Compute)파이프라인의 일부로만 설정이 가능하다.  
계산쉐이더는 범용적이라 정점과 픽셀이 아닌 어떤 데이터에도 작동가능하다.  
문제는 어떻게 input/output을 구성하느냐다.  
계산쉐이더는 디스패치 단위로 실행된다.  
디스패치는 ID3D12GraphicsCommandList::Dispatch를 통해 실행된다.  
디스패치는 3차원이다. 쓰레드 그룹이 디스패치 된다. 그룹내의 쓰레드 개수는 hlsl에서 정의가능.  
SV\_GroupID = 디스패치로 보낸 쓰레드 그룹들 중 하나를 가리킴  
SV\_GroupThreadID =한 쓰레드 그룹내의 쓰레드 중 하나를 가리킴  
SV\_DispatchThreadID = 디스패치 기준으로 해당 쓰레드를 가리킴  
\[numthreads(x, y, z)\] = 한 쓰레드 그룹의 레이아웃. x\*y\*z개의 쓰레드로 구성 됨.  
쓰레드 그룹은 gpu에서 실행시에 wave들로 나뉠수있다. wave내의 쓰레드들은 락스텝단위로 실행되는 데, wave내의 명령어들은 signle streaming multiprocessor에서 병렬적(parallel)하게 실행가능하다. (스트림 프로세서는 입력 집합(스트림)에 대해 함수/커널을 병렬 실행하여 출력 집합을 생성한다)  
그룹내 또는 디스패치 내의 쓰레드 총개수는 query불가능하므로 쉐이더에 인자로 꼭 보내줘야한다.  
그룹공유메모리는 쓰레드그룹내에서 공유가능한 메모리이다. 그 크기는 D3D12에서는 딱히 명시되어있지만 GPU아키텍쳐에 의존적이다. 이 메모리는 동일한 크기의, 32비트단위의 교차되는(interleaved) 주소를 가진 메모리 뱅크들로 나뉜다.  
이 교차주소는 한 wave내에서 쓰레드가 다른 메모리 뱅크를 coflict없이 접근 가능케한다. 뱅크 conflict는 wave내의 서로 다른 쓰레드가 같은 메모리 뱅크에 접근하지만 동일한 주소에 접근하지 않을 떄 발생한다. 뱅크 conflict는 스레드의 액세스 간격이 32bit배수가 아닐 때 더 자주 발생한다. 뱅크 출돌 발생시 메모리 관리자는 모든 읽기 요청을 처리하기위해 공유메모리로 부터 다수의 읽기를 실행한다. coflict가 없다면 모든 읽기는 한번의 읽기만큼의 시간이 소요된다. 계산쉐이더의 출력에는 UAV라는 형식의 뷰를 사용해야한다.  
UAV에서 어떤 밉레벨(MipSlice), 어떤 배열 요소(ArraySlice)를 접근 할지 정해야한다.  
D3D12\_RESOURCE\_STATE\_PIXEL\_SHADER\_RESOURCE는 오로지 ps전용이고 그 외의 shader에서 srv로써 접근하려면 D3D12\_RESOURCE\_STATE\_NON\_PIXEL\_SHADER\_RESOURCE로 설정해야한다.  
UnderSampling은 소스 밉에서 일부 텍스쳐가 사용되지 않을 경우 발생하는 데, 이를 방지하기위해 홀수크기의 텍스쳐에서는 여러 샘플들이 읽혀져야한다. 크기가 반으로 줄때, 픽셀의 일부를 가지는 게 불가능하니, 소수점자리는 버려진다.  
또한 sRGB 텍스쳐는 UAV쓰기로는 지원이 안되기에 UAV 호환가능한 복사본을 만들어 사용해야한다. 텍셀들을 blending이전에 감마 보정하고 blending후에 다시 sRGB차원으로 전환해야한다.  
선형보간으로 샘플링 하고 좌표를 0.5만큼 offset하여 보간을 유도한다.  
miplevel1의 것을 만들 때에 원본의 4사분면만 반영이 된다. ->uv\*2.0f - 1.0f해줘서 해결

### 11과 달라진 점

*   scissorEnable은 래스터 디스크립션에서 사라지고 하드웨어 측에서 테스트 하도록 변경 됨
*   rasterState, depthStencilState, blendState모두 기존에 COM에서, APP측의 개체로 관리하는 걸로 바뀜. pso에 넣어서 관리. 즉, pso남발..
*   rtv, srv, dsv들조차도 그냥 핸들로서 저장한다
*   generateMips()가 없어서 컴퓨트 셰이더로 직접 만들어야한다
*   11의 블렌드 데스크의 마스크 변수가 pso desc의 변수가 되었다
*   CreateCommittedResource()에서 말하는 heap은 서술자 힙이 아니다. 단순히 텍스쳐하나 생성해도 heap속성을 요구한다. "the method will contain the resource heap properties...."리소스힙이다.
*   SetGraphicsRootDescriptorTable로 정해진 루트패러미터에 서술자 핸들을 매치 시켜야한다, 버퍼에 데이터 업로드 자체는 11의 map과 크게 다른 건 없다
*   셰이더가 바이트코드선에서 끝나서 device개체를 쓸 일이 없어졌다. InputLayout도 com이 아니다.
*   COM개체에서 GetDesc()쓰면 디스크립션 가져올수있음
*   정점과 인덱스를 IASetVertexBuffers에서 offeset을 가능했던것과다르게 12에선 DrawCall할떄 지정하는 수밖에 없다
*   바인딩 안된 srv에 nullSrv를 대신해서 넣어줘야함.
*   한 번에 한타입의 heap만 바인드 가능
*   DXGI\_SWAP\_EFFECT\_DISCARD미지원
*   플립모델에서는 고로 2개는 이상은 되야한다.
*   풀스크린 모드에서는 이 변수에 프론트 버퍼용으로 한 개 더 추가해야한다.
*   ClearView에 전달할 초기화 값도 view생성시에 지정해줘야 최적화가 된다.
*   dss설정 함수에서 같이 전달했던 stencilRef값은 OMSetStencilRef()로 직접 처리 하게 됨
*   모델 2개이상인데 matbuffer는 하나라서 flush를 먼저 하거나 모델 클래스에 matcb를 생성해줘야함. 전자로 하면 setup을 다시 다해줘야해서 matbuffer클래스를 모델클래스에 멤버변수로 선언하기로 함.
*   11과 다르게 12는 GenerateMips()같은 고차원 함수는 없어서 직접 ComputeShader를 이용해서 밉맵을 생성해야한다.
*   D3D11CreateshaderFromfile, d3d11compileShader->D3DCompileFromFile

#### reference

[마이크로소프트사의 present, swapMode 강의](https://www.youtube.com/watch?v=E3wTajGZOsA&t=1459s)  
[MSDN의 dx11 텍스쳐 생성함수](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-textures-create)  
[dx12 컴퓨트쉐이더 예제 설명](https://www.3dgep.com/learning-directx-12-4/#Compute_Shaders)  
[LearnOpenGL의 컴퓨트 쉐이더 설명](https://learnopengl.com/Guest-Articles/2022/Compute-Shaders/Introduction)  
[MSDN의 컴퓨트 쉐이더 시스템 시멘틱 설명](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/sv-groupid?redirectedfrom=MSDN)
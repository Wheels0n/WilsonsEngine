# DirectXMath와 SIMD

이전에 SIMD에 대한 지식없이 써 온 DirectXMath에 대해 다시 정리해보려 한다.

## SIMD

DirectXMath API는 DirectX 애플리케이션에 선형 대수 및 그래픽 수학 연산에 대해 SIMD 친화적인  
C++ 타입과 함수를 제공한다.

여기서 말하는 SIMD란 'Single Instruction, Multiple Data'을 뜻 하며 Flynn의 컴퓨터 구조 분류법이  
유래이다. 반대로 전통적인 폰 노이만 구조는 SISD, 'Single Instruction, Single Data'라고 한다.  
SIMD는 하나의 동작으로 여러 데이터를 동시에 처리함으로써 데이터 레벨의 병렬성(parallelism)을  
이용하는 병렬(parallel) 처리 기술이다.

![SIMD 다이어그램](https://www.rastergrid.com/wp-content/uploads/2022/02/SIMD.png)

그림을 보면 하나의 명렁어 스케줄러를 여러 프로세싱 유닛에 재활용하고 있다. 같은 수의 트렌지스터들로
명령어 스케줄러와 프로세싱 유닛을 1대1 대응시키는 고전적인 스칼라 프로세싱 코어에 비해 더 높은 계산
처리량을 달성 할 수 있다.

SIMD는 GPU말고 CPU에도 있다.역사 또한 길어서 MMX, SSE, NEON, 그리고 AVX등의 명령어 집합을 고전적인
스칼라 계산 말고도 CPU에서 제공 한다.

## x86-64 부동소수점 지원 역사

1997년 펜티엄/MMX 출시이후 인텔과 AMD 모두 그래픽과 이미 처리를 위해 여러 세대에 걸친
미디어 명령어들을 통합해 왔다. 이러한 연산들은 원래 SIMD에 초점을 두었다. 수년간 이러한  
확장(Extension)에 대한 진전이 있었다. 일련된 커다란 변화를 거친 끝이 MMX, SSE 그리고
AVX로 이름이 바뀌었다. 각 세대에는 또 각각의 버전들이 있었다. 각 확장은 데이터를 'MM'
(MMX, 64bit), XMM(SSE, 128bit), 그리고 YMM(AVX, 256bit)라는 레지스터 셋에 관리한다.

2000년 펜티엄4 출시이후 SSE2부터 XMM/YMM 레지스터의 하위 32/64비트를 이용하여 스칼라  
 부동소수점 데이터를 작업하는 명령어를 도입했다. 이 스칼라 모드는 다른 프로세서들이  
 부동소수점을 지원하는 방식과 더 유사한 레지스터와 명령어 세트를 제공한다. x86-64코드를  
 처리 할 수있는 모든 프로세서들은 SSE2이상의 것을 지원한다. 고로 x86-64의 부동소수점은
SSE나 AVX에 기반한다.

## DirectXMath

윈도우 8부터 제공 된 3D 수학 라이브러리로 윈도우 SDK의 포함 되어있다. SSE2(Streaming SIMD Extension2)  
명령어 셋을 사용하며 지원 안된다면 정의되지 않는 명령어 예외가 나니 XMVerifyCPUSupport()로 확인하자.

128bit짜리 SIMD 레지스터를 이용하여 32비트 float 이나 int 4개의 스칼라 연산이 아닌 하나의 벡터연산으로 처리 할 수 있다. (double, long, short, byte는 안된다) 64비트 CPU들은 SSE2를 지원하기에 프로젝트 환경에서 별도의 설정없이 헤더만 추가 하면 바로 사용가능하다.

핵심 타입으로 XMVECTOR가 있는데 이는 XMM[0-7] 레지스터와 매핑 된다. 128비트 타입으로 32비트  
 float4개를 한번에 처리 할 수 있게하는 타입이다. SSE2가 지원된다면 다음과 같이 정의 된다.

```c++
typedef __m128 XMVECTOR;
```

ARM은 NEON 명령어 셋을 쓰기에 달라진다. 라이브러리는 성능을 위해 컴파일러 intrinsic을 활용한다.  
물론 intrinsic 없이 구현된 버전도 있긴하다. x64 CPU를 사용하니 SSE/SSE2로 가정하겠다.

16바이트 정렬이 되어야 하는 데 지역/전역 변수라면 컴파일러가 자동으로 처리해준다. 만약 그렇지 않다면
실행 중에 mis-alignment 예외로 크래시가 난다. 그런데 멤버 변수로 선언되면 힙에 선언이 될 텐데
64비트 어플리케이션이라면 힙에 16바이트 정렬을 적용하지만 x86이나 ARM 계통이라면 그렇지 않다. 그래서  
힙을 통해서 XMVECTOR타입을 쓸 때에는 주의를 요구한다. 대신에 멤버변수로는 XMFLOATXX 타입들이 권장된다.

```c++
 struct XMFLOAT2
 {
    float x;
    float y;
    XMFLOAT2() {}
    XMFLOAT2(float _x, float _y) : x(_x), y(_y) {}
    explicit XMFLOAT2(_In_reads_(2) const float *pArray) :
        x(pArray[0]), y(pArray[1]) {}
    XMFLOAT2& operator (const XMFLOAT2& Float2)
    { x  Float2.x; y  Float2.y; return *this; }
 };
 struct XMFLOAT3
 {
    float x;
    float y;
    float z;
    XMFLOAT3() {}
    XMFLOAT3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    explicit XMFLOAT3(_In_reads_(3) const float *pArray) :
        x(pArray[0]), y(pArray[1]), z(pArray[2]) {}
    XMFLOAT3& operator (const XMFLOAT3& Float3)
    { x  Float3.x; y  Float3.y; z  Float3.z; return *this; }
 };
 struct XMFLOAT4
 {
    float x;
    float y;
    float z;
    float w;
    XMFLOAT4() {}
    XMFLOAT4(float _x, float _y, float _z, float _w) :
        x(_x), y(_y), z(_z), w(_w) {}
    explicit XMFLOAT4(_In_reads_(4) const float *pArray) :
        x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}
    XMFLOAT4& operator (const XMFLOAT4& Float4)
    { x  Float4.x; y  Float4.y; z  Float4.z; w  Float4.w; return
    *this; }
 };
```

허나 이것들은 SIMD레지스터가 아닌 메모리에 있어서 SIMD를 이용하려면 XMVECTOR 타입으로 전환
해야한다. XMLoadXXX()/XMStoreXXX() 함수들을 이용하면 상호 전환이 가능하다.

스칼라와 벡터간의 변환은 비효율적이여서 내적처럼 스칼라값이 나오는 연산의 경우 그 값으로만  
이루어진 벡터형태로 반환된다. 그래도 스칼라 형태로 값이 필요하다면 Getter를 이용하거나  
Store()함수를 이용하면 된다.

적절한 호출 규약으로 함수 인자로 안전하게 전달이 된다. 플랫폼 별로 요구사항이 달라진다.
64비트 윈도우의 경우 두 가지 호출 규약이 있다. 하나는 \_fastcall로, 모든 \_m128값을 스택에  
집어 넣는다. 다른하나는 새 비주얼 스튜디어 컴파일러가 지원하는 \_vectorcallfh, \_m128값을
6개까지는 SSE/SSE2레지스터로 전달한다. 공간이 된다면 XMMATRIX도 전달 된다.

FXMVECTOR, GXMVECTOR, HXMVECTOR, 그리고 CXMVECTOR 라는 별칭은 다음과 같은 규약을 지원한다.

- 함수의 첫 세 XMVECTOR 인자까지는 FXMVECTOR 사용
- 네 번째 XMVECTOR 인자로 GXMVECTOR 사용
- 다섯, 여섯번쨰 XMVECTOR 인자들로 HXMVECTOR 사용
- 추가 인자들로 CXMVECTOR 사용

출력인자로 XMVECTOR\* or XMVECTOR&를 사용하고, 입력인자와 관련해서는 이들을 무시 할 것

## 함수 호출에서의 부동소수점

x86-64에서는 XMM 레지스터들이 부동소수점 인자를 전달하고, 반환값으로 받는 데에 쓰인다.
![레지스터 ](http://www.infophysics.net/amd64regs.png)

8개까지 XMM 레지스터를 통해 전달 되며 그 이상은 스택으로 전달된다. 부동소수점 반환값은
XMM0을 통한다. 모든 XMM레지스터들은 caller가 저장하는 방식이다.

만약 인자로 부동소수점 말고 정수나 포인터도 같이 받으면 부동 소수점만 XMM으로 가고, 나머지는  
범용 레지스터로 간다.

### 기타 변경 사항

- 스트링 생성자를 막기 위해 스트링 포인터 타입으로써 인자 전달
- 스텐실 버퍼를 이용한 Outliner 관련 로직 모두 제거
- 프러스텀 컬링 관련연산에 SIMD 이용
- 불필요한 행렬 갱신 및 mempcpy 제거
- 메시 큐에서 메시 벡터로 변경

#### 참조

- [MSDN : DirectXMath](https://learn.microsoft.com/ko-kr/windows/win32/dxmath/directxmath-portal)
- [Introduction to XNA Math](https://www.asawicki.info/news_1366_introduction_to_xna_math)
- [SIMD In the GPU World](https://www.rastergrid.com/blog/gpu-tech/2022/02/simd-in-the-gpu-world/)
- [Flynn's taxonomy](https://en.wikipedia.org/wiki/Flynn%27s_taxonomy)
- [MSDN : \_m128](https://learn.microsoft.com/en-us/cpp/cpp/m128?view=msvc-170)
- [MSDN : Getting started (DirectXMath)](https://learn.microsoft.com/en-us/windows/win32/dxmath/pg-xnamath-getting-started)
- [SimpleMath : XMVECTOR and XMMATRIX](https://github.com/microsoft/DirectXTK/wiki/SimpleMath/564bdb09f64ea2324cefc28ee69232060a765ba7)
- [MSDN : Code Optimization with the DirectXMath Library](https://learn.microsoft.com/en-us/windows/win32/dxmath/pg-xnamath-optimizing)
- [MSDN : Library Internals](https://learn.microsoft.com/en-us/windows/win32/dxmath/pg-xnamath-internals#windows-sse-versus-sse2)

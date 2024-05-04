# 스마트 포인터

여지껏 내가 delete만 잘해주면 굳이 스마트 포인터를 쓸 필요가 있나?  
해서 직접 원시포인터로 하나하나 다 제거해주었다.

assert는 조건을 확인후 거짓이면 std::abort를 호출한다.  
이는 비정상적인 종료로, 어떠한 개체를 파괴하지도 않고 그냥 종료한다.

예외 또한 마찬가지이다. 예외 발생 후 정상적인 흐름대로 코드가 진행 되지  
않을 경우 delete는 불려지지 않는다.

## unique_ptr

예외로 인한 종료의 경우에도 delete를 잊지 않고 해준다.  
지금 내 코드에서는 한 클래스 개체를, 사실상 싱글톤, 여러 클래스에서 공유하는
방식인데 위험 감수하고 get()으로 공유할지 고민이다.

갑자기 가상 소멸자가 호출안된다. editor가 먼저 해제되어야 참조카운트가 0이되는데  
Renderer가 먼저 해제되니 메모리 누수가 난다. 표준에 따르면 선언의  
역순으로 소멸자가 호출이 된다.

```C++
Timer m_timer;
ImGuiManager m_ImGuiManager;
std::unique_ptr<Renderer> m_pRenderer;
std::unique_ptr<Editor> m_pEditor;
```

재설정하려면 .reset() 호출 또는 nullptr를 대입한다.

## ComPtr

c++템플릿 기반의 COM개체를 위한 스마트 포인터로, 참조카운팅 방식이다.  
참조 카운트가 자동으로 관라하며 0이되면 인터페이스를 해제한다.  
마찬가지로 범위밖으로 나가면 알아서 Release()를 호출해준다.

만약 원시 포인터가 필요하다면 .Get()을 호출하면 된다.  
포인터의 포인터로는 .GetAddressOf()를 호출하면된다.  
ComPtr에서 &연산자는 릴리즈 후 \*\*반환이라서 주의해야한다. 이는 크래시로  
이어진다.

만약 D3D인터페이스 포인터 배열을 만든다면 지역변수로 하나 만들면된다.

ComPtr를 값에의한 전달을 하면 참조수치가 변하기에 이를 피하려면 ComPtr를  
cosnt 참조 방식또는 원시 포인터를 전달하면 된다. ComPtr개체를 생성자에  
전달하는 경우, 보통은 참조카운트가 올라가길 원한다. 이경우  
새 ComPtr에다가 해당 원시 포인터를 할당하거나 아예 기존 ComPtr를 대입하면된다.

참조를 해제(release)하고 싶다면 .Reset()함수 호출 또는 nullptr를 대입한다.  
ComPtr가 nullptr인지 확인하려면 그냥 불연산을 하면된다. 오버로딩이 되어있다.

만약 ComPtr에 참조 카운트 변화 없이 원시 포인터를 주고싶다면 .Attach()를 쓴다.  
또 한 ComPtr로부터 다른 ComPtr로 참조 카운트 변화 없이 원시 포인터를 주고싶다면  
Detach후, Attach를 하면된다.

다른 클래스에서 원시포인터로 받아서 쓸거면 원시포인터로 저장해야한다.  
안그러면 COM참조 Underflow가 난다.

### 기타 변경 사항

- D3DMemoryLeakCheck()함수를 Renderer12로 옮김
- Frustum을 Camera클래스에 멤버변수로만 선언되게 변경

#### 참조

-[예외 발생시 해제](https://learn.microsoft.com/en-us/cpp/mfc/exceptions-freeing-objects-in-exceptions?view=msvc-170)  
-[std::unique_ptr](https://en.cppreference.com/w/cpp/memory/unique_ptr)  
-[std::abort](https://cplusplus.com/reference/cstdlib/abort/)  
-[ComPtr](https://github.com/Microsoft/DirectXTK/wiki/ComPtr)  
-[소멸자 호출 순서](https://isocpp.org/wiki/faq/dtors#order-dtors-for-members)

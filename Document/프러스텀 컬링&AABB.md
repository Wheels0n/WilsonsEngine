# 프러스텀 컬링 & 바운딩 볼륨

## 프러스텀 생성

GPU연산을 아끼기 위해 뷰 프러스텀 내에들어 오는 오브젝트들만 그리도록 하는 기술을  
Frustum Culling이라고 한다. 이를 통해 GPU연산을 줄일 수가 있다. 컴퓨터내에 다른  
파트끼리 데이터를 보내는 데에 시간이 오래걸린다는 점을 상기하자.  
유의 할점은 여전히 CPU측에서는 모든 오브젝트를 다 살펴보고 있다. 이를 개선하기 위해서는  
공간 분할이 필요하다.

![프러스텀](https://learnopengl.com/img/guest/2021/Frustum_culling/VisualCameraFrustum.png)

그렇다면 수학적으로 프러스텀을 어떻게 표현할 까? near, far, right, left, top,  
and bottom으로 총 6개의 평면으로 구성한다. 이 평면들 안에 들어오거나 걸치면  
오브젝트는 보인다는 뜻이다. 수학적으로 평은 법선과 원점으로 부터의 거리로 표현  
된다. 평면(plane) 자체는 사각형(quad)과는 다르게 크기 제한이 없다.
![평면 수학적 표현](https://learnopengl.com/img/guest/2021/Frustum_culling/plan.png)

```C++
struct Plane
{
    DirectX::XMVECTOR norm;
    float d;
};
//Frustum.h
Plane m_planes[6];
```

near평면의 경우 법선은 카메라의 전방 벡터이며, far의 경우 그 반대이다. right의 경우는 외적을  
해야 한다. 전방 벡터를 구하려면 right벡터와 up벡터를 내적한다.  
![외적 예시](https://learnopengl.com/img/guest/2021/Frustum_culling/RightNormal.png)

허나 카메라로 부터 far평면까지의 방향 벡터를 구하기위해 Far 사각형의 변의 길이를 알아야한다.  
![전방 벡터 계산](https://learnopengl.com/img/guest/2021/Frustum_culling/hAndVSide.png)

far 사각형의 가로/세로 길이는 프러스텀의 다른 평면들에 의해 제한 된다. 이 경계를 계산하기위해  
삼각함수를 이용해야한다. 세로(vSide)길이를, 직사각형의 높이, 구해야하고 카메라의 FarZ값,  
직사각형의 아랫 변,이 주어진 상황이다. Tan(fovY) = vSide/FarZ 이다. 이로 부터 vSide값은  
구한 셈이다. 화면 종횡비 덕에 hSide = Ar \* vSide로 쉽게 구할 수가 있다.

종횡비는 프로젝션 윈도우(뷰공간내의 2D 씬)의 종횡비를 나타내는 데 보통 백버퍼에다가 결과를 그릴 거라서
백버퍼의 그것으로 설정하는 게 보통이다.

```C++
//Frustum 클래스 생성자
float zFar = *(pCam->GetFarZ());
float zNear = *(pCam->GetNearZ());
float ratio = pCam->GetAspect();
float fovY = *(pCam->GetFovY());

const float halfVSide = zFar * tanf(fovY * 0.5f);
const float halfHSide = halfVSide * ratio;

DirectX::XMVECTOR frontV = pCam->GetDir();
frontV = DirectX::XMVector3Normalize(frontV);

DirectX::XMVECTOR frontMulNear = DirectX::XMVectorScale(frontV, zNear);
DirectX::XMVECTOR frontMulFar = DirectX::XMVectorScale(frontV, zFar);

DirectX::XMVECTOR posV = *(pCam->GetPosition());
DirectX::XMVECTOR right = pCam->GetRight();
DirectX::XMVECTOR up = pCam->GetUp();

DirectX::XMVECTOR dot;


//near, far, right, left, top, bottom
m_planes[0].norm = frontV;
dot = DirectX::XMVector3Dot(DirectX::XMVectorAdd(posV, frontMulNear), m_planes[0].norm);
DirectX::XMStoreFloat(&m_planes[0].d, dot);

frontV = DirectX::XMVectorScale(frontV, -1);
m_planes[1].norm = frontV;
dot = DirectX::XMVector3Dot(DirectX::XMVectorAdd(posV, frontMulFar), m_planes[1].norm);
DirectX::XMStoreFloat(&m_planes[1].d, dot);

DirectX::XMVECTOR cross = DirectX::XMVectorScale(right, halfHSide);
cross = DirectX::XMVectorSubtract(frontMulFar, cross);
cross = DirectX::XMVector3Cross(cross, up);
m_planes[2].norm = DirectX::XMVector3Normalize(cross);
dot = DirectX::XMVector3Dot(posV, m_planes[2].norm);
DirectX::XMStoreFloat(&m_planes[2].d, dot);


cross = DirectX::XMVectorScale(right, halfHSide);
cross = DirectX::XMVectorAdd(frontMulFar, cross);
cross = DirectX::XMVector3Cross(up, cross);
m_planes[3].norm = DirectX::XMVector3Normalize(cross);
dot = DirectX::XMVector3Dot(posV, m_planes[3].norm);
DirectX::XMStoreFloat(&m_planes[3].d, dot);

cross = DirectX::XMVectorScale(up, halfVSide);
cross = DirectX::XMVectorSubtract(frontMulFar, cross);
cross = DirectX::XMVector3Cross(right, cross);
m_planes[4].norm = DirectX::XMVector3Normalize(cross);
dot = DirectX::XMVector3Dot(posV, m_planes[4].norm);
DirectX::XMStoreFloat(&m_planes[4].d, dot);

cross = DirectX::XMVectorScale(up, halfVSide);
cross = DirectX::XMVectorAdd(frontMulFar, cross);
cross = DirectX::XMVector3Cross(cross, right);
m_planes[5].norm = DirectX::XMVector3Normalize(cross);
dot = DirectX::XMVector3Dot(posV, m_planes[5].norm);
DirectX::XMStoreFloat(&m_planes[5].d, dot);
```

## Sphere

![구 방식 적용](https://learnopengl.com/img/guest/2021/Frustum_culling/boundingSphere.png)  
제일 간단한 방식이며, 중앙위치 값과 반지름 값으로 표현된다. 구는 회전을 수반하는 어떤 메시로 포함  
하기에 적합하다. 꼭 크기와, 위치에 따른 조정이 필요하다.

## AABB

![충돌 판정 알고리듬](https://learnopengl.com/img/guest/2021/Frustum_culling/boundingVolumeQuality.png)

잠시 메시의 충돌을 판정하는 알고리듬을 상상해보자. 만약 삼각형 단위로 판정을 하되 메시가 하나라면  
매우 빠르게 느껴질 것이다. 허나 수천개의 삼각형으로 이루어진 다수의 메시를 이런식으로 판정하다간  
매우 느려질것이다. 또 다른 방식으론 오브젝트를 구, 사각형, 캡슐과 같은 간단한 기하학적 오브젝트로  
덮어 씌우는 것이다. 이를 보고 바운딩 볼륨라하고 원래 메시 대신에 쓰여서 판정을 간단하게 해준다.  
본 프로젝트에서는 AABB를 쓰기로 하였다. ~~본인한테 가장 직관적이라~~

AABB는 축 정렬 바운딩 박스(Axis algined bounding box)의 축약어이다. 이말은 즉슨 이 볼륨은 월드와  
같은 방향을을 띄고 있다는 것이다. 다양한 방법으로 생성이 가능하지만 중심과 각 축의 길이의 절반을  
가지고 만들기로 한다. 이 길이들은 lx, ly, lz로 부르기로한다.  
![AABB 도면](https://learnopengl.com/img/guest/2021/Frustum_culling/AABBRepresentation.png)

```C++
//AABB 생성자
AABB::AABB(DirectX::XMFLOAT3 minAABB, DirectX::XMFLOAT3 maxAABB)
{
    DirectX::XMFLOAT4 center((minAABB.x + maxAABB.x) * 0.5f, (minAABB.y + maxAABB.y) * 0.5f, (minAABB.z + maxAABB.z) * 0.5f, 1.0f);
    m_center = DirectX::XMLoadFloat4(&center);

    DirectX::XMFLOAT3 extents((maxAABB.x-center.x), (maxAABB.y-center.y) , (maxAABB.z-center.z));
    m_extents = DirectX::XMLoadFloat3(&extents);
    UpdateVertices();
}
AABB::AABB(DirectX::XMFLOAT4 center, float il, float ij, float ik)
{

    m_center = DirectX::XMLoadFloat4(&center);
    DirectX::XMFLOAT3 extents(il, ij, ik);
    m_extents = DirectX::XMLoadFloat3(&extents);
    UpdateVertices();

}
```

문제가 있는 데 메쉬가 회전하면 AABB도 조정 되어야 한다. 아래의 그림을 보자.  
![AABB-회전](https://learnopengl.com/img/guest/2021/Frustum_culling/AABBProblem.png)

![AABB-회전문제 해결](https://learnopengl.com/img/guest/2021/Frustum_culling/AABB%20orientation.png)  
(기준계가 단위 벡터로 아닌 거리 축으로 이루어져 있다고 가정). 세 번쨰 그림에서처럼 새로운 거리 축은  
월드 공간 축과 메시의 크기 조절된 기준계의 내적의 합으로 조정한다. 사각형이 따라 회전하지는 않고  
대신 크기가 변한다. 아래는 왜 내적의 합인지 이해를 돕기위한 그림이다.

![AABB-회전 덧셈 이유](https://johanhelsing.studio/processed_images/64b5b9f125a64c8400.png)

```C++
DirectX::XMVECTOR globalCenterV = DirectX::XMVector4Transform(m_center, transform);
DirectX::XMFLOAT4 globalCenter;
DirectX::XMStoreFloat4(&globalCenter, globalCenterV);

DirectX::XMFLOAT3 extents;
DirectX::XMStoreFloat3(&extents, m_extents);


DirectX::XMVECTOR right = DirectX::XMVectorScale(transform.r[0], extents.x);
DirectX::XMVECTOR up = DirectX::XMVectorScale(transform.r[1], extents.y);
DirectX::XMVECTOR forward = DirectX::XMVectorScale(transform.r[2], -extents.z);


DirectX::XMVECTOR x = DirectX::XMVectorSet(1.0, 0.0f, 0.0f, 0.f);
DirectX::XMVECTOR y = DirectX::XMVectorSet(0.0, 1.0f, 0.0f, 0.f);
DirectX::XMVECTOR z = DirectX::XMVectorSet(0.0, 0.0f, 1.0f, 0.f);

float dotRight;
float dotUp;
float dotForward;

DirectX::XMStoreFloat(&dotRight,DirectX::XMVector3Dot(x, right));
DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(x, up));
DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(x, forward));

const float newli = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);

DirectX::XMStoreFloat(&dotRight, DirectX::XMVector3Dot(y, right));
DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(y, up));
DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(y, forward));

const float newlj = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);

DirectX::XMStoreFloat(&dotRight, DirectX::XMVector3Dot(z, right));
DirectX::XMStoreFloat(&dotUp, DirectX::XMVector3Dot(z, up));
DirectX::XMStoreFloat(&dotForward, DirectX::XMVector3Dot(z, forward));

const float newlk = std::abs(dotRight) + std::abs(dotUp) + std::abs(dotForward);


const AABB globalAABB(globalCenter, newli, newlj, newlk);
```

## 프러스텀 컬링

평면 위에 걸치는 지 안에 포함되는 지 여부는

```C++
bool AABB::IsOnOrForwardPlane(const Plane& plane) const
{
    DirectX::XMFLOAT3 extents;
    DirectX::XMStoreFloat3(&extents, m_extents);
    DirectX::XMFLOAT3 norm;
    DirectX::XMStoreFloat3(&norm, plane.norm);

    //AABB 중심과 표면의 거리와 AABB의 reach 비교
    const float r = extents.x * std::abs(norm.x) + extents.y * std::abs(norm.y) +
        extents.z * std::abs(norm.z);

    DirectX::XMVECTOR dot = DirectX::XMVector3Dot(plane.norm, m_center);
    float signedDistanceToPlane = dot.m128_f32[0] - plane.d;
    return -r <=signedDistanceToPlane;
}
```

![점과 평면의 거리](https://learnopengl.com/img/guest/2021/Frustum_culling/SignedDistanceDraw.png)  
어느 점이 평면의 앞에 있다면 거리는 양수가 되고 그 반대의 경우 음수가 된다. 내적을 통해  
벡터에서 벡터로의 투영을 얻을 수가 있다. 결과는 스칼라값으로, 거리이다. 만약 두 벡터가  
반대 방향이라면 값도 음수이다. 이 덕에 평면의 법선을 방향으로 수평 방향의 크기를 구할  
수가 있다. 이 값에서 평면과 원점 사이의 최단거리를 뺴주면 된다.

#### 참조

[LearnOpenGl - 프러스텀 컬링](https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling)
[충돌 판정관련 네이버블로그 포스트](https://m.blog.naver.com/jerrypoiu/221172549241)  
[AABB 프러스텀 판정 알고리듬 설명](https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html)

#

## ExecuteBundle

API는 번들이라는 것을 이용하여 GPU 하드웨어의 기능을 극대화한다.  
번들은 앱측에서 소수의 api명령어들을 그룹지어서 나중에 쓰기위함이다.  
번들 생성시에 드라이버는 나중에 실행할때 최대한 가볍게 하기위해  
최적화를 실행한다. 번들은 여러번 사용하기위해 디자인되었다는 점이  
일반 명령어 리스트와는 다르다. 물론 명령어리스트도 여러번 실행될수는  
있다. 아래는 번들에 포함 할 수 없는 함수이다.

- Any Clear method
- Any Copy method
- DiscardResource
- ExecuteBundle
- ResourceBarrier
- ResolveSubresource
- SetPredication
- BeginQuery
- EndQuery
- SOSetTargets
- OMSetRenderTargets
- RSSetViewports
- RSSetScissorRects

번들 기록 방식은 명령어리스트 방식과 같다. 한번 기록하고 close().
사용시에는 명령어리스트->ExecuteBundle()함수를 호출해줘야한다.

카메라 이동 떄문에 계속 메시큐가 갱신되서 함부로 Bundle은 못쓴다.
굳이 쓰면 hiz컬링, SkyBox, SSAO, 라이팅, 후처리뿐.  
Compute나 Copy로 설정된 명령어리스트는 번들을 못쓴다.

병목의 원인이 무수한 Drawcall + 노는 GPU탓이라 CPU쪽을
최적화한다고 더 좋을 건 없다. 성능도 차이가 없다.

## 메시 클러스터

모든 메시를 나만의 primitive로 잘게 썰어서(?) 컬링의 정확도를  
높이는 방법. 어쌔신 크리드 유니티에서는 한 메시를 64개 짜리  
삼각형 스트립으로 구성하였다. 스트립방식으로하면 그 스트립의  
첫번쨰 삼각형을 뺴고는 직전의 정점 2개를 쓰기 떄문에 정점이  
리스트 방식을 썼을 떄보다 2/3 감소한다. 이렇게 작은 메시 단위로  
하면 컬링도 쉬울 것이다. 지금 나같은 경우에는 커다란 메시가 너무  
많아서 HW오클루전 컬링을 해도 3~400개는 기본으로 살아 남는다.

strip이라서 모든 면을 살려야한다. RS에서 컬링 논으로 설정  
strip은 정점이 단하나만 있으면 된다. map으로 확인해보니  
각 정점이 5~7개씩 불려진다. 16개짜리도 있다. 3차원이라는 걸
감안하면 놀랄일도아님.

항상 차수가 낮은 삼각형을 우선으로, 우선순위큐 이용.  
차수가 같으면 그 삼각형의 이웃을 비교해서 판정.  
그래도 안되면 아무것이나.  
시작은 차수가낮은 삼각형 아무거나 시작. 모든 삼각형을  
찾으면 끝나니 선형시간내로 끝난다.

```C
//triQ:우선순위큐, 차수가 낮은 순으로 삼각형을 저장
//visited
GenStrip();
{
    while(!triQ.empty())
    {
        if(start)
        {
            tri = triQ.front();
            while(visited[tri])
            {
                 triQ.pop();
                tri = triQ.front();
            }
            start=false;
        }

        next = null;
        int ref = INT_MAX;
        int curE;
        for(e : 3)
        {
            for(tri : 참조중인 삼각형)
            {
                if(ref>tri.ref)
                {
                    ref=tri.ref;
                    next=tri;
                    curE=e;
                }
            }
        }
        if(next==null)
        {
            start=true;
        }
        else
        {
            result.push_back(tri.vetex);
            tri = next;
        }
    }

}
```

새로운 strip이 막히면, 다시 삼각형을 그려야한다.  
이떄 degenerate Triangle, 평평한 삼각형, 을  
임의로 끼워 넣어서 계속이어나간다. 방법은 직전  
삼각형의 마지막 정점과 새 삼각형의 첫 정점을 1회  
반복한다.

64~66개단위로 나누어서 렌더링에는 성공하였으나, 같은  
데이터라도 드로우콜이 많으면 왜느려지는지 이제야 체감하고있다.  
1번에서 8천번으로 극단적이긴하지만 170fps 10fps로 줄었다.  
cluster용 클래스를 만들면 subMesh가 매우 복잡해지는 데다가 풀링하는 데 오래걸림  
배열로 subMesh에 저장. subMesh확인 끝날떄마다 가림여부 갱신.  
너무 많은 Cluster가 있어서 한계가 있다.

Hi-Z를 다시 구현해보려고 3일간 노력했으나 안되었다.  
혹시나 싶어서 AABB를 다시 구현하여 실행해보았다.  
![AABB와이어프레임](처참한클러스터링%20결과.JPG)  
위 사진은 256개 단위로 자른 것이지만 정점 8개 단위로  
자른들 다른 바운딩박스와 겹처서  
컬링이 제대로 안되는게 이상하지 않다.

### 기타 변경 사항

- Importer12::LoadSubFbx()함수에서 탄젠트 생성 시 코드 중복제거
- 정점을 색인 수만큼 중복해서 담았던것 수정

#### 참조

-[MSDN-번들 생성 및 기록](https://learn.microsoft.com/en-us/windows/win32/direct3d12/recording-command-lists-and-bundles)  
-[SIGGRAPH 2015:GPU-Driven Rendering Pipelines](https://advances.realtimerendering.com/s2015/aaltonenhaar_siggraph2015_combined_final_footer_220dpi.pdf)  
-[SGI알고리즘 논문](https://www.cs.umd.edu/gvil/papers/av_ts.pdf)  
-[degenerate triagle](https://www.learnopengles.com/tag/triangle-strips/)

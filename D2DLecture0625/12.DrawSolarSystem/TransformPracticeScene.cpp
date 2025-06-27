#include "pch.h"
#include "InputManager.h"
#include "D2DTransform.h"
#include "SolarSystemRenderer.h"
#include "TransformPracticeScene.h"

using TestRenderer = myspace::D2DRenderer;
using Vec2 = MYHelper::Vector2F;


class CelestialObj // 천체 클래스
{
    CelestialObj() = delete; //기본생성자 삭제
    CelestialObj(const CelestialObj&) = delete; // 복사생성자 삭제
    void operator=(const CelestialObj&) = delete; // 복사대입연산자 삭제

public:
    CelestialObj(ComPtr<ID2D1Bitmap1>& bitmapPtr, float rotationSpeed, bool isSelfRotation)
    {
        m_bitmapPtr = bitmapPtr; // 이미지 넣어주기.
        m_rotationSpeed = rotationSpeed; // 자전속도. 얼마만큼 회전할 건지.
        m_isSelfRotation = isSelfRotation; // 자전 여부.
        m_renderTM = MYTM::MakeRenderMatrix(true); // 렌더링 변환 행렬 만들기. // 이미 기본값이 들어있음. 유니티만 true로 바꿔줌.
        D2D1_SIZE_F size = { m_rect.right - m_rect.left, m_rect.bottom - m_rect.top }; // 사각형 크기 구하기
        m_transform.SetPivotPreset(D2DTM::PivotPreset::Center, size); // 트렌스폼 피봇 설정. 이건 사각형 중심에.

    }

    ~CelestialObj() = default;

    void Update(float deltaTime)
    {
        if (m_isSelfRotation)
        {
            m_transform.Rotate(deltaTime * m_rotationSpeed); // 자전. 
            //std::cout << "돈다" << std::endl;

        }
    }

    void Draw(TestRenderer& testRender, D2D1::Matrix3x2F viewTM)
    {
        static  D2D1_RECT_F s_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f); // 기준 사각형. // 한 번만 생성해 계속 재사용.

        D2D1::Matrix3x2F worldTM = m_transform.GetWorldMatrix();  //개별 오브젝트의 월드 트랜스폼
        D2D1::Matrix3x2F finalTM = m_renderTM * worldTM * viewTM; // 로컬좌표 -> 월드좌표 -> 화면좌표 // 화면에 최종 찍히는 위치로 바꿔줌
        // m_renderTM : 렌더링에 필요한 추가 행렬. ex) 오브젝트를 2배 확대.
        // worldTM : 오브젝트 자체의 위치, 회전. // viewTM : 카메라(뷰)의 변환.

        testRender.SetTransform(finalTM); // 이후 그릴 도형은 finalTM을 적용해서 그려라.

        D2D1_RECT_F dest = D2D1::RectF(s_rect.left, s_rect.top, s_rect.right, s_rect.bottom); // 그릴 사각형. s_rect를 사용해도 똑같지만
        // 혹시 있을 변형을 대비. 기준사각형은 바뀌면 안됨.

        testRender.DrawBitmap(m_bitmapPtr.Get(), dest); // 사각형 위치에 비트맵 이미지 그려라.
    }

    void SetPosition(const Vec2& pos)
    {
        m_transform.SetPosition(pos);
    }

    void SetScale(const Vec2& scale)
    {
        m_transform.SetScale(scale);
    }

    D2DTM::Transform* GetTransform()
    {
        return &m_transform;
    }

    void SetParent(CelestialObj* parent) // 부모 관계 설정.
    {
        assert(parent != nullptr);

        if (nullptr != m_transform.GetParent())
        {
            m_transform.DetachFromParent(); // 이미 부모가 있다면 부모 관계를 해제. 
        }

        m_transform.SetParent(parent->GetTransform());
    }

    void DetachFromParent() // 부모 관계 해제.
    {
        m_transform.DetachFromParent();
    }

private:
    D2DTM::Transform m_transform; // 트랜스폼

    MAT3X2F m_renderTM; // 렌더링 변환 행렬

    D2D1_RECT_F m_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f); // 사각형

    bool m_isSelfRotation; // 태양과 행성은 true, 달은 false로 할 듯. 위성을 가진 행성 하나 더 추가. 그 위성은 true로.

    float m_rotationSpeed; // 자전 속도. 각도긴 하지만 작을수록 느리게 도니까 걍 속도라고 하겠음.

    ComPtr<ID2D1Bitmap1> m_bitmapPtr; // 비트맵 이미지. 이건 자동으로 delete해줘서 소멸자가 할 일이 없음..
};

TransformPracticeScene::~TransformPracticeScene()
{
    for (auto& obj : m_CelestialObjs)
    {
        delete obj;
    }
}

void TransformPracticeScene::SetUp(HWND hWnd)
{
    constexpr int defaultGameObjectCount = 30;
    m_CelestialObjs.reserve(defaultGameObjectCount); // 이 벡터에 이만큼 넣을거야 미리 알려주는 것.

    m_hWnd = hWnd;

    SetWindowText(m_hWnd,
        L"가상의 태양계를 만들어 주세요. 물리 법칙은 무시 합니다. ^^;;");

    std::cout << "태양은 자전을 해야 합니다." << std::endl; // 넵
    std::cout << "행성들은 자전을 하며 동시에 태양의 자전에 영향을 받아 공전하는 것처럼 보입니다." << std::endl; // 지구는 그렇게 했어요.
    std::cout << "달은 자전을 하면서 동시에 지구의 자전에 영향을 받아 공전하는 것처럼 보입니다." << std::endl; // 달이랑 가니메데요.
    std::cout << "회전 속도는 자유롭게 설정하세요." << std::endl; // 넵

    // 비트맵 가져오기
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/cat.png", *m_BitmapPtr.GetAddressOf()); // 공전을 위한 더미
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/sun.png", *m_bitmapPtr_Sun.GetAddressOf()); // 태양
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/mercury.png", *m_bitmapPtr_Mercury.GetAddressOf()); // 수성
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/venus.png", *m_bitmapPtr_Venus.GetAddressOf()); // 금성
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/earth.png", *m_bitmapPtr_Earth.GetAddressOf()); // 지구
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/moon.png", *m_bitmapPtr_Moon.GetAddressOf()); // 달
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/mars.png", *m_bitmapPtr_Mars.GetAddressOf()); // 화성
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/jupiter.png", *m_bitmapPtr_Jupiter.GetAddressOf()); // 목성
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/ganymede.png", *m_bitmapPtr_Ganymede.GetAddressOf()); // 가니메데
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/saturnlol.png", *m_bitmapPtr_Saturn.GetAddressOf()); // 토성
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/uranus.png", *m_bitmapPtr_Uranus.GetAddressOf()); // 천왕성
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/neptune.png", *m_bitmapPtr_Neptune.GetAddressOf()); // 해왕성

    // 천체 배치
    AddCelestialObjs();

    RECT rc;
    if (::GetClientRect(hWnd, &rc))
    {
        float w = static_cast<float>(rc.right - rc.left);
        float h = static_cast<float>(rc.bottom - rc.top);

        m_UnityCamera.SetScreenSize(w, h);
    }
}

void TransformPracticeScene::Tick(float deltaTime)
{
    ProcessKeyboardEvents(); // 카메라 이동.
    
    for (auto& obj : m_CelestialObjs)
    {
        obj->Update(deltaTime); // 천체들 업데이트
    }


    // 카메라 업데이트
    MAT3X2F cameraTM = m_UnityCamera.GetViewMatrix();
    MAT3X2F renderTM = MYTM::MakeRenderMatrix(true); // 카메라 위치 렌더링 매트릭스
    MAT3X2F finalTM = renderTM * cameraTM;

    // 렌더링
    static myspace::D2DRenderer& globalRenderer = SolarSystemRenderer::Instance();

    globalRenderer.RenderBegin();

    globalRenderer.SetTransform(finalTM);

    for (auto& obj : m_CelestialObjs)
    {
        obj->Draw(globalRenderer, cameraTM);
    }

    globalRenderer.RenderEnd();
}

void TransformPracticeScene::OnResize(int width, int height)
{
    // 윈도우 크기 변경 시 카메라의 화면 크기를 업데이트
    m_UnityCamera.SetScreenSize(width, height);
}

void TransformPracticeScene::ProcessKeyboardEvents()
{
    // 카메라 이동
    static const std::vector<std::pair<int, Vec2>> kCameraMoves = {
    { VK_RIGHT, {  1.f,  0.f } },
    { VK_LEFT,  { -1.f,  0.f } },
    { VK_UP,    {  0.f,  1.f } },
    { VK_DOWN,  {  0.f, -1.f } },
    };

    // C++17부터는 structured binding을 사용하여 더 간결하게 표현할 수 있습니다.
    for (auto& [vk, dir] : kCameraMoves)
    {
        if (InputManager::Instance().GetKeyDown(vk))
        {
            m_UnityCamera.Move(dir.x, dir.y);
        }
    }
}

void TransformPracticeScene::AddCelestialObjs() // 천체 추가 // 일단 하드코딩.. // 고증을 지키고 싶었지만 포기..
{
    //=============================공전용 더미==================================================
    CelestialObj* ofMercury = new CelestialObj(m_BitmapPtr, 41.f, true); // 수성용 
    ofMercury->SetScale(Vec2(0.1f, 0.1f));
    ofMercury->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofMercury);

    CelestialObj* ofVenus = new CelestialObj(m_BitmapPtr, 16.f, true); // 금성용
    ofVenus->SetScale(Vec2(0.1f, 0.1f));
    ofVenus->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofVenus);

    CelestialObj* ofMars = new CelestialObj(m_BitmapPtr, 5.f, true); // 화성용
    ofMars->SetScale(Vec2(0.1f, 0.1f));
    ofMars->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofMars);

    CelestialObj* ofJupiter = new CelestialObj(m_BitmapPtr, 3.f, true); // 목성용
    ofJupiter->SetScale(Vec2(0.1f, 0.1f));
    ofJupiter->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofJupiter);

    CelestialObj* ofSaturn = new CelestialObj(m_BitmapPtr, 2.5f, true); // 토성용
    ofSaturn->SetScale(Vec2(0.1f, 0.1f));
    ofSaturn->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofSaturn);

    CelestialObj* ofUranus = new CelestialObj(m_BitmapPtr, 1.5f, true); // 천왕성용
    ofUranus->SetScale(Vec2(0.1f, 0.1f));
    ofUranus->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofUranus);

    CelestialObj* ofNeptune = new CelestialObj(m_BitmapPtr, 1.f, true); // 해왕성용
    ofNeptune->SetScale(Vec2(0.1f, 0.1f));
    ofNeptune->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofNeptune);
    //=============================공전용 더미 끝==================================================

    CelestialObj* Sun = new CelestialObj(m_bitmapPtr_Sun, 10.0f, true); // 태양
    Sun->SetScale(Vec2(1.5f, 1.5f));
    Sun->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(Sun);

    CelestialObj* Mercury = new CelestialObj(m_bitmapPtr_Mercury, 15.0f, true); // 수성
    Mercury->SetScale(Vec2(0.2f, 0.2f));
    Mercury->SetPosition(Vec2(35.f, 50.f)); // 부모 설정 하기 전에 스케일, 위치 설정 먼저.
    Mercury->SetParent(ofMercury);
    m_CelestialObjs.push_back(Mercury);

    CelestialObj* Venus = new CelestialObj(m_bitmapPtr_Venus, -10.0f, true); // 금성
    Venus->SetScale(Vec2(0.35f, 0.35f));
    Venus->SetPosition(Vec2(80.f, 50.f));
    Venus->SetParent(ofVenus);
    m_CelestialObjs.push_back(Venus);

    CelestialObj* Earth = new CelestialObj(m_bitmapPtr_Earth, 25.0f, true); // 지구
    Earth->SetScale(Vec2(0.5f, 0.5f));
    Earth->SetPosition(Vec2(150.f, 50.f));
    Earth->SetParent(Sun);
    m_CelestialObjs.push_back(Earth);

    CelestialObj* Moon = new CelestialObj(m_bitmapPtr_Moon, 0.0f, false); // 달 // 공전주기와 자전주기가 같아서 항상 같은 면이 지구를 보고있음. 그래서 false함.
    Moon->SetScale(Vec2(0.15f, 0.15f));
    Moon->SetPosition(Vec2(180.f, 50.f));
    Moon->SetParent(Earth);
    m_CelestialObjs.push_back(Moon);

    CelestialObj* Mars = new CelestialObj(m_bitmapPtr_Mars, 20.0f, true); // 화성
    Mars->SetScale(Vec2(0.5f, 0.5f));
    Mars->SetPosition(Vec2(230.f, 50.f));
    Mars->SetParent(ofMars);
    m_CelestialObjs.push_back(Mars);

    CelestialObj* Jupiter = new CelestialObj(m_bitmapPtr_Jupiter, 60.0f, true); // 목성
    Jupiter->SetScale(Vec2(1.f, 1.f));
    Jupiter->SetPosition(Vec2(360.f, 50.f));
    Jupiter->SetParent(ofJupiter);
    m_CelestialObjs.push_back(Jupiter);

    CelestialObj* Ganymede = new CelestialObj(m_bitmapPtr_Ganymede, 10.0f, true); // 가니메데 // 이 친구도 자전이랑 공전주기가 같은데.. 그래도 돌려보고 싶어서 false안함.
    Ganymede->SetScale(Vec2(0.25f, 0.25f));
    Ganymede->SetPosition(Vec2(420.f, 50.f));
    Ganymede->SetParent(Jupiter);
    m_CelestialObjs.push_back(Ganymede);

    CelestialObj* Saturn = new CelestialObj(m_bitmapPtr_Saturn, 55.0f, true); // 토성
    Saturn->SetScale(Vec2(0.8f, 0.8f));
    Saturn->SetPosition(Vec2(480.f, 50.f));
    Saturn->SetParent(ofSaturn);
    m_CelestialObjs.push_back(Saturn);

    CelestialObj* Uranus = new CelestialObj(m_bitmapPtr_Uranus, -30.0f, true); // 천왕성
    Uranus->SetScale(Vec2(0.6f, 0.6f));
    Uranus->SetPosition(Vec2(580.f, 50.f));
    Uranus->SetParent(ofUranus);
    m_CelestialObjs.push_back(Uranus);

    CelestialObj* Neptune = new CelestialObj(m_bitmapPtr_Neptune, 30.0f, true); // 해왕성
    Neptune->SetScale(Vec2(0.6f, 0.6f));
    Neptune->SetPosition(Vec2(670.f, 50.f));
    Neptune->SetParent(ofNeptune);
    m_CelestialObjs.push_back(Neptune);


}

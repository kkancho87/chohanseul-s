#include "pch.h"
#include "InputManager.h"
#include "D2DTransform.h"
#include "SolarSystemRenderer.h"
#include "TransformPracticeScene.h"

using TestRenderer = myspace::D2DRenderer;
using Vec2 = MYHelper::Vector2F;


class CelestialObj // õü Ŭ����
{
    CelestialObj() = delete; //�⺻������ ����
    CelestialObj(const CelestialObj&) = delete; // ��������� ����
    void operator=(const CelestialObj&) = delete; // ������Կ����� ����

public:
    CelestialObj(ComPtr<ID2D1Bitmap1>& bitmapPtr, float rotationSpeed, bool isSelfRotation)
    {
        m_bitmapPtr = bitmapPtr; // �̹��� �־��ֱ�.
        m_rotationSpeed = rotationSpeed; // �����ӵ�. �󸶸�ŭ ȸ���� ����.
        m_isSelfRotation = isSelfRotation; // ���� ����.
        m_renderTM = MYTM::MakeRenderMatrix(true); // ������ ��ȯ ��� �����. // �̹� �⺻���� �������. ����Ƽ�� true�� �ٲ���.
        D2D1_SIZE_F size = { m_rect.right - m_rect.left, m_rect.bottom - m_rect.top }; // �簢�� ũ�� ���ϱ�
        m_transform.SetPivotPreset(D2DTM::PivotPreset::Center, size); // Ʈ������ �Ǻ� ����. �̰� �簢�� �߽ɿ�.

    }

    ~CelestialObj() = default;

    void Update(float deltaTime)
    {
        if (m_isSelfRotation)
        {
            m_transform.Rotate(deltaTime * m_rotationSpeed); // ����. 
            //std::cout << "����" << std::endl;

        }
    }

    void Draw(TestRenderer& testRender, D2D1::Matrix3x2F viewTM)
    {
        static  D2D1_RECT_F s_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f); // ���� �簢��. // �� ���� ������ ��� ����.

        D2D1::Matrix3x2F worldTM = m_transform.GetWorldMatrix();  //���� ������Ʈ�� ���� Ʈ������
        D2D1::Matrix3x2F finalTM = m_renderTM * worldTM * viewTM; // ������ǥ -> ������ǥ -> ȭ����ǥ // ȭ�鿡 ���� ������ ��ġ�� �ٲ���
        // m_renderTM : �������� �ʿ��� �߰� ���. ex) ������Ʈ�� 2�� Ȯ��.
        // worldTM : ������Ʈ ��ü�� ��ġ, ȸ��. // viewTM : ī�޶�(��)�� ��ȯ.

        testRender.SetTransform(finalTM); // ���� �׸� ������ finalTM�� �����ؼ� �׷���.

        D2D1_RECT_F dest = D2D1::RectF(s_rect.left, s_rect.top, s_rect.right, s_rect.bottom); // �׸� �簢��. s_rect�� ����ص� �Ȱ�����
        // Ȥ�� ���� ������ ���. ���ػ簢���� �ٲ�� �ȵ�.

        testRender.DrawBitmap(m_bitmapPtr.Get(), dest); // �簢�� ��ġ�� ��Ʈ�� �̹��� �׷���.
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

    void SetParent(CelestialObj* parent) // �θ� ���� ����.
    {
        assert(parent != nullptr);

        if (nullptr != m_transform.GetParent())
        {
            m_transform.DetachFromParent(); // �̹� �θ� �ִٸ� �θ� ���踦 ����. 
        }

        m_transform.SetParent(parent->GetTransform());
    }

    void DetachFromParent() // �θ� ���� ����.
    {
        m_transform.DetachFromParent();
    }

private:
    D2DTM::Transform m_transform; // Ʈ������

    MAT3X2F m_renderTM; // ������ ��ȯ ���

    D2D1_RECT_F m_rect = D2D1::RectF(0.f, 0.f, 100.f, 100.f); // �簢��

    bool m_isSelfRotation; // �¾�� �༺�� true, ���� false�� �� ��. ������ ���� �༺ �ϳ� �� �߰�. �� ������ true��.

    float m_rotationSpeed; // ���� �ӵ�. ������ ������ �������� ������ ���ϱ� �� �ӵ���� �ϰ���.

    ComPtr<ID2D1Bitmap1> m_bitmapPtr; // ��Ʈ�� �̹���. �̰� �ڵ����� delete���༭ �Ҹ��ڰ� �� ���� ����..
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
    m_CelestialObjs.reserve(defaultGameObjectCount); // �� ���Ϳ� �̸�ŭ �����ž� �̸� �˷��ִ� ��.

    m_hWnd = hWnd;

    SetWindowText(m_hWnd,
        L"������ �¾�踦 ����� �ּ���. ���� ��Ģ�� ���� �մϴ�. ^^;;");

    std::cout << "�¾��� ������ �ؾ� �մϴ�." << std::endl; // ��
    std::cout << "�༺���� ������ �ϸ� ���ÿ� �¾��� ������ ������ �޾� �����ϴ� ��ó�� ���Դϴ�." << std::endl; // ������ �׷��� �߾��.
    std::cout << "���� ������ �ϸ鼭 ���ÿ� ������ ������ ������ �޾� �����ϴ� ��ó�� ���Դϴ�." << std::endl; // ���̶� ���ϸ޵���.
    std::cout << "ȸ�� �ӵ��� �����Ӱ� �����ϼ���." << std::endl; // ��

    // ��Ʈ�� ��������
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/cat.png", *m_BitmapPtr.GetAddressOf()); // ������ ���� ����
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/sun.png", *m_bitmapPtr_Sun.GetAddressOf()); // �¾�
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/mercury.png", *m_bitmapPtr_Mercury.GetAddressOf()); // ����
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/venus.png", *m_bitmapPtr_Venus.GetAddressOf()); // �ݼ�
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/earth.png", *m_bitmapPtr_Earth.GetAddressOf()); // ����
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/moon.png", *m_bitmapPtr_Moon.GetAddressOf()); // ��
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/mars.png", *m_bitmapPtr_Mars.GetAddressOf()); // ȭ��
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/jupiter.png", *m_bitmapPtr_Jupiter.GetAddressOf()); // ��
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/ganymede.png", *m_bitmapPtr_Ganymede.GetAddressOf()); // ���ϸ޵�
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/saturnlol.png", *m_bitmapPtr_Saturn.GetAddressOf()); // �伺
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/uranus.png", *m_bitmapPtr_Uranus.GetAddressOf()); // õ�ռ�
    SolarSystemRenderer::Instance().CreateBitmapFromFile(L"../Resource/neptune.png", *m_bitmapPtr_Neptune.GetAddressOf()); // �ؿռ�

    // õü ��ġ
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
    ProcessKeyboardEvents(); // ī�޶� �̵�.
    
    for (auto& obj : m_CelestialObjs)
    {
        obj->Update(deltaTime); // õü�� ������Ʈ
    }


    // ī�޶� ������Ʈ
    MAT3X2F cameraTM = m_UnityCamera.GetViewMatrix();
    MAT3X2F renderTM = MYTM::MakeRenderMatrix(true); // ī�޶� ��ġ ������ ��Ʈ����
    MAT3X2F finalTM = renderTM * cameraTM;

    // ������
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
    // ������ ũ�� ���� �� ī�޶��� ȭ�� ũ�⸦ ������Ʈ
    m_UnityCamera.SetScreenSize(width, height);
}

void TransformPracticeScene::ProcessKeyboardEvents()
{
    // ī�޶� �̵�
    static const std::vector<std::pair<int, Vec2>> kCameraMoves = {
    { VK_RIGHT, {  1.f,  0.f } },
    { VK_LEFT,  { -1.f,  0.f } },
    { VK_UP,    {  0.f,  1.f } },
    { VK_DOWN,  {  0.f, -1.f } },
    };

    // C++17���ʹ� structured binding�� ����Ͽ� �� �����ϰ� ǥ���� �� �ֽ��ϴ�.
    for (auto& [vk, dir] : kCameraMoves)
    {
        if (InputManager::Instance().GetKeyDown(vk))
        {
            m_UnityCamera.Move(dir.x, dir.y);
        }
    }
}

void TransformPracticeScene::AddCelestialObjs() // õü �߰� // �ϴ� �ϵ��ڵ�.. // ������ ��Ű�� �;����� ����..
{
    //=============================������ ����==================================================
    CelestialObj* ofMercury = new CelestialObj(m_BitmapPtr, 41.f, true); // ������ 
    ofMercury->SetScale(Vec2(0.1f, 0.1f));
    ofMercury->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofMercury);

    CelestialObj* ofVenus = new CelestialObj(m_BitmapPtr, 16.f, true); // �ݼ���
    ofVenus->SetScale(Vec2(0.1f, 0.1f));
    ofVenus->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofVenus);

    CelestialObj* ofMars = new CelestialObj(m_BitmapPtr, 5.f, true); // ȭ����
    ofMars->SetScale(Vec2(0.1f, 0.1f));
    ofMars->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofMars);

    CelestialObj* ofJupiter = new CelestialObj(m_BitmapPtr, 3.f, true); // �񼺿�
    ofJupiter->SetScale(Vec2(0.1f, 0.1f));
    ofJupiter->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofJupiter);

    CelestialObj* ofSaturn = new CelestialObj(m_BitmapPtr, 2.5f, true); // �伺��
    ofSaturn->SetScale(Vec2(0.1f, 0.1f));
    ofSaturn->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofSaturn);

    CelestialObj* ofUranus = new CelestialObj(m_BitmapPtr, 1.5f, true); // õ�ռ���
    ofUranus->SetScale(Vec2(0.1f, 0.1f));
    ofUranus->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofUranus);

    CelestialObj* ofNeptune = new CelestialObj(m_BitmapPtr, 1.f, true); // �ؿռ���
    ofNeptune->SetScale(Vec2(0.1f, 0.1f));
    ofNeptune->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(ofNeptune);
    //=============================������ ���� ��==================================================

    CelestialObj* Sun = new CelestialObj(m_bitmapPtr_Sun, 10.0f, true); // �¾�
    Sun->SetScale(Vec2(1.5f, 1.5f));
    Sun->SetPosition(Vec2(-50.f, 50.f));
    m_CelestialObjs.push_back(Sun);

    CelestialObj* Mercury = new CelestialObj(m_bitmapPtr_Mercury, 15.0f, true); // ����
    Mercury->SetScale(Vec2(0.2f, 0.2f));
    Mercury->SetPosition(Vec2(35.f, 50.f)); // �θ� ���� �ϱ� ���� ������, ��ġ ���� ����.
    Mercury->SetParent(ofMercury);
    m_CelestialObjs.push_back(Mercury);

    CelestialObj* Venus = new CelestialObj(m_bitmapPtr_Venus, -10.0f, true); // �ݼ�
    Venus->SetScale(Vec2(0.35f, 0.35f));
    Venus->SetPosition(Vec2(80.f, 50.f));
    Venus->SetParent(ofVenus);
    m_CelestialObjs.push_back(Venus);

    CelestialObj* Earth = new CelestialObj(m_bitmapPtr_Earth, 25.0f, true); // ����
    Earth->SetScale(Vec2(0.5f, 0.5f));
    Earth->SetPosition(Vec2(150.f, 50.f));
    Earth->SetParent(Sun);
    m_CelestialObjs.push_back(Earth);

    CelestialObj* Moon = new CelestialObj(m_bitmapPtr_Moon, 0.0f, false); // �� // �����ֱ�� �����ֱⰡ ���Ƽ� �׻� ���� ���� ������ ��������. �׷��� false��.
    Moon->SetScale(Vec2(0.15f, 0.15f));
    Moon->SetPosition(Vec2(180.f, 50.f));
    Moon->SetParent(Earth);
    m_CelestialObjs.push_back(Moon);

    CelestialObj* Mars = new CelestialObj(m_bitmapPtr_Mars, 20.0f, true); // ȭ��
    Mars->SetScale(Vec2(0.5f, 0.5f));
    Mars->SetPosition(Vec2(230.f, 50.f));
    Mars->SetParent(ofMars);
    m_CelestialObjs.push_back(Mars);

    CelestialObj* Jupiter = new CelestialObj(m_bitmapPtr_Jupiter, 60.0f, true); // ��
    Jupiter->SetScale(Vec2(1.f, 1.f));
    Jupiter->SetPosition(Vec2(360.f, 50.f));
    Jupiter->SetParent(ofJupiter);
    m_CelestialObjs.push_back(Jupiter);

    CelestialObj* Ganymede = new CelestialObj(m_bitmapPtr_Ganymede, 10.0f, true); // ���ϸ޵� // �� ģ���� �����̶� �����ֱⰡ ������.. �׷��� �������� �; false����.
    Ganymede->SetScale(Vec2(0.25f, 0.25f));
    Ganymede->SetPosition(Vec2(420.f, 50.f));
    Ganymede->SetParent(Jupiter);
    m_CelestialObjs.push_back(Ganymede);

    CelestialObj* Saturn = new CelestialObj(m_bitmapPtr_Saturn, 55.0f, true); // �伺
    Saturn->SetScale(Vec2(0.8f, 0.8f));
    Saturn->SetPosition(Vec2(480.f, 50.f));
    Saturn->SetParent(ofSaturn);
    m_CelestialObjs.push_back(Saturn);

    CelestialObj* Uranus = new CelestialObj(m_bitmapPtr_Uranus, -30.0f, true); // õ�ռ�
    Uranus->SetScale(Vec2(0.6f, 0.6f));
    Uranus->SetPosition(Vec2(580.f, 50.f));
    Uranus->SetParent(ofUranus);
    m_CelestialObjs.push_back(Uranus);

    CelestialObj* Neptune = new CelestialObj(m_bitmapPtr_Neptune, 30.0f, true); // �ؿռ�
    Neptune->SetScale(Vec2(0.6f, 0.6f));
    Neptune->SetPosition(Vec2(670.f, 50.f));
    Neptune->SetParent(ofNeptune);
    m_CelestialObjs.push_back(Neptune);


}

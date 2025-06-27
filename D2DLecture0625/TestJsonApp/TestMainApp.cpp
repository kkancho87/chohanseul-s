#include "pch.h"
#include "GameTimer.h"
#include "D2DRender.h"
#include "TestMainApp.h"

using namespace sample;
////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////
// 
bool TestMainApp::Initialize()
{
    const wchar_t* className = L"D2DLesson2";
    const wchar_t* windowName = L"D2DLesson2";

    if (false == __super::Create(className, windowName, 1024, 800))
    {
        return false;
    }

    m_Renderer = std::make_shared<D2DRenderer>();
    m_Renderer->Initialize(m_hWnd);

    m_GameTimer = std::make_shared<GameTimer>();
    m_GameTimer->Reset();

    TestJsonLoad();
    TestJsonAndImageLoad();

    return true;
}

void TestMainApp::Run()
{
    MSG msg = { 0 };

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            UpdateTime();

            Render();
            
        }
    }
}
void TestMainApp::Finalize()
{
    m_spriteData.m_image.Reset(); // 이미지 리소스 해제
    __super::Destroy();

}


void TestMainApp::UpdateTime()
{
    m_GameTimer->Tick();
}

void TestMainApp::Render()
{
    if (m_Renderer == nullptr) return;
   
    m_Renderer->RenderBegin();

    D2D1_RECT_F targetRect = D2D1::RectF(
        0, 0, 
        static_cast<float>(m_spriteData.m_image->GetSize().width), 
        static_cast<float>(m_spriteData.m_image->GetSize().height));

    m_Renderer->DrawBitmap(m_spriteData.m_image.Get(), targetRect);

    int frameIndex = 0;
    for (auto& f : m_spriteData.m_frames)
    {
        D2D1_RECT_F frameRect = D2D1::RectF(
            static_cast<float>(f.m_frame.m_x),
            static_cast<float>(f.m_frame.m_y),
            static_cast<float>(f.m_frame.m_x + f.m_frame.m_w),
            static_cast<float>(f.m_frame.m_y + f.m_frame.m_h)
        );
  
        int xOffset = static_cast<float>(frameIndex * f.m_frame.m_w) + 10.f;
        int yOffset = 500;
        D2D1_RECT_F renderRect = D2D1::RectF(
            xOffset, yOffset,
            xOffset + f.m_frame.m_w, yOffset + f.m_frame.m_h
        );

        m_Renderer->DrawBitmap(m_spriteData.m_image.Get(), renderRect, frameRect);
        frameIndex++;
    }

    m_Renderer->RenderEnd();
}

void TestMainApp::TestJsonLoad()
{
    std::cout << "TestJsonLoad" << std::endl;

    // JSON 파일을 읽고 파싱해 봅니다. 내용 확인해 보기용!

    AsepriteData data_Local;
    std::string path = "../resource/redbirdSheet.json";

    if (!LoadAsepriteJson(path, data_Local))
    {
        std::cout << "Failed to load JSON file: " << path << std::endl;
        return;
    }

    for (const auto& frame : data_Local.m_frames)
    {
        std::cout << "Frame: " << frame.m_filename 
                  << " - X: " << frame.m_frame.m_x 
                  << ", Y: " << frame.m_frame.m_y 
                  << ", Width: " << frame.m_frame.m_w 
                  << ", Height: " << frame.m_frame.m_h
                  << ", Duration: " << frame.m_duration
                  << std::endl;
    }

}

std::wstring ConvertToWString(const std::string& str)
{
    size_t len = 0;
    mbstowcs_s(&len, nullptr, 0, str.c_str(), _TRUNCATE);
    if (len == 0)
        return L"";

    std::wstring wstr(len, L'\0');
    mbstowcs_s(&len, &wstr[0], len, str.c_str(), _TRUNCATE);
    wstr.resize(len - 1); // Remove the null terminator added by mbstowcs_s  
    return wstr;
}


void TestMainApp::TestJsonAndImageLoad()
{
    std::wcout << "TestJsonAndImageLoad" << std::endl;
    // JSON 파일과 이미지 파일을 읽고 파싱해 봅니다.
    
    std::string jsonPath = "../resource/redbirdSheet.json";
    std::string imagePath = "../resource/redbirdSheet.png";

    if (!LoadAsepriteJson(jsonPath, m_spriteData))
    {
        std::cout << "Failed to load JSON file: " << jsonPath << std::endl;
        return;
    }

    std::wstring wImagePath = ConvertToWString(imagePath);
    m_Renderer->CreateBitmapFromFile(wImagePath.c_str(), *m_spriteData.m_image.GetAddressOf());

    std::cout << "Loaded " << m_spriteData.m_frames.size() << " frames\n";
}

void TestMainApp::OnResize(int width, int height)
{
    __super::OnResize(width, height);

    if (m_Renderer != nullptr) m_Renderer->Resize(width, height);
}

void TestMainApp::OnClose()
{
    if (m_Renderer != nullptr) m_Renderer->Uninitialize();

    std::cout << "OnClose" << std::endl;
}


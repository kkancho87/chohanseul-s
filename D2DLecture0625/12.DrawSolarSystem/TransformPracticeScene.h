#pragma once
#include "OnlyForTestScene.h"
#include "Camera2D.h"
#include <list>
#include <vector>
#include <wrl/client.h>
#include <d2d1_1.h>

class CelestialObj;

class TransformPracticeScene : public OnlyForTestScene
{
public:
    TransformPracticeScene() = default;
    virtual ~TransformPracticeScene();
    
    void SetUp(HWND hWnd) override;

    void Tick(float deltaTime) override;

    void OnResize(int width, int height) override;

private:

    void ProcessKeyboardEvents();

    void AddCelestialObjs();
   
    UnityCamera m_UnityCamera;

    ComPtr<ID2D1Bitmap1> m_BitmapPtr;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Sun;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Mercury;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Venus;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Earth;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Moon;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Mars;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Jupiter;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Ganymede;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Saturn;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Uranus;
    ComPtr<ID2D1Bitmap1> m_bitmapPtr_Neptune;

    std::vector<CelestialObj*> m_CelestialObjs;
};


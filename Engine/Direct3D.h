#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#define SAFE_RELEASE(p) if(p != nullptr){ p->Release(); p = nullptr;}
#define SAFE_DELETE(p) if(p != nullptr){ delete p; p = nullptr;}

//リンカ
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;

enum SHADER_TYPE
{
	SHADER_2D,
	SHADER_3D,
	SHADER_POINT,
	SHADER_MAX
};


namespace Direct3D
{
	extern ID3D11Device* pDevice_;			//デバイス
	extern ID3D11DeviceContext* pContext_;	//デバイスコンテキスト

	//初期化
	HRESULT Initialize(int winW, int winH, HWND hWnd);

	//シェーダー準備
	HRESULT InitShader();
	HRESULT InitShader3D();
	HRESULT InitShader2D();
	HRESULT InitPointLightShader();

	void SetShader(SHADER_TYPE type);

	//描画開始
	void BeginDraw();

	//描画終了
	void EndDraw();

	//解放
	void Release();

	XMFLOAT4 GetLightPos();
	void SetLightPos(XMFLOAT4 pos);
};
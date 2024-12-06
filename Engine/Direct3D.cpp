#include <d3dcompiler.h>
#include "Direct3D.h"
#include <DirectXMath.h>
#include <cassert>
#include <vector>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"

//変数
namespace Direct3D
{
	ID3D11Device* pDevice_;		//デバイス
	ID3D11DeviceContext* pContext_;		//デバイスコンテキスト
	IDXGISwapChain* pSwapChain_;		//スワップチェイン
	ID3D11RenderTargetView* pRenderTargetView_;	//レンダーターゲットビュー
	ID3D11Texture2D* pDepthStencil;				//深度ステンシル
	ID3D11DepthStencilView* pDepthStencilView;	//深度ステンシルビュー

	struct SHADER_BUNDLE
	{
		ID3D11VertexShader* pVertexShader_ = nullptr;	//頂点シェーダー
		ID3D11PixelShader* pPixelShader_ = nullptr;		//ピクセルシェーダー
		ID3D11InputLayout* pVertexLayout_ = nullptr;	//頂点インプットレイアウト
		ID3D11RasterizerState* pRasterizerState_ = nullptr;	//ラスタライザー
	};
	SHADER_BUNDLE shaderBundle[SHADER_MAX];
	XMFLOAT4 lightPos{ 0, 0.5, 0, 0 };
}



//初期化
HRESULT Direct3D::Initialize(int winW, int winH, HWND hWnd)
{
	HRESULT hr; //エラー処理用

	///////////////////////////いろいろ準備するための設定///////////////////////////////
//いろいろな設定項目をまとめた構造体
	DXGI_SWAP_CHAIN_DESC scDesc;

	//とりあえず全部0
	ZeroMemory(&scDesc, sizeof(scDesc));

	//描画先のフォーマット
	scDesc.BufferDesc.Width = winW;		//画面幅
	scDesc.BufferDesc.Height = winH;	//画面高さ
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 何色使えるか

	//FPS（1/60秒に1回）
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;

	//その他
	scDesc.Windowed = TRUE;			//ウィンドウモードかフルスクリーンか
	scDesc.OutputWindow = hWnd;		//ウィンドウハンドル
	scDesc.BufferCount = 1;			//バックバッファの枚数
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//バックバッファの使い道＝画面に描画するために
	scDesc.SampleDesc.Count = 1;		//MSAA（アンチエイリアス）の設定
	scDesc.SampleDesc.Quality = 0;		//　〃


	////////////////上記設定をもとにデバイス、コンテキスト、スワップチェインを作成////////////////////////
	D3D_FEATURE_LEVEL level;
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,				// どのビデオアダプタを使用するか？既定ならばnullptrで
		D3D_DRIVER_TYPE_HARDWARE,		// ドライバのタイプを渡す。ふつうはHARDWARE
		nullptr,				// 上記をD3D_DRIVER_TYPE_SOFTWAREに設定しないかぎりnullptr
		0,					// 何らかのフラグを指定する。（デバッグ時はD3D11_CREATE_DEVICE_DEBUG？）
		nullptr,				// デバイス、コンテキストのレベルを設定。nullptrにしとけばOK
		0,					// 上の引数でレベルを何個指定したか
		D3D11_SDK_VERSION,			// SDKのバージョン。必ずこの値
		&scDesc,				// 上でいろいろ設定した構造体
		&pSwapChain_,				// 無事完成したSwapChainのアドレスが返ってくる
		&pDevice_,				// 無事完成したDeviceアドレスが返ってくる
		&level,					// 無事完成したDevice、Contextのレベルが返ってくる
		&pContext_);				// 無事完成したContextのアドレスが返ってくる

	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "デバイス、コンテキスト、スワップチェインの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}

	///////////////////////////レンダーターゲットビュー作成///////////////////////////////
//スワップチェーンからバックバッファを取得（バックバッファ ＝ レンダーターゲット）
	ID3D11Texture2D* pBackBuffer;
	hr = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
	{
		MessageBox(NULL, "バックバッファの取得に失敗しました", "エラー", MB_OK);
		//エラー処理
		return hr;
	}

	//レンダーターゲットビューを作成
	hr = pDevice_->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView_);
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "レンダーターゲットビューの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}
	//一時的にバックバッファを取得しただけなので解放
	pBackBuffer->Release();


	///////////////////////////ビューポート（描画範囲）設定///////////////////////////////
//レンダリング結果を表示する範囲
	D3D11_VIEWPORT vp;
	vp.Width = (float)winW;	//幅
	vp.Height = (float)winH;//高さ
	vp.MinDepth = 0.0f;	//手前
	vp.MaxDepth = 1.0f;	//奥
	vp.TopLeftX = 0;	//左
	vp.TopLeftY = 0;	//上



	//深度ステンシルビューの作成
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = winW;
	descDepth.Height = winH;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	pDevice_->CreateTexture2D(&descDepth, NULL, &pDepthStencil);
	pDevice_->CreateDepthStencilView(pDepthStencil, NULL, &pDepthStencilView);


	//データを画面に描画するための一通りの設定（パイプライン）
	pContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // データの入力種類を指定
	pContext_->OMSetRenderTargets(1, &pRenderTargetView_, nullptr);            // 描画先を設定
	pContext_->OMSetRenderTargets(1, &pRenderTargetView_, pDepthStencilView);            // 描画先を設定
	pContext_->RSSetViewports(1, &vp);

	//シェーダー準備
	hr = InitShader();
	if (FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

//シェーダー準備
HRESULT Direct3D::InitShader()
{

	if (FAILED(InitShader3D()))
	{
		return E_FAIL;
	}

	if (FAILED(InitShader2D()))
	{
		return E_FAIL;
	}

	if (FAILED(InitPointLightShader()))
	{
		return E_FAIL;
	}

	return S_OK;


}

HRESULT Direct3D::InitShader3D()
{
	using namespace Direct3D;
	HRESULT hr;
	// 頂点シェーダの作成（コンパイル）
	ID3DBlob* pCompileVS = nullptr;
	D3DCompileFromFile(L"Simple3D.hlsl", nullptr, nullptr, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS != nullptr); //ここはassertionで処理

	hr = pDevice_->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(),
		NULL, &(shaderBundle[SHADER_3D].pVertexShader_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "頂点シェーダーの作成に失敗しました", "エラー", MB_OK);
		//解放処理
		return hr;
	}

	//頂点インプットレイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },	//位置
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::XMVECTOR) , D3D11_INPUT_PER_VERTEX_DATA, 0 },//UV座標
		{ "NORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(DirectX::XMVECTOR) * 2 ,	D3D11_INPUT_PER_VERTEX_DATA, 0 },//法線
	};
	hr = pDevice_->CreateInputLayout(layout, 3, pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(),
		&(shaderBundle[SHADER_3D].pVertexLayout_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "頂点インプットレイアウトの作成に失敗しました", "エラー", MB_OK);
		//解放処理
		SAFE_RELEASE(pCompileVS);
		return hr;
	}
	SAFE_RELEASE(pCompileVS);

	// ピクセルシェーダの作成（コンパイル）
	ID3DBlob* pCompilePS = nullptr;
	D3DCompileFromFile(L"Simple3D.hlsl", nullptr, nullptr, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS != nullptr);

	hr = pDevice_->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &(shaderBundle[SHADER_3D].pPixelShader_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "ピクセルシェーダの作成に失敗しました", "エラー", MB_OK);
		SAFE_RELEASE(pCompilePS);
		return hr;
	}

	SAFE_RELEASE(pCompilePS);

	//ラスタライザ作成
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = FALSE;
	hr = pDevice_->CreateRasterizerState(&rdc, &(shaderBundle[SHADER_3D].pRasterizerState_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "ラスタライザの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}

	return S_OK;
}

HRESULT Direct3D::InitShader2D()
{
	HRESULT hr;

	// 頂点シェーダの作成（コンパイル）
	ID3DBlob* pCompileVS = nullptr;
	D3DCompileFromFile(L"Simple2D.hlsl", nullptr, nullptr, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS != nullptr);
	hr = pDevice_->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &(shaderBundle[SHADER_2D].pVertexShader_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "頂点シェーダーの作成に失敗しました", "エラー", MB_OK);
		SAFE_RELEASE(pCompileVS);
		//解放処理
		return hr;
	}

	//頂点インプットレイアウト
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },	//位置
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::XMVECTOR) , D3D11_INPUT_PER_VERTEX_DATA, 0 },//UV座標
	};
	hr = pDevice_->CreateInputLayout(layout.data(), layout.size(),
		pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &(shaderBundle[SHADER_2D].pVertexLayout_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "頂点インプットレイアウトの作成に失敗しました", "エラー", MB_OK);
		//解放処理
		SAFE_RELEASE(pCompileVS);
		return hr;
	}
	SAFE_RELEASE(pCompileVS);

	// ピクセルシェーダの作成（コンパイル）
	ID3DBlob* pCompilePS = nullptr;
	D3DCompileFromFile(L"Simple2D.hlsl", nullptr, nullptr, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS != nullptr);

	hr = pDevice_->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &(shaderBundle[SHADER_2D].pPixelShader_));
	SAFE_RELEASE(pCompilePS);
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "ピクセルシェーダの作成に失敗しました", "エラー", MB_OK);
		//SAFE_RELEASE(pCompilePS);
		return hr;
	}

	//SAFE_RELEASE(pCompilePS);

	//ラスタライザ作成
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = FALSE;
	hr = pDevice_->CreateRasterizerState(&rdc, &(shaderBundle[SHADER_2D].pRasterizerState_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "ラスタライザの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}


	return S_OK;
}

HRESULT Direct3D::InitPointLightShader()
{

	using namespace Direct3D;
	HRESULT hr;
	// 頂点シェーダの作成（コンパイル）
	ID3DBlob* pCompileVS = nullptr;
	D3DCompileFromFile(L"PointLightShader.hlsl", nullptr, nullptr, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS != nullptr); //ここはassertionで処理

	hr = pDevice_->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(),
		NULL, &(shaderBundle[SHADER_POINT].pVertexShader_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "頂点シェーダーの作成に失敗しました", "エラー", MB_OK);
		//解放処理
		return hr;
	}

	//頂点インプットレイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },	//位置
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(DirectX::XMVECTOR) , D3D11_INPUT_PER_VERTEX_DATA, 0 },//UV座標
		{ "NORMAL",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(DirectX::XMVECTOR) * 2 ,	D3D11_INPUT_PER_VERTEX_DATA, 0 },//法線
	};
	hr = pDevice_->CreateInputLayout(layout, 3, pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(),
		&(shaderBundle[SHADER_POINT].pVertexLayout_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "頂点インプットレイアウトの作成に失敗しました", "エラー", MB_OK);
		//解放処理
		SAFE_RELEASE(pCompileVS);
		return hr;
	}
	SAFE_RELEASE(pCompileVS);

	// ピクセルシェーダの作成（コンパイル）
	ID3DBlob* pCompilePS = nullptr;
	D3DCompileFromFile(L"PointLightShader.hlsl", nullptr, nullptr, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompilePS != nullptr);

	hr = pDevice_->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &(shaderBundle[SHADER_POINT].pPixelShader_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "ピクセルシェーダの作成に失敗しました", "エラー", MB_OK);
		SAFE_RELEASE(pCompilePS);
		return hr;
	}

	SAFE_RELEASE(pCompilePS);

	//ラスタライザ作成
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = FALSE;
	hr = pDevice_->CreateRasterizerState(&rdc, &(shaderBundle[SHADER_POINT].pRasterizerState_));
	if (FAILED(hr))
	{
		//エラー処理
		MessageBox(NULL, "ラスタライザの作成に失敗しました", "エラー", MB_OK);
		return hr;
	}

	return S_OK;

}


void Direct3D::SetShader(SHADER_TYPE type)
{
	//それぞれをデバイスコンテキストにセット
	pContext_->VSSetShader(shaderBundle[type].pVertexShader_, NULL, 0);	//頂点シェーダー
	pContext_->PSSetShader(shaderBundle[type].pPixelShader_, NULL, 0);	//ピクセルシェーダー
	pContext_->IASetInputLayout(shaderBundle[type].pVertexLayout_);	//頂点インプットレイアウト
	pContext_->RSSetState(shaderBundle[type].pRasterizerState_);		//ラスタライザー
}

//描画開始
void Direct3D::BeginDraw()
{
	//背景の色
	float clearColor[4] = { 0.0f, 0.5f, 0.5f, 1.0f };//R,G,B,A

	//画面をクリア
	pContext_->ClearRenderTargetView(pRenderTargetView_, clearColor);

	//深度バッファクリア
	pContext_->ClearDepthStencilView(pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}



//描画終了

void Direct3D::EndDraw()
{


	//ImGui::Begin("TestWin");
	
	ImGui::Button("Button");
	//ImGui::End();
	//ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	//スワップ（バックバッファを表に表示する）
	pSwapChain_->Present(0, 0);


}



//解放処理
void Direct3D::Release()
{

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	//解放処理
	for (int i = 0; i < SHADER_MAX; i++)
	{
		SAFE_RELEASE(shaderBundle[i].pRasterizerState_);
		SAFE_RELEASE(shaderBundle[i].pVertexLayout_);
		SAFE_RELEASE(shaderBundle[i].pPixelShader_);
		SAFE_RELEASE(shaderBundle[i].pVertexShader_);
	}

	SAFE_RELEASE(pRenderTargetView_);
	SAFE_RELEASE(pSwapChain_);
	SAFE_RELEASE(pContext_);
	SAFE_RELEASE(pDevice_);
}

XMFLOAT4 Direct3D::GetLightPos()
{
	return lightPos;
}

void Direct3D::SetLightPos(XMFLOAT4 pos)
{
	lightPos = pos;
}

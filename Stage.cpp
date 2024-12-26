#include "Stage.h"
#include "Engine/Model.h"
#include "Engine/Input.h"
#include "Engine/Camera.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

namespace
{
    XMFLOAT4  lpos_backup[POINT_LIGHT_MAX];
    bool isRoateLight = false;
}

void Stage::InitConstantBuffer()
{
    D3D11_BUFFER_DESC cb;
    CONSTBUFFER_STAGE m;
    //cb.ByteWidth = sizeof(CONSTBUFFER_STAGE) + (sizeof(CONSTBUFFER_STAGE) % 16 == 0 ? 0 : 16 - sizeof(CONSTBUFFER_STAGE) % 16);
    cb.ByteWidth = sizeof(m) + (sizeof(m) % 16 == 0 ? 0 : 16 - sizeof(m) % 16);
    cb.Usage = D3D11_USAGE_DYNAMIC;
    cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cb.MiscFlags = 0;
    cb.StructureByteStride = 0;
    HRESULT hr;

    hr = Direct3D::pDevice_->CreateBuffer(&cb, nullptr, &pCBStage_);
    if (FAILED(hr))
    {
        MessageBox(NULL, "コンスタントバッファの作成に失敗しました", "エラー", MB_OK);
    }
}

//コンストラクタ
Stage::Stage(GameObject* parent)
    :GameObject(parent, "Stage"), pCBStage_(nullptr)
{
    hModel_ = -1;
    hGround = -1;
    hRoom_ = -1;
    hBunny_ = -1;
}

//デストラクタ
Stage::~Stage()
{

}

//初期化
void Stage::Initialize()
{
    hModel_ = Model::Load("Assets\\Ball.fbx");
    hRoom_ = Model::Load("Assets\\room.fbx");
    hGround = Model::Load("Assets\\plane3.fbx");
    hBunny_ = Model::Load("Assets\\Donut_phong.fbx");
    Camera::SetPosition(XMFLOAT3{ 0, 0.8, -2.8 });
    Camera::SetTarget(XMFLOAT3{ 0,0.8,0 });
    sptlight_ =
    {
        Direct3D::GetLightPos(),
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 0, -1, 0, 0.0 },
        40.0f,
        50.0f,
        0.1f,
        1.0f
    };
    
    ptlight_[0].lightPosition = { -0.5,  0.2, 0, 1.0 };
    ptlight_[1].lightPosition = {  0.5,  0.2, 0, 1.0 };
    ptlight_[2].lightPosition = { 0.0,  0.2, -0.2, 1.0 };
    ptlight_[3].lightPosition = { 0, 0, 0, 1.0 };
    ptlight_[4].lightPosition = { 0, 0, 0, 1.0 };
    ptlight_[0].pointLightColor = { 1,0, 0, 1.0 };
    ptlight_[1].pointLightColor = { 0, 1, 0, 1.0 };
    ptlight_[2].pointLightColor = { 0, 0, 1, 1.0 };
    ptlight_[3].pointLightColor = { 1, 1, 1, 1.0 };
    ptlight_[4].pointLightColor = { 1, 1, 1, 1.0 };
    ptlight_[0].kTerm = { 0.2f, 0.2f, 1.0f, 1.0f };
    ptlight_[1].kTerm = { 0.2f, 0.2f, 1.0f, 1.0f };
    ptlight_[2].kTerm = { 0.2f, 0.2f, 1.0f, 1.0f };
    ptlight_[3].kTerm = { 0.2f, 0.2f, 1.0f, 1.0f };
    ptlight_[4].kTerm = { 0.2f, 0.2f, 1.0f, 1.0f };
    ptlight_[0].sw = 1;
    ptlight_[1].sw = 1;
    ptlight_[2].sw = 1;
    ptlight_[3].sw = 0;
    ptlight_[4].sw = 0;
    

    for (int i = 0; i < POINT_LIGHT_MAX; i++)
    {
        lpos_backup[i] = ptlight_[i].lightPosition;
    }
    InitConstantBuffer();
}

//更新
void Stage::Update()
{
    transform_.rotate_.y += 0.5f;
    if (Input::IsKey(DIK_A))
    {
        XMFLOAT4 p = Direct3D::GetLightPos();
        p = { p.x - 0.01f,p.y, p.z,p.w };
        Direct3D::SetLightPos(p);
    }
    if (Input::IsKey(DIK_D))
    {
        XMFLOAT4 p = Direct3D::GetLightPos();
        p = { p.x + 0.01f,p.y, p.z,p.w };
        Direct3D::SetLightPos(p);
    }
    if (Input::IsKey(DIK_W))
    {
        XMFLOAT4 p = Direct3D::GetLightPos();
        p = { p.x,p.y, p.z + 0.01f,p.w };
        Direct3D::SetLightPos(p);
    }
    if (Input::IsKey(DIK_S))
    {
        XMFLOAT4 p = Direct3D::GetLightPos();
        p = { p.x ,p.y, p.z - 0.01f,p.w };
        Direct3D::SetLightPos(p);
    }
    if (Input::IsKey(DIK_UP))
    {
        XMFLOAT4 p = Direct3D::GetLightPos();
        p = { p.x,p.y + 0.01f, p.z,p.w };
        Direct3D::SetLightPos(p);
    }
    if (Input::IsKey(DIK_DOWN))
    {
        XMFLOAT4 p = Direct3D::GetLightPos();
        p = { p.x ,p.y - 0.01f, p.z,p.w };
        Direct3D::SetLightPos(p);
    }
    sptlight_.LightPosition = Direct3D::GetLightPos();
    //コンスタントバッファの設定と、シェーダーへのコンスタントバッファのセットを書くよ
    CONSTBUFFER_STAGE cb;
    //spotlight
    cb.sptLightPosition = sptlight_.LightPosition;
    XMStoreFloat4(&cb.eyePosition, Camera::GetPosition());
    //cb.pLightPosition = { -1.0, 1.0, 2.0, 1.0 };
    //XMStoreFloat4(&cb.eyePosition, Camera::GetPosition());
    //cb.pLightPosition = sptlight_.pLightPosition;
    cb.sptLightColor = sptlight_.color;
    cb.direction = sptlight_.direction;
    cb.sptLightparam = { sptlight_.theta,
                         sptlight_.phi,
                         sptlight_.att,
                         sptlight_.toff };
    //pointlight
    for (int i = 0; i < POINT_LIGHT_MAX; i++)
    {
        cb.pointlightPosition[i] = ptlight_[i].lightPosition;
        cb.pointLightColor[i] = ptlight_[i].pointLightColor;
        cb.kTerm[i] = ptlight_[i].kTerm;
        cb.pointListSW[i] = { ptlight_[i].sw, 0, 0,0 };
    }

    D3D11_MAPPED_SUBRESOURCE pdata;
    Direct3D::pContext_->Map(pCBStage_, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);	// GPUからのデータアクセスを止める
    ULONG cbsize =  sizeof(cb);

    memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), cbsize);	// データを値を送る
    Direct3D::pContext_->Unmap(pCBStage_, 0);	//再開

    //コンスタントバッファ
    Direct3D::pContext_->VSSetConstantBuffers(1, 1, &pCBStage_);	//頂点シェーダー用	
    Direct3D::pContext_->PSSetConstantBuffers(1, 1, &pCBStage_);	//ピクセルシェーダー用
}

//描画
void Stage::Draw()
{
    Transform ltr;
    ltr.position_ = { Direct3D::GetLightPos().x,Direct3D::GetLightPos().y,Direct3D::GetLightPos().z };
    ltr.scale_ = { 0.1,0.1,0.1 };
    Model::SetTransform(hModel_, ltr);
    Model::Draw(hModel_);

    Transform tr;
    tr.position_ = { 0, 0, 0 };
    //tr.scale_ = { 5.0f, 5.0f, 5.0f };
    tr.rotate_ = { 0,0,0 };
    //Model::SetTransform(hGround, tr);
    //Model::Draw(hGround);

    Model::SetTransform(hRoom_, tr);
    Model::Draw(hRoom_);

    static Transform tbunny;
    tbunny.scale_ = { 0.25,0.25,0.25 };
    tbunny.position_ = { 0, 0.5, 0 };
    tbunny.rotate_.y += 0.1;
    Model::SetTransform(hBunny_, tbunny);
    Model::Draw(hBunny_);

    static float lightRotAngle = 0;
    XMVECTOR pt[POINT_LIGHT_MAX];
    if (isRoateLight) {
        for (int i = 0; i < POINT_LIGHT_MAX; i++)
        {
            pt[i] = XMLoadFloat4(&lpos_backup[i]);
        }
        XMMATRIX yrot = XMMatrixRotationY(lightRotAngle);
        for (int i = 0; i < POINT_LIGHT_MAX; i++)
        {
            //ptlight_[i].lightPosition = XMVector3TransformCoord(pt[i], yrot);
            XMStoreFloat4(&(ptlight_[i].lightPosition),
                XMVector3TransformCoord(pt[i], yrot));
        }
    }
    else
    {
        for (int i = 0; i < POINT_LIGHT_MAX; i++)
        {
           ptlight_[i].lightPosition = lpos_backup[i];
        }
    }

    {
        ImGui::Text("Spot Light Params");

        ImGui::SliderFloat("phi(theta < phi)", &sptlight_.phi, sptlight_.theta, 180);
        ImGui::SliderFloat("theta", &sptlight_.theta, 1, 180);
        float dirval[4] = { sptlight_.direction.x,sptlight_.direction.y, sptlight_.direction.z,};
        ImGui::SliderFloat3("Light direction", dirval, -2.0, 2.0);
        dirval[3] = 1.0;
        sptlight_.direction = XMFLOAT4(dirval); 
        ImGui::Separator();
        ImGui::Text("pos:%.3f,%.3f,%.3f", sptlight_.LightPosition.x, sptlight_.LightPosition.y, sptlight_.LightPosition.z);
        ImGui::Text("dir:%.3f,%.3f,%.3f", sptlight_.direction.x, sptlight_.direction.y, sptlight_.direction.z);
        ImGui::Text("phi:%.3f", sptlight_.phi);

        ImGui::Separator();

        ImGui::Text("Point Lights Switch");
        ImGui::Separator();
        bool sw[3] = { (bool)ptlight_[0].sw,(bool)ptlight_[1].sw, (bool)ptlight_[2].sw };
        ImGui::Columns(3, NULL, true);
        ImGui::Checkbox("pLight0", &sw[0]);  ImGui::NextColumn();
        ImGui::Checkbox("pLight1", &sw[1]);  ImGui::NextColumn();
        ImGui::Checkbox("pLight2", &sw[2]);  ImGui::NextColumn();
        ptlight_[0].sw = sw[0];
        ptlight_[1].sw = sw[1];
        ptlight_[2].sw = sw[2];
        ImGui::Columns(1);
        ImGui::Separator();

        lightRotAngle += Direct3D::GetDeltaT() / 1000;
        //mGui::Text("deltaT:%.2f ms", lightRotAngle);
        ImGui::Checkbox("Rotation Light", &isRoateLight);
    }
}

//開放
void Stage::Release()
{
}
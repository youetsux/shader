#include "Stage.h"
#include "Engine/Model.h"
#include "Engine/Input.h"

//コンストラクタ
Stage::Stage(GameObject* parent)
    :GameObject(parent, "Stage")
{
    hModel_[0] = -1;
    hModel_[1] = -1;
    hGround = -1;
    hModel_[2] = -1;
}

//デストラクタ
Stage::~Stage()
{
}

//初期化
void Stage::Initialize()
{
    hModel_[0] = Model::Load("Assets\\Ball.fbx");
    hModel_[1] = Model::Load("Assets\\tama1.fbx");
    hModel_[2] = Model::Load("Assets\\tama2.fbx");
    hGround = Model::Load("Assets\\plane.fbx");
}

//更新
void Stage::Update()
{
    transform_.rotate_.y += 0.5f;
    if (Input::IsKey(DIK_A))
    {
        XMFLOAT4 p = Direct3D::GetLightPos();
        p = { p.x - 0.1f,p.y, p.z,p.w };
        Direct3D::SetLightPos(p);
    }
    if (Input::IsKey(DIK_D))
    {
        XMFLOAT4 p = Direct3D::GetLightPos();
        p = { p.x + 0.1f,p.y, p.z,p.w };
        Direct3D::SetLightPos(p);
    }
}

//描画
void Stage::Draw()
{
    for (int i = 0; i < 3; i++) {
        Transform tr;
        tr.position_ = { transform_.position_.x + (float)i*2.0f, transform_.position_.y ,transform_.position_.z };
        tr.scale_ = transform_.scale_;
        tr.rotate_ = transform_.rotate_;
        Model::SetTransform(hModel_[i], tr);
        Model::Draw(hModel_[i]);
    }

    Transform tr;
    tr.position_ = { 0, -2, 0 };
    tr.scale_ = { 15.0f, 15.0f, 15.0f };
    tr.rotate_ = { 0,0,0 };
    Model::SetTransform(hGround, tr);
    Model::Draw(hGround);
}

//開放
void Stage::Release()
{
}
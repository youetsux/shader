#include "Stage.h"
#include "Engine/Model.h"

//�R���X�g���N�^
Stage::Stage(GameObject* parent)
    :GameObject(parent, "Stage")
{
    hModel_[0] = -1;
    hModel_[1] = -1;
    hModel_[2] = -1;
}

//�f�X�g���N�^
Stage::~Stage()
{
}

//������
void Stage::Initialize()
{
    hModel_[0] = Model::Load("Assets\\Ball.fbx");
    hModel_[1] = Model::Load("Assets\\tama1.fbx");
    hModel_[2] = Model::Load("Assets\\tama2.fbx");
}

//�X�V
void Stage::Update()
{
    transform_.rotate_.y += 0.5f;
}

//�`��
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

}

//�J��
void Stage::Release()
{
}
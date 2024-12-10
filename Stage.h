#pragma once
#include "Engine/GameObject.h"

struct CONSTBUFFER_STAGE
{
    XMFLOAT4 lightPosition; //�����ʒu
    XMFLOAT4 eyePosition;//���_�̈ʒu
};

//���������Ǘ�����N���X
class Stage : public GameObject
{
    int hModel_;    //���f���ԍ�
    int hRoom_;
    int hGround;
    int hBunny_;
    ID3D11Buffer* pConstantBuffer_;
    void InitConstantBuffer();
public:
    //�R���X�g���N�^
    Stage(GameObject* parent);

    //�f�X�g���N�^
    ~Stage();

    //������
    void Initialize() override;

    //�X�V
    void Update() override;

    //�`��
    void Draw() override;

    //�J��
    void Release() override;
};
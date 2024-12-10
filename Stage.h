#pragma once
#include "Engine/GameObject.h"

struct CONSTBUFFER_STAGE
{
    XMFLOAT4 lightPosition; //光源位置
    XMFLOAT4 eyePosition;//視点の位置
};

//◆◆◆を管理するクラス
class Stage : public GameObject
{
    int hModel_;    //モデル番号
    int hRoom_;
    int hGround;
    int hBunny_;
    ID3D11Buffer* pConstantBuffer_;
    void InitConstantBuffer();
public:
    //コンストラクタ
    Stage(GameObject* parent);

    //デストラクタ
    ~Stage();

    //初期化
    void Initialize() override;

    //更新
    void Update() override;

    //描画
    void Draw() override;

    //開放
    void Release() override;
};
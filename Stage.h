#pragma once
#include "Engine/GameObject.h"

struct CONSTBUFFER_STAGE
{
    XMFLOAT4 lightPosition; //光源位置
    XMFLOAT4 eyePosition;//視点の位置
    XMFLOAT4 pLightPosition;
    XMFLOAT4 color;
    XMFLOAT4 direction;
    float theta;//theta phi<---<---theta--->--->phi
    float phi;//phi phi<---<---theta--->--->phi
    float att;
    float toff;
};

struct spotLightState
{
    XMFLOAT4 pLightPosition;
    XMFLOAT4 color;
    XMFLOAT4 direction;
    float theta;//theta phi<---<---theta--->--->phi
    float phi;//phi phi<---<---theta--->--->phi
    float att;
    float toff;
};


//◆◆◆を管理するクラス
class Stage : public GameObject
{
    int hModel_;    //モデル番号
    int hRoom_;
    int hGround;
    int hBunny_;
    ID3D11Buffer* pCBStage_;
    //ID3D11Buffer* pCBSpot_;
    void InitConstantBuffer();
    spotLightState sptlight_;
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
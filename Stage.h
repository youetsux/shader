#pragma once
#include "Engine/GameObject.h"

namespace
{
    const int POINT_LIGHT_MAX{ 5 };
}


struct CONSTBUFFER_STAGE
{
    XMFLOAT4 pointlightPosition[POINT_LIGHT_MAX];  //点光源位置最大5個
    XMFLOAT4 eyePosition;    //視点の位置
    XMFLOAT4 sptLightPosition; //スポットライトの位置
    XMFLOAT4 pointLightColor[POINT_LIGHT_MAX];
    XMFLOAT4 sptLightColor;
    XMFLOAT4 direction;
    XMFLOAT4 kTerm[POINT_LIGHT_MAX];
    XMFLOAT4 sptLightparam;
    XMINT4 pointListSW[POINT_LIGHT_MAX];//点光源のスイッチ
    };

struct spotLightState
{
    XMFLOAT4 LightPosition;
    XMFLOAT4 color;
    XMFLOAT4 direction;
    float theta;//theta phi<---<---theta--->--->phi
    float phi;//phi phi<---<---theta--->--->phi
    float att;
    float toff;
};

struct pointLightState
{
    XMFLOAT4 lightPosition;
    XMFLOAT4 pointLightColor;
    XMFLOAT4 kTerm;
    int sw;
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
    pointLightState ptlight_[POINT_LIGHT_MAX];
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
#pragma once
#include "Engine/GameObject.h"

class Fbx;

class Childoden :
    public GameObject
{
    Fbx* pFbx;
    int hModel_;//モデル番号
public:
    //コンストラクタ
    Childoden(GameObject* parent);

    //デストラクタ
    ~Childoden();

    //初期化
    void Initialize() override;

    //更新
    void Update() override;

    //描画
    void Draw() override;

    //開放
    void Release() override;
};


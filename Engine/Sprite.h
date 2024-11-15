#pragma once
#include "Direct3D.h"
#include "Texture.h"
#include <vector>
#include "Transform.h"



#define SAFE_DELETE_ARRAY(p) if(p != nullptr){ delete[] p; p = nullptr;}



//四角形ポリゴン（三角形を２枚）を描画するクラス
class Sprite
{
	//コンスタントバッファー
	struct CONSTANT_BUFFER
	{
		XMMATRIX	matW;		//ワールド行列
	};

	//頂点情報
	struct VERTEX
	{
		XMVECTOR position;	//位置
		XMVECTOR uv;		//UV
	};

protected:
	uint64_t vertexNum_;		//頂点数
	std::vector<VERTEX> vertices_;		//頂点情報
	ID3D11Buffer* pVertexBuffer_;		//頂点バッファ

	uint64_t indexNum;			//インデックス数
	std::vector<int> index_;			//インデックス情報
	ID3D11Buffer* pIndexBuffer_;		//インデックスバッファ

	ID3D11Buffer* pConstantBuffer_;	//コンスタントバッファ

	Texture* pTexture_;		//テクスチャ


public:
	Sprite();
	~Sprite();

	//初期化（ポリゴンを表示するための各種情報を準備）
	//戻値：成功／失敗
	HRESULT Initialize();

	//描画
	//引数：transform	トランスフォームクラスオブジェクト
	void Draw(Transform& transform);

	//解放
	void Release();



private:
	//---------Initializeから呼ばれる関数---------
	virtual void InitVertexData();		//頂点情報の準備
	HRESULT CreateVertexBuffer();		//頂点バッファを作成

	virtual void InitIndexData();		//インデックス情報を準備
	HRESULT CreateIndexBuffer();		//インデックスバッファを作成

	HRESULT CreateConstantBuffer();		//コンスタントバッファ作成

	HRESULT LoadTexture();				//テクスチャをロード


	//---------Draw関数から呼ばれる関数---------
	void PassDataToCB(XMMATRIX worldMatrix);	//コンスタントバッファに各種情報を渡す
	void SetBufferToPipeline();							//各バッファをパイプラインにセット
};
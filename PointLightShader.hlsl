//───────────────────────────────────────
 // テクスチャ＆サンプラーデータのグローバル変数定義
//───────────────────────────────────────
Texture2D g_texture : register(t0); //テクスチャー
SamplerState g_sampler : register(s0); //サンプラー

//───────────────────────────────────────
// コンスタントバッファ
// DirectX 側から送信されてくる、ポリゴン頂点以外の諸情報の定義
//───────────────────────────────────────
cbuffer global
{
    float4x4 matW; // ワールド行列
    float4x4 matWVP; // ワールド・ビュー・プロジェクションの合成行列
    float4x4 matNormal; // ワールド行列
    float4 diffuseColor; //マテリアルの色＝拡散反射係数tt
    float4 lightPosition;
    float2 factor;
    bool isTextured; //テクスチャーが貼られているかどうか
};

//───────────────────────────────────────
// 頂点シェーダー出力＆ピクセルシェーダー入力データ構造体
//───────────────────────────────────────
struct VS_OUT
{
    float4 wpos : POSITION; //位置
    float4 pos : SV_POSITION; //位置
    float2 uv : TEXCOORD; //UV座標
    float4 color : COLOR; //色（明るさ）
    float4 wnormal : NORMAL;
};

//───────────────────────────────────────
// 頂点シェーダ
//───────────────────────────────────────
VS_OUT VS(float4 pos : POSITION, float4 uv : TEXCOORD, float4 normal : NORMAL)
{
	//ピクセルシェーダーへ渡す情報
    VS_OUT outData;

	//ローカル座標に、ワールド・ビュー・プロジェクション行列をかけて
	//スクリーン座標に変換し、ピクセルシェーダーへ
    
    outData.wpos = mul(pos, matW);
    outData.pos = mul(pos, matWVP);
    outData.uv = uv;
    
    outData.wnormal = mul(normal, matNormal);
    //normal.w = 0;
    //normal = normalize(normal);
    
	//float4 light = float4(0, 1, -1, 0);
   // float4 lightvec = lightPosition - outData.pos;
   // float4 light = normalize(lightvec);
   
    //outData.color = clamp(dot(normal, light), 0, 1);

	//まとめて出力
    return outData;
}

//───────────────────────────────────────
// ピクセルシェーダ
//───────────────────────────────────────
float4 PS(VS_OUT inData) : SV_Target
{
    //float4 lightSource = float4(1.0, 1.0, 1.0, 1.0);
    float3 ambentSource = float3(0.2, 0.2, 0.2);
    float3 dir = normalize(lightPosition.xyz - inData.wpos.xyz);
    inData.wnormal.z = 0;
    float3 col = saturate(dot(normalize(inData.wnormal).xyz, dir));
    //減衰
    float len = length(lightPosition.xyz - inData.wpos.xyz);
    float colA = saturate(1.0f / (1.0 + 1.0 * len + 0.5 * len * len));
    
    float3 diffuse;
    float3 ambient;
    if (isTextured == false)
    {
        diffuse = diffuseColor.xyz * col * factor.x;
        ambient = diffuseColor.xyz * ambentSource * factor.x;
    }
    else
    {
        diffuse = g_texture.Sample(g_sampler, inData.uv).xyz * col*colA;
        ambient = g_texture.Sample(g_sampler, inData.uv).xyz * ambentSource * factor.x;

    }
	//return g_texture.Sample(g_sampler, inData.uv);// (diffuse + ambient);]
	//float4 diffuse = lightSource * inData.color;
	//float4 ambient = lightSource * ambentSource;
    return float4(diffuse+ambient, 1.0f);
}

//───────────────────────────────────────
 // テクスチャ＆サンプラーデータのグローバル変数定義
//───────────────────────────────────────
Texture2D g_texture : register(t0); //テクスチャー
SamplerState g_sampler : register(s0); //サンプラー
Texture2D g_nTexture : register(t1); //ノーマルマップテクスチャ
SamplerState g_nTsampler : register(s1); //サンプラー
//───────────────────────────────────────
// コンスタントバッファ
// DirectX 側から送信されてくる、ポリゴン頂点以外の諸情報の定義
//───────────────────────────────────────
cbuffer gModel : register(b0)
{
    float4x4 matWVP; // ワールド・ビュー・プロジェクションの合成行列
    float4x4 matW; //ワールド変換マトリクス
    float4x4 matNormal; // ワールド行列
    float4 diffuseColor; //マテリアルの色＝拡散反射係数tt
    float4 factor;
    float4 ambientColor;
    float4 specularColor;
    float4 shininess;
    int4 isTextured; //テクスチャーが貼られているかどうか
    int4 isNormalMapped; //法線マップが貼られているかどうか
};

cbuffer gStage : register(b1)
{
    float4 lightPosition[5];
    float4 eyePosition; //ワールド座標での視点
    float4 pLightposition;
    float4 pointLightColor[5];
    float4 spotLightColor;
    float4 direction;
    float4 kTerm[5];
    float4 sptParam;
    int4 pointListSW[5];
};



//───────────────────────────────────────
// 頂点シェーダー出力＆ピクセルシェーダー入力データ構造体
//───────────────────────────────────────
struct VS_OUT
{
    float4 pos : SV_POSITION; //位置
    float2 uv : TEXCOORD; //UV座標
    float4 eyev : POSITION; //ワールド座標に変換された視線ベクトル
    float4 Neyev : POSITION1; //ノーマルマップ用の接空間に変換された視線ベクトル
    float4 normal : NORMAL; //法線ベクトル
    float4 light : POSITION2; //ライトを接空間に変換したベクトル
    float4 color : COLOR; //ランバートの拡散反射計算用
};

//───────────────────────────────────────
// 頂点シェーダ
//───────────────────────────────────────
VS_OUT VS(float4 pos : POSITION, float4 uv : TEXCOORD, float4 normal : NORMAL, float4 tangent: TANGENT)
{
	//ピクセルシェーダーへ渡す情報
    VS_OUT outData;

	//ローカル座標に、ワールド・ビュー・プロジェクション行列をかけて
	//スクリーン座標に変換し、ピクセルシェーダーへ
    outData.pos = mul(pos, matWVP);
    outData.uv = uv.xy;
    //接戦、法線、従法線を計算して、ローカル座標に変換
    float3 tmp = cross(tangent.xyz, normal.xyz);
    //tmp.w = 0;
    float4 binormal = mul(tmp, matNormal);
    binormal = normalize(binormal); //従法線をローカル座標に変換したやつ
    normal = mul(normal, matNormal); //法線をローカル座標に変換したやつ
    normal.w = 0;
    outData.normal = normalize(normal);
    
    tangent = mul(tangent, matNormal);
    tangent.w = 0;
    tangent = normalize(tangent);
    
    //視線ベクトル（ワールド座標）
    float4 posw = mul(pos, matW);
    outData.eyev = normalize(posw - eyePosition); //ワールド座標の視線ベクトル
    
    //視線ベクトルを接空間に変換
    outData.Neyev.x = dot(outData.eyev, tangent );
    outData.Neyev.y = dot(outData.eyev, binormal );
    outData.Neyev.z = dot(outData.eyev, normal );
    outData.Neyev.w = 0;
    
	//float4 light = float4(0, 1, -1, 0);
    //float4 light = lightPosition[0];
    float4 light = pLightposition;
    light.w = 0;
    light = normalize(light);
    //ライトを接空間に変換
    outData.light.x = mul(light, tangent);
    outData.light.y = mul(light, binormal);
    outData.light.z = mul(light, normal);
    outData.light.w = 0;
    
    outData.color = clamp(dot(outData.normal, light), 0, 1);

	//まとめて出力
    return outData;
}

//───────────────────────────────────────
// ピクセルシェーダ
//───────────────────────────────────────
float4 PS(VS_OUT inData) : SV_Target
{
    float4 lightSource = float4(1.0, 1.0, 1.0, 1.0);
    float4 ambentSource = float4(0.5, 0.5, 0.5, 1.0);
    float4 diffuse;
    float4 ambient;
    if (isNormalMapped.x == 1)
    {   //ノーマルマップ画像の読み込み
        float4 nmap = g_nTexture.Sample(g_nTsampler, inData.uv) * 2.0f - 1.0f;
        nmap = normalize(nmap);
        nmap.w = 0;
        //ランバートのやつ
        float4 NL = clamp(dot(normalize(inData.light), nmap), 0, 1);
        //鏡面反射の計算
        float4 reflection = reflect(normalize(inData.light), nmap);
        float4 specular = pow(clamp(dot(reflection, inData.Neyev), 0, 1), shininess);
        
        if (isTextured.x == 0)
        {
            diffuse = diffuseColor * NL * factor.x;
            ambient = diffuseColor * ambentSource * factor.x;
        }
        else
        {
            //diffuse = g_texture.Sample(g_sampler, inData.uv) * NL * factor.x;
            //ambient = g_texture.Sample(g_sampler, inData.uv) * ambentSource;
            diffuse = NL + specular;
            ambient = float4(0.3, 0.3, 0.3, 1.0);
        }
        return diffuse + specular + ambient;

    }
    else
    {
        if (isTextured.x == 0)
        {
            diffuse = diffuseColor * inData.color * factor.x;
            ambient = diffuseColor * ambentSource * factor.x;
        }
        else
        {
            diffuse = g_texture.Sample(g_sampler, inData.uv) * inData.color * factor.x;
            ambient = g_texture.Sample(g_sampler, inData.uv) * ambentSource * factor.x;

        }
	//return g_texture.Sample(g_sampler, inData.uv);// (diffuse + ambient);]
	//float4 diffuse = lightSource * inData.color;
	//float4 ambient = lightSource * ambentSource;
        return diffuse + ambient;
    }
}

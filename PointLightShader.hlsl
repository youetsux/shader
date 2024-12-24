//───────────────────────────────────────
 // テクスチャ＆サンプラーデータのグローバル変数定義
//───────────────────────────────────────
Texture2D g_texture : register(t0); //テクスチャー
SamplerState g_sampler : register(s0); //サンプラー

//───────────────────────────────────────
// コンスタントバッファ
// DirectX 側から送信されてくる、ポリゴン頂点以外の諸情報の定義
//───────────────────────────────────────
cbuffer gModel:register(b0)
{
    float4x4 matWVP; // ワールド・ビュー・プロジェクションの合成行列
    float4x4 matW; //ワールド変換マトリクス
    float4x4 matNormal; // ワールド行列
    float4 diffuseColor; //マテリアルの色＝拡散反射係数tt
    float4 factor;
    float4 ambientColor;
    float4 specularColor;
    float4 shininess;

    bool isTextured; //テクスチャーが貼られているかどうか
};

cbuffer gStage:register(b1)
{
    float4 lightPosition;
    float4 eyePosition;
    float4 pLightposition;
    float4 color;
    float4 direction;
    float theta;
    float phi;
    float att;
    float toff;
};

//───────────────────────────────────────
// 頂点シェーダー出力＆ピクセルシェーダー入力データ構造体
//───────────────────────────────────────
struct VS_OUT
{
    float4 wpos : POSITION0; //位置
    float4 pos : SV_POSITION; //位置
    float2 uv : TEXCOORD; //UV座標
    float4 normal : NORMAL;
    float4 eyev : POSITION1;
    //float4 col : COLOR;
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
    float4 spos = mul(pos, matWVP);
    float4 wpos = mul(pos, matW);//ワールド座標に変換
    float4 wnormal = mul(normal, matNormal);
    
    outData.pos = spos;
    outData.wpos = wpos;
    outData.uv = uv.xy;
    outData.normal = wnormal;
    outData.eyev = eyePosition - wpos;
    
    //float4 dir = normalize(lightPosition - wpos);
    //outData.col = clamp(dot(normalize(wnormal), dir), 0, 1);
    
	//まとめて出力
    return outData;
}

//───────────────────────────────────────
// ピクセルシェーダ
//───────────────────────────────────────
float4 PS(VS_OUT inData) : SV_Target
{
    //float4 diffuse;
    //float4 ambient;
    float4 ambientSource = { 0.1, 0.1, 0.1, 1.0 };
    //float3 dir = normalize(lightPosition.xyz - inData.wpos.xyz); //ピクセル位置のポリゴンの3次元座標＝wpos
    ////inData.normal.z = 0;
    //float color = saturate(dot(normalize(inData.normal.xyz), dir));
    //float3 k = { 0.2f, 0.2f, 1.0f };
    //float len = length(lightPosition.xyz - inData.wpos.xyz);
    //float dTerm = 1.0 / (k.x + k.y*len + k.z*len*len);
    
    //float4 R = reflect(normalize(inData.normal), normalize(float4(dir, 1.0)));
    //float4 specular = pow(saturate(dot(R, normalize(inData.eyev))), shininess) * specularColor;
    
    //if (isTextured == false)
    //{
    //    diffuse =  diffuseColor * color * dTerm * factor.x;
    //    ////diffuse = float4(1.0, 1.0, 1.0, 1.0);
    //    ambient =  diffuseColor * ambentSource;
    //}
    //else
    //{
    //    diffuse =   g_texture.Sample(g_sampler, inData.uv) * color * dTerm*factor.x;
    //    ambient = g_texture.Sample(g_sampler, inData.uv) * ambentSource;

    //}

    //return diffuse +  specular + ambient;
    //return specular + ambient;
    
    float3 spLightDir = normalize(pLightposition.xyz - inData.wpos.xyz);
    float len = length(pLightposition.xyz - inData.wpos.xyz);
    float attenuation = 1.0 / (att * len * len);

    float3 spLightDirN = normalize(spLightDir);
    float3 spor_dirN = normalize(direction.xyz);
    float cos_alpha = dot(-spLightDir, spor_dirN);
    float cos_half_theta = cos(radians(theta / 2.0));
    float cos_half_phi = cos(radians(phi / 2.0));
    //diffuseの計算
    float4 diffuse;
    float specular;
    
    if (cos_alpha <= cos_half_phi)
    {
        diffuse = float4(0, 0, 0, 1);
        specular = float4(0, 0, 0, 1);
    }
    else
    {
        if (cos_alpha > cos_half_theta)
        {
            // inner corn
            // attenuation * 1.f
            attenuation = 1.0;
        }
        else
        {
            // outer corn
            attenuation = pow((cos_alpha - cos_half_phi) / (cos_half_theta - cos_half_phi), toff);
        }
        inData.normal.w = 0;
        diffuse = clamp(dot(spLightDirN, normalize(inData.normal.xyz)), 0.0, 1.0);
        
        //specularの計算

        float4 R = reflect(normalize(inData.normal), normalize(float4(spLightDirN, 1.0)));
        float specularPower = pow(clamp(dot(R, normalize(inData.eyev)), 0.0, 1.0), shininess);
        specular = specularColor * specularPower;
    }
    
    if(isTextured == false)
    {
        diffuse = color * diffuse * diffuseColor;
    }
    else
    {
        diffuse = color * diffuse * g_texture.Sample(g_sampler, inData.uv);
    }
    

    
    
    //ambientの計算
    float4 ambient;
    if(isTextured == false)
    {
        ambient = diffuseColor * ambientSource;
    }
    else
    {
        ambient = g_texture.Sample(g_sampler, inData.uv) * ambientSource;
    }
    

    return diffuse*attenuation + specular + ambient;
 
    
}

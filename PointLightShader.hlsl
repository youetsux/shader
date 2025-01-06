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
    float4 lightPosition[5];
    float4 eyePosition;
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
    float4 pt_diffuse = { 0, 0, 0, 1.0f };
    float4 pt_ambient = { 0, 0, 0, 1.0f };
    float4 pt_specular = { 0, 0, 0, 1.0f };
    float4 ambientSource = { 0.1, 0.1, 0.1, 1.0 };
    for (int i = 0; i < 5; i++)
    {
        if (pointListSW[i].x == 1)
        {
            float3 dir = normalize(lightPosition[i].xyz - inData.wpos.xyz); //ピクセル位置のポリゴンの3次元座標＝wpos
            inData.normal.w = 0;
            float ptPower = saturate(dot(normalize(inData.normal.xyz), dir));
            //float3 k = { 0.2f, 0.2f, 1.0f };
            float len = length(lightPosition[i].xyz - inData.wpos.xyz);
            float dTerm = 1.0 / (kTerm[i].x + kTerm[i].y * len + kTerm[i].z * len * len);
    
            float4 R = reflect(normalize(inData.normal), normalize(float4(dir, 1.0)));
            pt_specular += pow(saturate(dot(R, normalize(inData.eyev))), shininess) * specularColor;
    
            if (isTextured == false)
            {
                pt_diffuse += diffuseColor * pointLightColor[i] * dTerm * factor.x;
        ////diffuse = float4(1.0, 1.0, 1.0, 1.0);
                pt_ambient += diffuseColor * ambientSource;
            }
            else
            {
                pt_diffuse += g_texture.Sample(g_sampler, inData.uv) * pointLightColor[i] * dTerm * factor.x;
                pt_ambient += g_texture.Sample(g_sampler, inData.uv) * ambientSource;
            }

    //return diffuse +  specular + ambient;
    //return specular + ambient;
        }

    }

    float theta = sptParam.x;
    float phi = sptParam.y;
    float att = sptParam.z;
    float toff = sptParam.w;
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
    float4 specular;
    
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
        float4 specularPower = pow(clamp(dot(R, normalize(inData.eyev)), 0.0, 1.0), shininess);
        specular = specularColor * specularPower;
    }
    
    if(isTextured == false)
    {
        diffuse = spotLightColor * diffuse * diffuseColor * factor.x;
    }
    else
    {
        diffuse = spotLightColor * diffuse * g_texture.Sample(g_sampler, inData.uv) * factor.x;
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
    

    //return clamp(diffuse * attenuation + pt_diffuse + pt_specular + specular + ambient, 0.0f, 1.0f);
    return pt_diffuse;
    //return g_texture.Sample(g_sampler, inData.uv);
    //if (pointListSW[1].x == 1)
    //    return float4(1, 0, 0, 1.0);
    //else
    //    return float4(0, 0, 0, 1.0);
   // return diffuse * attenuation + specular + ambient;
    //return pLightposition[1];

}

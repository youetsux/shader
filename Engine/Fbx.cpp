#include <assert.h>
#include "Fbx.h"
#include "Direct3D.h"
#include "Camera.h"
#include "Texture.h"
#include <vector>
#include <filesystem>

Fbx::Fbx()
	:vertexCount_(0), polygonCount_(0), materialCount_(0),
	pVertexBuffer_(nullptr), pIndexBuffer_(nullptr), pConstantBuffer_(nullptr)
{
}

HRESULT Fbx::Load(std::string fileName)
{
	//マネージャを生成
	FbxManager* pFbxManager = FbxManager::Create();

	//インポーターを生成
	FbxImporter* fbxImporter = FbxImporter::Create(pFbxManager, "imp");
	fbxImporter->Initialize(fileName.c_str(), -1, pFbxManager->GetIOSettings());

	//シーンオブジェクトにFBXファイルの情報を流し込む
	FbxScene* pFbxScene = FbxScene::Create(pFbxManager, "fbxscene");
	fbxImporter->Import(pFbxScene);
	fbxImporter->Destroy();

	//メッシュ情報を取得
	FbxNode* rootNode = pFbxScene->GetRootNode();
	FbxNode* pNode = rootNode->GetChild(0);
	FbxMesh* mesh = pNode->GetMesh();

	//各情報の個数を取得
	vertexCount_ = mesh->GetControlPointsCount();	//頂点の数
	polygonCount_ = mesh->GetPolygonCount();	//ポリゴンの数
	materialCount_ = pNode->GetMaterialCount();

	//現在のカレントディレクトリを取得
	char defaultCurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, defaultCurrentDir);

	//引数のfileNameからディレクトリ部分を取得
	char dir[MAX_PATH];
	_splitpath_s(fileName.c_str(), nullptr, 0, dir, MAX_PATH, nullptr, 0, nullptr, 0);

	//カレントディレクトリ変更
	SetCurrentDirectory(dir);

	InitVertex(mesh);		//頂点バッファ準備
	InitIndex(mesh);		//インデックスバッファ準備
	IntConstantBuffer();	//コンスタントバッファ準備
	InitMaterial(pNode);

	//カレントディレクトリを元に戻す
	SetCurrentDirectory(defaultCurrentDir);


	//マネージャ解放
	pFbxManager->Destroy();
	return S_OK;
}

//頂点バッファ準備
void Fbx::InitVertex(fbxsdk::FbxMesh* mesh)
{
	//頂点情報を入れる配列
	VERTEX* vertices = new VERTEX[vertexCount_];

	//全ポリゴン
	for (DWORD poly = 0; poly < polygonCount_; poly++)
	{
		//3頂点分
		for (int vertex = 0; vertex < 3; vertex++)
		{
			//調べる頂点の番号
			int index = mesh->GetPolygonVertex(poly, vertex);

			//頂点の位置
			FbxVector4 pos = mesh->GetControlPointAt(index);
			vertices[index].position = XMVectorSet((float)pos[0], (float)pos[1], -(float)pos[2], 0.0f);

			//頂点のUV
			FbxLayerElementUV* pUV = mesh->GetLayer(0)->GetUVs();
			int uvIndex = mesh->GetTextureUVIndex(poly, vertex, FbxLayerElement::eTextureDiffuse);
			FbxVector2  uv = pUV->GetDirectArray().GetAt(uvIndex);
			vertices[index].uv = XMVectorSet((float)uv.mData[0], (float)(1.0 - uv.mData[1]), 0.0f, 0.0f);


			FbxLayerElementNormal* leNormal = mesh->GetLayer(0)->GetNormals();
			FbxLayerElement::EMappingMode mp = leNormal->GetMappingMode();
			//頂点の法線
			//FbxVector4 Normal;
			//mesh->GetPolygonVertexNormal(poly, vertex, Normal);	//ｉ番目のポリゴンの、ｊ番目の頂点の法線をゲット

			FbxVector4 Normal = leNormal->GetDirectArray().GetAt(index);
			
			vertices[index].normal = XMVectorSet((float)Normal[0], (float)Normal[1], -(float)Normal[2], 0.0f);
		}
	}

	//頂点バッファ
	HRESULT hr;
	D3D11_BUFFER_DESC bd_vertex;
	bd_vertex.ByteWidth = sizeof(VERTEX) * vertexCount_;
	bd_vertex.Usage = D3D11_USAGE_DEFAULT;
	bd_vertex.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd_vertex.CPUAccessFlags = 0;
	bd_vertex.MiscFlags = 0;
	bd_vertex.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA data_vertex;
	data_vertex.pSysMem = vertices;
	hr = Direct3D::pDevice_->CreateBuffer(&bd_vertex, &data_vertex, &pVertexBuffer_);
	if (FAILED(hr))
	{
		MessageBox(NULL, "頂点バッファの作成に失敗しました", "エラー", MB_OK);
	}
}



//インデックスバッファ準備
void Fbx::InitIndex(fbxsdk::FbxMesh* mesh)
{
	pIndexBuffer_ = new ID3D11Buffer * [materialCount_];
	indexCount_ = vector<int>(materialCount_);
	//indexCount_  = new int[materialCount_]


	vector<int> index(polygonCount_ * 3);//ポリゴン数*3＝全頂点分用意
	//int* index = new int[polygonCount_ * 3];

	for (int i = 0; i < materialCount_; i++)
	{
		int count = 0;
		//全ポリゴン
		for (DWORD poly = 0; poly < polygonCount_; poly++)
		{
			//あるマテリアルを持ったポリゴンのリストをとってきて、頂点をリストアップ
			FbxLayerElementMaterial* mtl = mesh->GetLayer(0)->GetMaterials();
			int mtlId = mtl->GetIndexArray().GetAt(poly);

			if (mtlId == i)
			{
				//3頂点分
				for (DWORD vertex = 0; vertex < 3; vertex++)
				{
					index[count] = mesh->GetPolygonVertex(poly, 2 - vertex);
					count++;
				}
			}	
		}
		indexCount_[i] = count;

		D3D11_BUFFER_DESC   bd;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int) * polygonCount_ * 3;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = index.data();
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		HRESULT hr;
		hr = Direct3D::pDevice_->CreateBuffer(&bd, &InitData, &pIndexBuffer_[i]);
		if (FAILED(hr))
		{
			MessageBox(NULL, "インデックスバッファの作成に失敗しました", "エラー", MB_OK);
		}
	}
}

void Fbx::IntConstantBuffer()
{
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(CONSTBUFFER_MODEL);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;

	// コンスタントバッファの作成
	HRESULT hr;
	hr = Direct3D::pDevice_->CreateBuffer(&cb, nullptr, &pConstantBuffer_);
	if (FAILED(hr))
	{
		MessageBox(NULL, "コンスタントバッファの作成に失敗しました", "エラー", MB_OK);
	}
}
namespace fs = std::filesystem;
void Fbx::InitMaterial(fbxsdk::FbxNode* pNode)
{
	pMaterialList_ = std::vector<MATERIAL>(materialCount_);

	for (int i = 0; i < materialCount_; i++)
	{
		//i番目のマテリアル情報を取得
		FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);

		//テクスチャ情報
		FbxProperty  lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);

		//テクスチャの数数
		int fileTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();

		//テクスチャあり
		if (fileTextureCount)
		{
			FbxFileTexture* textureInfo = lProperty.GetSrcObject<FbxFileTexture>(0);
			const char* textureFilePath = textureInfo->GetRelativeFileName();

			//ファイル名+拡張だけにする
			//char name[_MAX_FNAME];	//ファイル名
			//char ext[_MAX_EXT];	//拡張子
			//_splitpath_s(textureFilePath, nullptr, 0, nullptr, 0, name, _MAX_FNAME, ext, _MAX_EXT);
			//wsprintf(name, "%s%s", name, ext);
			fs::path texFile(textureFilePath);
			//FbxFileTexture* textureInfo = lProperty.GetSrcObject<FbxFileTexture>(0);
			//const char* textureFilePath = textureInfo->GetRelativeFileName();

			//ファイルからテクスチャ作成
			if (fs::is_regular_file(texFile))
			{
				pMaterialList_[i].pTexture = new Texture;
				HRESULT hr = pMaterialList_[i].pTexture->Load(texFile.string());
				assert(hr == S_OK);
			}
			FbxSurfacePhong* pMaterial = (FbxSurfacePhong*)pNode->GetMaterial(i);
			FbxDouble  diffuse = pMaterial->DiffuseFactor;
			FbxDouble3  ambient = pMaterial->Ambient;
			pMaterialList_[i].factor = XMFLOAT4((float)diffuse, (float)diffuse, (float)diffuse, (float)diffuse);
			pMaterialList_[i].ambient = { (float)ambient[0], (float)ambient[1], (float)ambient[2], 1.0f };
			//あなたはフォンのパラメータを持ってますか？
			if (pMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				FbxDouble3 specular = pMaterial->Specular;
				FbxDouble shininess = pMaterial->Shininess; //4つとも同じ値でセット
				//ここで、自分のpMaterialList_[i]に値を設定
				pMaterialList_[i].specular = { (float)specular[0],(float)specular[1], (float)specular[2], 1.0f };
				pMaterialList_[i].shininess = { (float)shininess,(float)shininess, (float)shininess, 1.0 };
			}
			


		}

		//テクスチャ無し
		else
		{
			pMaterialList_[i].pTexture = nullptr;
			//マテリアルの色
			FbxSurfacePhong* pMaterial = (FbxSurfacePhong*)pNode->GetMaterial(i);
			FbxDouble3  diffuse = pMaterial->Diffuse;
			pMaterialList_[i].diffuse = XMFLOAT4((float)diffuse[0], (float)diffuse[1], (float)diffuse[2], 1.0f);
			//FbxSurfaceLambert* pMaterial = (FbxSurfaceLambert*)pNode->GetMaterial(i);
			FbxDouble  factor = pMaterial->DiffuseFactor;
			pMaterialList_[i].factor = XMFLOAT4((float)factor, (float)factor, (float)factor, (float)factor);
			FbxDouble3  ambient = pMaterial->Ambient;
			pMaterialList_[i].ambient = { (float)ambient[0], (float)ambient[1], (float)ambient[2], 1.0f };
			if (pMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				FbxDouble3 specular = pMaterial->Specular;
				FbxDouble shininess = pMaterial->Shininess; //4つとも同じ値でセット
				//ここで、自分のpMaterialList_[i]に値を設定
				pMaterialList_[i].specular = { (float)specular[0],(float)specular[1], (float)specular[2], 1.0f };
				pMaterialList_[i].shininess = { (float)shininess,(float)shininess, (float)shininess, 1.0 };
			}
			else
			{
				pMaterialList_[i].specular = {0.0f,0.0f,0.0f,1.0f };
				pMaterialList_[i].shininess = {10.0f,10.0f,10.0f, 1.0 };
			}

		}
	}
}


void Fbx::Draw(Transform& transform)
{
	Direct3D::SetShader(SHADER_POINT);
	transform.Calclation();//トランスフォームを計算
	

	for (int i = 0; i < materialCount_; i++)
	{
		//コンスタントバッファに情報を渡す
		CONSTBUFFER_MODEL cb;
		cb.matW = XMMatrixTranspose(transform.GetWorldMatrix());
		cb.matWVP = XMMatrixTranspose(transform.GetWorldMatrix() * Camera::GetViewMatrix() * Camera::GetProjectionMatrix());
		cb.matW = XMMatrixTranspose(transform.GetWorldMatrix());
		cb.matNormal = XMMatrixTranspose(transform.GetNormalMatrix());
		cb.ambientColor = pMaterialList_[i].ambient;
		cb.specularColor = pMaterialList_[i].specular;
		cb.shininess = pMaterialList_[i].shininess;
		cb.diffuseColor = pMaterialList_[i].diffuse;
		cb.diffuseFactor = pMaterialList_[i].factor;
		cb.isTextured = pMaterialList_[i].pTexture != nullptr;
	
		D3D11_MAPPED_SUBRESOURCE pdata;
		Direct3D::pContext_->Map(pConstantBuffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);	// GPUからのデータアクセスを止める
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));	// データを値を送る

		Direct3D::pContext_->Unmap(pConstantBuffer_, 0);	//再開

		//頂点バッファ、インデックスバッファ、コンスタントバッファをパイプラインにセット
		//頂点バッファ
		UINT stride = sizeof(VERTEX);
		UINT offset = 0;
		Direct3D::pContext_->IASetVertexBuffers(0, 1, &pVertexBuffer_, &stride, &offset);


		// インデックスバッファーをセット
		stride = sizeof(int);
		offset = 0;
		Direct3D::pContext_->IASetIndexBuffer(pIndexBuffer_[i], DXGI_FORMAT_R32_UINT, 0);

		//コンスタントバッファ
		Direct3D::pContext_->VSSetConstantBuffers(0, 1, &pConstantBuffer_);	//頂点シェーダー用	
		Direct3D::pContext_->PSSetConstantBuffers(0, 1, &pConstantBuffer_);	//ピクセルシェーダー用


		if (pMaterialList_[i].pTexture)
		{
			ID3D11SamplerState* pSampler = pMaterialList_[i].pTexture->GetSampler();
			Direct3D::pContext_->PSSetSamplers(0, 1, &pSampler);

			ID3D11ShaderResourceView* pSRV = pMaterialList_[i].pTexture->GetSRV();
			Direct3D::pContext_->PSSetShaderResources(0, 1, &pSRV);
		}

		//描画
		Direct3D::pContext_->DrawIndexed(indexCount_[i], 0, 0);
	}
}

void Fbx::Release()
{
}

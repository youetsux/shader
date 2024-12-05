#pragma once

#include <d3d11.h>
#include <fbxsdk.h>
#include <string>
#include "Transform.h"
#include <vector>


#pragma comment(lib, "LibFbxSDK-MD.lib")
#pragma comment(lib, "LibXml2-MD.lib")
#pragma comment(lib, "zlib-MD.lib")

using std::vector;

class Texture;

class Fbx
{
	//�}�e���A��
	struct MATERIAL
	{
		Texture* pTexture;
		XMFLOAT4 diffuse;
		XMFLOAT4 factor;
	};

	struct CONSTANT_BUFFER
	{
		XMMATRIX	matWVP;//�X�N���[���ϊ��}�g���N�X
		XMMATRIX	matW; //���[���h�ϊ��}�g���N�X
		XMMATRIX	matNormal;//�@�����[���h�ϊ��p�}�g���N�X
		XMFLOAT4	diffuseColor;//RGB�̊g�U���ˌW���i�F�j
		XMFLOAT4    lightPosition;//�����ʒu
		XMFLOAT4	diffuseFactor;//�g�U���̔��ˌW��
		int			isTextured;
	};

	struct VERTEX
	{
		XMVECTOR position;//�ʒu
		XMVECTOR uv; //�e�N�X�`�����W
		XMVECTOR normal; //�@���x�N�g��
	};

	int vertexCount_;	//���_��
	int polygonCount_;	//�|���S����
	int materialCount_;	//�}�e���A���̌�

	ID3D11Buffer* pVertexBuffer_;
	ID3D11Buffer** pIndexBuffer_;
	ID3D11Buffer* pConstantBuffer_;
	std::vector<MATERIAL> pMaterialList_;
	vector <int> indexCount_;
	
	void InitVertex(fbxsdk::FbxMesh* mesh);
	void InitIndex(fbxsdk::FbxMesh* mesh);
	void IntConstantBuffer();
	void InitMaterial(fbxsdk::FbxNode* pNode);
public:

	Fbx();
	HRESULT Load(std::string fileName);
	void    Draw(Transform& transform);
	void    Release();
};
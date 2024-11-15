#pragma once
#include "GameObject.h"

//ƒQ[ƒ€‚ÌƒV[ƒ“‚ğ—ñ‹“
enum SCENE_ID
{
	SCENE_ID_TEST = 0,
};


class SceneManager :
    public GameObject
{
	SCENE_ID currentSceneID_;
	SCENE_ID nextSceneID_;
public:
	SceneManager(GameObject* parent);
	~SceneManager();
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Release() override;
	void ChangeScene(SCENE_ID _next);
};


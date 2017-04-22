#ifndef DYENGINE_SCENE_MANAGER_H
#define DYENGINE_SCENE_MANAGER_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <memory>
#include <vector>
#include <cmath>
#include <iostream>
#include <functional>
#include "ObjectManager.h"
#include "Object.h"
#include "Shape.h"

using namespace std;
using namespace cv;

class ObjectManager;
class Component;
class Object;
class DynamicObject;

typedef std::function<void(ObjectManager*)> LoadFunction;

class SceneManager;

// Scene is used to connect Object And RenderWindow
class Scene
{
public:
	//! 0 is black
	float FadeColorValue = 255.0f;

	int Identity;
	unique_ptr<Mat> TargetFrame = nullptr;
	unique_ptr<ObjectManager> AttachedObjectManager;

	SceneManager* ScnManager;

	Scene(int _id);

	Mat* GetTargetFrame();
	Mat GetWindowFrame();
	ObjectManager* GetAttachedObjectManager();

	//! Instantiate Obj in the scene with LoadingFunc
	void Load();

	//! Delete Obj in the scene
	void Release();

	void SetLoadingFunction(LoadFunction _func) 
	{ 
		m_loadingFunc = _func; 
	}

	cv::Rect GetWindowRect(float offset)
	{
		Rect rect(-offset, -offset, GetTargetFrame()->size().width + offset * 2, GetTargetFrame()->size().height + offset * 2);
	}

private:
	LoadFunction m_loadingFunc;
};

class SceneManager
{
public:
	static bool ShowDebugeWindow;

	Scene* CurrentScene = nullptr;
	vector<unique_ptr<Scene>> ScenesList;

	Scene* GetCurrentScene();

	Scene* GetScene(int index);

	Scene* CreateNewScene(LoadFunction _loadingfunction);

	void LoadScene(int _id);

	void LoadScene(int _id, float _delay);

	//! May cause violation
	void InstantLoadScene(int _id);

	void Start();

	void Update(float timeStep);

	void LateUpdate(float timeStep);

	bool m_LoadSceneRequest = false;
private:
	int m_RequestScene;
	float m_DelayTimer = 0.0f;

};


#endif
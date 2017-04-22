#include "SceneManager.h"

using namespace std;
using namespace cv;

Scene::Scene(int _id) : Identity(_id)
{
	TargetFrame = make_unique<Mat>();
	AttachedObjectManager = make_unique<ObjectManager>();
	AttachedObjectManager->m_Scene = this;
}

Mat* Scene::GetTargetFrame()
{
	return TargetFrame.get();
}

Mat Scene::GetWindowFrame()
{
	Mat frame = TargetFrame.get()->clone();
	for (int i = 0; i < frame.size().width; i++)
	{
		for (int j = 0; j < frame.size().height; j++)
		{
			for (int c = 0; c < 3; c++)
			{
				frame.at<Vec3b>(j, i)[c] *= FadeColorValue / 255.0f;
			}
		}
	}
	return frame;
}

ObjectManager* Scene::GetAttachedObjectManager()
{
	return AttachedObjectManager.get();
}

void Scene::Load() 
{ 
	m_loadingFunc(AttachedObjectManager.get());
}

void Scene::Release()
{
	AttachedObjectManager->ReleaseObjects();
}
//////////////////////////////////////////////////////////////////////
bool SceneManager::ShowDebugeWindow = false;

Scene* SceneManager::GetCurrentScene()
{
	return CurrentScene;
}

Scene* SceneManager::GetScene(int index)
{
	return ScenesList.at(index).get();
}

Scene* SceneManager::CreateNewScene(LoadFunction _loadingfunction)
{
	ScenesList.push_back(make_unique<Scene>((int) ScenesList.size()));
	Scene* scene = ScenesList.at(ScenesList.size() - 1).get();
	scene->ScnManager = this;
	scene->SetLoadingFunction(_loadingfunction);
	return scene;
}

void SceneManager::LoadScene(int _id)
{
	m_RequestScene = _id;
	m_LoadSceneRequest = true;
}

void SceneManager::LoadScene(int _id, float _delay)
{
	m_DelayTimer = _delay;
}

void SceneManager::InstantLoadScene(int _id)
{
	m_LoadSceneRequest = false;

	std::cout << "Load Scene: " << _id << std::endl;
	std::cout << "Try Leave Scene" << std::endl;
	// release scene first if current scene is not empty
	if (CurrentScene != nullptr)
	{
		CurrentScene->Release();
	}
	std::cout << "Releaseing Scene..." << std::endl;
	// load scene
	CurrentScene = ScenesList.at(_id).get();
	CurrentScene->Load();
}

void SceneManager::Start()
{
	CurrentScene->GetAttachedObjectManager()->Start();
}

void SceneManager::Update(float timeStep)
{
	if (m_LoadSceneRequest)
		InstantLoadScene(m_RequestScene);
	else
		CurrentScene->GetAttachedObjectManager()->Update(timeStep);
}

void SceneManager::LateUpdate(float timeStep)
{
	CurrentScene->GetAttachedObjectManager()->LateUpdate(timeStep);
}
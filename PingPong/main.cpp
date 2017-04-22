#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <thread>
#include <atomic>
#include <functional>


#include "SceneManager.h"
#include "Object.h"
#include "Component.h"

#include "XmlParser\rapidxml.hpp"

#define WIDTH 640
#define HEIGHT 480

//UPPER: 90 220 255
//LOWER: 10 120 200

using namespace std;
using namespace cv;

string IMG_DIR = "./Images/";
string TILE_DIR = "./Images/TileSets/";
string MAP_DIR = "./Maps/";

typedef rapidxml::xml_node<> XmlNode;

//! store the information in the Tile XML
std::map<char, TileInform> tileMap;
//! store the information in the Item XML
std::map<string, ItemInform> itemList;

int seed = 100;

float mapOffset;
VECTOR::vector2f mapOrigin;

cv::Scalar upperColor;
cv::Scalar lowerColor;
float upperRadius;
float lowerRadius;

std::vector<VECTOR::vector2f> rightPos;
std::vector<VECTOR::vector2f> leftPos;

cv::Scalar colorOffset(20, 20, 40);
float selectTime = 3;
SceneManager* scnManager;

// Xml Utilities
XmlNode* GetNode(XmlNode* root, std::vector<std::string> nodesnames)
{
	for (auto &name : nodesnames)
	{
		root = root->first_node(name.c_str());
	}
	return root;
}

std::vector<XmlNode*> GetChildNodes(XmlNode* parent, std::string childname)
{
	std::vector<XmlNode*> ans;
	for (XmlNode* childNode = parent->first_node(childname.c_str());
		childNode;
		childNode = childNode->next_sibling())
	{
		ans.push_back(childNode);
	}
	return ans;
}

void parseDebugInformation(bool& enable, int& _seed)
{
	// Parse Game Setting Xml file
	rapidxml::xml_document<> settings_xml;
	XmlNode * root_node;

	std::ifstream xmlFile("settings.xml");

	vector<char> buffer((istreambuf_iterator<char>(xmlFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	settings_xml.parse<0>(&buffer[0]);
	// Find our root node
	root_node = settings_xml.first_node("Game");

	XmlNode* node = root_node->first_node("ShowDebugWindow");
	enable = (node->value() == "1");

	node = root_node->first_node("Seed");
	_seed = std::stoi(node->value());
}

// Parse Color Tracking Settings
void parseColorTrackingFile(cv::Scalar& upperColor, cv::Scalar& lowerColor, float& upperRadius, float& lowerRadius)
{
	// Parse Game Setting Xml file
	rapidxml::xml_document<> settings_xml;
	rapidxml::xml_node<> * root_node;

	std::ifstream xmlFile("settings.xml");

	vector<char> buffer((istreambuf_iterator<char>(xmlFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	settings_xml.parse<0>(&buffer[0]);
	// Find our root node
	root_node = settings_xml.first_node("Game");

	// Get Color Boundary
	rapidxml::xml_node<> * upperNodeC = root_node->first_node("TrackingColor")->first_node("Upper");
	rapidxml::xml_node<> * lowerNodeC = root_node->first_node("TrackingColor")->first_node("Lower");

	std::string upperH(upperNodeC->first_attribute("h")->value());
	std::string upperS(upperNodeC->first_attribute("s")->value());
	std::string upperV(upperNodeC->first_attribute("v")->value());

	std::string lowerH(lowerNodeC->first_attribute("h")->value());
	std::string lowerS(lowerNodeC->first_attribute("s")->value());
	std::string lowerV(lowerNodeC->first_attribute("v")->value());

	upperColor = cv::Scalar(std::stof(upperH), std::stof(upperS), std::stof(upperV));
	lowerColor = cv::Scalar(std::stof(lowerH), std::stof(lowerS), std::stof(lowerV));

	// Get Radius Boundary
	rapidxml::xml_node<> * upperNodeR = root_node->first_node("TrackingRadius")->first_node("Upper");
	rapidxml::xml_node<> * lowerNodeR = root_node->first_node("TrackingRadius")->first_node("Lower");

	std::string upperR(upperNodeR->value());
	std::string lowerR(lowerNodeR->value());

	upperRadius = std::stof(upperR);
	lowerRadius = std::stof(lowerR);

	xmlFile.close();
}
// Parse tilesets.xml
void parseTileSetsFile(std::map<char, TileInform>& mapping)
{
	// read xml file
	rapidxml::xml_document<> tilsets_xml;
	rapidxml::xml_node<> * root_node;

	std::ifstream xmlFile(MAP_DIR + "tilesets.xml");

	vector<char> buffer((istreambuf_iterator<char>(xmlFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	tilsets_xml.parse<0>(&buffer[0]);
	// Find our root node
	root_node = tilsets_xml.first_node("Game");
	XmlNode* tilesets_node = root_node->first_node("TileSets");
	std::vector<XmlNode*> tilesNodes = GetChildNodes(tilesets_node, "Tile");

	// Iterate through all tiles
	for (auto & tileNode : tilesNodes)
	{

		TileInform& tile = mapping[tileNode->first_node("Code")->value()[0]];
		tile.Name = tileNode->first_attribute("name")->value();
		tile.TileCode = tileNode->first_node("Code")->value()[0];
		tile.ScorePerSecond = std::stoi(tileNode->first_node("Score")->value());
		
		std::string Type = tileNode->first_node("Images")->first_attribute("type")->value();
		if (Type == "sliced")
			tile.TileType = Sliced;
		else if (Type == "9-sliced")
			tile.TileType = NineSliced;
		else
			tile.TileType = Animated;

		std::vector<XmlNode*> imagesNodes = GetChildNodes(tileNode->first_node("Images"), "Image");

		// cout << "Images Node Count" << imagesNodes.size() << endl;

		for (auto &imageNode : imagesNodes)
		{
			std::string TileImageName = std::string(imageNode->value());

			cv::Ptr<Mat> mat = cv::makePtr<Mat>(imread(TILE_DIR + TileImageName, IMREAD_UNCHANGED));

			tile.TileImages.push_back(mat);
		}
	}

	xmlFile.close();
}
// Parse tilesets.xml items
void parseItemsFile(std::map<std::string, ItemInform>& mapping)
{
	// read xml file
	rapidxml::xml_document<> items_xml;
	rapidxml::xml_node<> * root_node;

	std::ifstream xmlFile(MAP_DIR + "tilesets.xml");

	vector<char> buffer((istreambuf_iterator<char>(xmlFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	items_xml.parse<0>(&buffer[0]);
	// Find our root node
	root_node = items_xml.first_node("Game");
	XmlNode* items_node = root_node->first_node("Items");
	std::vector<XmlNode*> items_nodes = GetChildNodes(items_node, "Item");

	// Iterate through all tiles
	for (auto & itemNode : items_nodes)
	{
		std::string ItemName = itemNode->first_attribute("name")->value();
		std::string ImageName = IMG_DIR + itemNode->first_node("Image")->value();
		std::string Ra = itemNode->first_node("Radius")->value();
		float Radius = std::stof(Ra);
		std::string Sc = itemNode->first_node("Score")->value();
		float Score = std::stof(Sc);
		std::string Sp = itemNode->first_node("Speed")->value();
		float Speed = std::stof(Sp);
		std::string De = itemNode->first_node("Death")->value();
		bool Kill = (De == "1") ? true : false;
		std::string Ea = itemNode->first_node("Eatable")->value();
		bool Eatable = (Ea == "1") ? true : false;

		ItemInform item;
		item.Name = ItemName;
		item.ImageName = ImageName;
		item.Radius = Radius;
		item.Score = Score;
		item.Speed = Speed;
		item.Kill = Kill;
		item.Eatable = Eatable;

		mapping[ItemName] = item;

		// cout << ItemName << ":" << ImageName << endl;
	}

	xmlFile.close();
}
// Parse Map Setting
void parseMapInform(float &offset, VECTOR::vector2f &origin)
{

	// Parse Game Setting Xml file
	rapidxml::xml_document<> settings_xml;
	XmlNode * root_node;

	std::ifstream xmlFile("settings.xml");

	vector<char> buffer((istreambuf_iterator<char>(xmlFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	settings_xml.parse<0>(&buffer[0]);
	// Find our root node
	root_node = settings_xml.first_node("Game");

	XmlNode* MapsNode = root_node->first_node("Maps");
	offset = std::stof(MapsNode->first_node("Offset")->value());
	float x = std::stof(MapsNode->first_node("Origin")->first_attribute("x")->value());
	float y = std::stof(MapsNode->first_node("Origin")->first_attribute("y")->value());
	origin = VECTOR::vector2f(x, y);

	xmlFile.close();
}

void parseSelectPosition(std::vector<VECTOR::vector2f>& rp, std::vector<VECTOR::vector2f>& lp)
{
	rp.clear();
	lp.clear();

	// Parse Game Setting Xml file
	rapidxml::xml_document<> settings_xml;
	XmlNode * root_node;

	std::ifstream xmlFile("settings.xml");

	vector<char> buffer((istreambuf_iterator<char>(xmlFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	settings_xml.parse<0>(&buffer[0]);
	// Find our root node
	root_node = settings_xml.first_node("Game");

	root_node = root_node->first_node("SelectPositions");

	selectTime = std::stof(root_node->first_node("Time")->value());

	colorOffset[0] = std::stof(root_node->first_node("Offset")->first_attribute("h")->value());
	colorOffset[1] = std::stof(root_node->first_node("Offset")->first_attribute("s")->value());
	colorOffset[2] = std::stof(root_node->first_node("Offset")->first_attribute("v")->value());

	XmlNode* rightNode = root_node->first_node("Right");
	XmlNode* leftNode = root_node->first_node("Left");

	std::vector<XmlNode*> rightNodes = GetChildNodes(rightNode, "Position");

	for (auto & posNode : rightNodes)
	{
		std::string X = posNode->first_attribute("x")->value();
		std::string Y = posNode->first_attribute("y")->value();
		// cout << "Selecting Points: " << X << ":" << Y << endl;

		VECTOR::vector2f pos(std::stof(X), std::stof(Y));
		rp.push_back(pos);
	}

	std::vector<XmlNode*> leftNodes = GetChildNodes(leftNode, "Position");

	for (auto & posNode : leftNodes)
	{
		std::string X = posNode->first_attribute("x")->value();
		std::string Y = posNode->first_attribute("y")->value();
		// cout << "Selecting Points: " << X << ":" << Y << endl;

		VECTOR::vector2f pos(std::stof(X), std::stof(Y));
		lp.push_back(pos);
	}
}

void loadSceneSelector(ObjectManager* objMgr)
{
	Mat eagle_Image = imread(IMG_DIR + "Eagle_00.png", IMREAD_UNCHANGED);
	Mat eagle_Mask = imread(IMG_DIR + "Eagle_00.png", IMREAD_UNCHANGED);

	Mat arrow_Image = imread(IMG_DIR + "arrow.png", IMREAD_UNCHANGED);
	Mat arrow_Mask = imread(IMG_DIR + "arrow_mask.png", IMREAD_UNCHANGED);

	Mat cursor_Image = imread(IMG_DIR + "cursor.png", IMREAD_UNCHANGED);
	Mat cursor_Mask = imread(IMG_DIR + "cursor.png", IMREAD_UNCHANGED);

	Object* colorSelectorObj =
		objMgr->CreateStaticObject(Vec2f(320, 240), 0);
	colorSelectorObj->AddComponent<ImageComponent>(ImageComponent(cursor_Image))->RotationEnabled = false;

	ColorSelector* colorSelector = colorSelectorObj->AddComponent<ColorSelector>(ColorSelector());

	Object* colorSelectManagerObj =
		objMgr->CreateStaticObject(Vec2f(0, 0), 0);
	ColorSelectManager* colorMgr = colorSelectManagerObj->AddComponent<ColorSelectManager>(ColorSelectManager(rightPos, leftPos));

	colorMgr->Selector = colorSelector;

	colorMgr->SelectTime = selectTime;

	Object* information =
		objMgr->CreateStaticObject(Vec2f(320, 350), 0);

	TextComponent* text = information->AddComponent<TextComponent>(TextComponent());

	text->Anchor = VECTOR::vector2f(0.5, 0.5);
	colorMgr->CounterText = text;
}

void loadTitleScene(ObjectManager* objMgr)
{
	Mat eagleSceneImage = imread(IMG_DIR + "eagle_scene.png", IMREAD_UNCHANGED);
	Mat rockSceneImage = imread(IMG_DIR + "rock_scene.png", IMREAD_UNCHANGED);
	Mat resetSceneImage = imread(IMG_DIR + "reset_scene.png", IMREAD_UNCHANGED);
	Mat plateImage = imread(IMG_DIR + "plate.png", IMREAD_UNCHANGED);
	///////////////////////////////////////////////////////////////////////////////////
	Object* titleObj =
		objMgr->CreateStaticObject(VECTOR::vector2f(0, 0), 0);
	TitleManager* titleManager =
		titleObj->AddComponent<TitleManager>(TitleManager());

	Object* plateObj =
		objMgr->CreateStaticObject(Vec2f(320, 218), 0);
	ImageComponent* plateImgComp = plateObj->AddComponent<ImageComponent>(ImageComponent(plateImage));

	// Information UI
	Object* Infomation =
		objMgr->CreateStaticObject(Vec2f(320, 50), 0);
	TextComponent* text =
		Infomation->AddComponent<TextComponent>(TextComponent());

	text->Text = "Rotate And Stay";
	text->Anchor = VECTOR::vector2f(0.5, 0.5);
	text->FontType = FONT_HERSHEY_PLAIN;
	text->Color = cv::Scalar(255, 255, 255);
	text->Scale = 1.5;

	// Image UI
	Object* eagleSceneObj =
		objMgr->CreateStaticObject(Vec2f(464, 287), 0);
	ImageComponent* eagleImgComp = eagleSceneObj->AddComponent<ImageComponent>(ImageComponent(eagleSceneImage));
	ImageLerper* eagleLerper = eagleSceneObj->AddComponent<ImageLerper>(ImageLerper(eagleImgComp));


	Object* resetSceneObj =
		objMgr->CreateStaticObject(Vec2f(320, 341), 0);
	ImageComponent* resetImgComp = resetSceneObj->AddComponent<ImageComponent>(ImageComponent(resetSceneImage));
	ImageLerper* resetLerper = resetSceneObj->AddComponent<ImageLerper>(ImageLerper(resetImgComp));


	Object* rockSceneObj =
		objMgr->CreateStaticObject(Vec2f(176, 287), 0);
	ImageComponent* rockImgComp = rockSceneObj->AddComponent<ImageComponent>(ImageComponent(rockSceneImage));
	ImageLerper* rockLerper = rockSceneObj->AddComponent<ImageLerper>(ImageLerper(rockImgComp));

	// Right Input Detector and Motion
	Object* rightTrackerObj =
		objMgr->CreateStaticObject(Vec2f(540, 240), 1);

	BallTrackerComponent* rightTracker =
		rightTrackerObj->AddComponent<BallTrackerComponent>
		(BallTrackerComponent(upperRadius, lowerRadius, ColorSelectManager::RightMaxColor + colorOffset, ColorSelectManager::RightMinColor - colorOffset));
	rightTracker->LerpToPosition = true;

	// Left Input Detector and Motion 
	Object* leftTrackerObj =
		objMgr->CreateStaticObject(Vec2f(100, 240), 1);

	BallTrackerComponent* leftTracker =
		leftTrackerObj->AddComponent<BallTrackerComponent>
		(BallTrackerComponent(upperRadius, lowerRadius, ColorSelectManager::LeftMaxColor + colorOffset, ColorSelectManager::LeftMinColor - colorOffset));
	leftTracker->LerpToPosition = true;

	// Circle Indicator
	Object* edgeCircleObj =
		objMgr->CreateStaticObject(Vec2f(320, 157), 0);

	CircleIndicator* edgeCircle =
		edgeCircleObj->AddComponent<CircleIndicator>(CircleIndicator());
	edgeCircle->Thickness = 1;
	edgeCircle->Radius = 80;

	Object* fillCircleObj =
		objMgr->CreateStaticObject(Vec2f(320, 157), 0);

	CircleIndicator* fillCircle =
		fillCircleObj->AddComponent<CircleIndicator>(CircleIndicator());
	fillCircle->Thickness = -1;
	fillCircle->Radius = 0;


	// title manager setup
	titleManager->EagleImage = eagleLerper;
	titleManager->ResetImage = resetLerper;
	titleManager->RockImage = rockLerper;

	titleManager->RightCursor = rightTracker;
	titleManager->LeftCursor = leftTracker;
	titleManager->EdgeCircle = edgeCircle;
	titleManager->FillCircle = fillCircle;
	titleManager->InformationText = text;

}

void loadSceneControl1(ObjectManager* objMgr)
{
	/////////////////////////////////////////////////////////////////////////////////////
	// load Image
	Mat ring_Image = imread(IMG_DIR + "ring.png", IMREAD_UNCHANGED);

	Mat eagle_Image = imread(IMG_DIR + "Eagle_00.png", IMREAD_UNCHANGED);
	Mat eagle_shadow = imread(IMG_DIR + "Eagle_Shadow_00.png", IMREAD_UNCHANGED);

	Mat progress_Image = imread(IMG_DIR + "bar.png", IMREAD_UNCHANGED);
	Mat progress_ptr_Image = imread(IMG_DIR + "positionPointer.png", IMREAD_UNCHANGED);
	/////////////////////////////////////////////////////////////////////////////////////
	// create object
	bool test;
	parseDebugInformation(test, seed);
	srand(seed);

	// Main Game Manager
	Object* MapManager =
		objMgr->CreateDynamicObject(Vec2f(-480, 0), Vec2f(0, 0), 1);
	MapManager->Name = "MapManager";
	TileMapManager* tileMapManager = MapManager->AddComponent<TileMapManager>(TileMapManager(tileMap, itemList, mapOffset));
	tileMapManager->LoadMap(MAP_DIR + "sea_short.txt");
	tileMapManager->Origin = mapOrigin;

	//ColorSelectManager::RightMinColor[2] = 50;
	//ColorSelectManager::LeftMinColor[2] = 50;

	// Right Input Detector and Motion
	Object* rightTrackerObj =
		objMgr->CreateDynamicObject(Vec2f(540, 240), Vec2f(0, 0), 1);
	/*
	BallTrackerComponent* rightTracker =
		rightTrackerObj->AddComponent<BallTrackerComponent>
		(BallTrackerComponent(upperRadius, lowerRadius, upperColor, lowerColor));
	*/
	BallTrackerComponent* rightTracker =
		rightTrackerObj->AddComponent<BallTrackerComponent>
		(BallTrackerComponent(upperRadius, lowerRadius, ColorSelectManager::RightMaxColor + colorOffset, ColorSelectManager::RightMinColor - colorOffset));
		
	rightTracker->LerpToPosition = true;

	MotionQuantifiler* rightMotion = rightTrackerObj->AddComponent<MotionQuantifiler>(MotionQuantifiler(rightTrackerObj));

	// Left Input Detector and Motion 
	Object* leftTrackerObj =
		objMgr->CreateDynamicObject(Vec2f(100, 240), Vec2f(0, 0), 1);
	/*
	BallTrackerComponent* leftTracker =
		leftTrackerObj->AddComponent<BallTrackerComponent>
		(BallTrackerComponent(upperRadius, lowerRadius, upperColor, lowerColor));
		*/
	
	BallTrackerComponent* leftTracker =
		leftTrackerObj->AddComponent<BallTrackerComponent>
		//(BallTrackerComponent(upperRadius, lowerRadius, upperColor, lowerColor));
		(BallTrackerComponent(upperRadius, lowerRadius, ColorSelectManager::LeftMaxColor + colorOffset, ColorSelectManager::LeftMinColor - colorOffset));
		
	leftTracker->LerpToPosition = true;
	
	MotionQuantifiler* leftMotion = leftTrackerObj->AddComponent<MotionQuantifiler>(MotionQuantifiler(leftTrackerObj));

	// Player
	Object* playerShadow =
		objMgr->CreateDynamicObject(Vec2f(0, -25), Vec2f(0, 0), 1);
	playerShadow->AddComponent<ImageComponent>(ImageComponent(eagle_shadow));
	playerShadow->AddComponent<Rotator>(Rotator());

	Object* Eagle =
		objMgr->CreateDynamicObject(Vec2f(320, -70), Vec2f(0, 0), 1);
	ImageComponent* eagleImgComp = Eagle->AddComponent<ImageComponent>(ImageComponent(eagle_Image));

	playerShadow->Parent = Eagle;
	playerShadow->GetComponent<Rotator>()->Target = Eagle;

	// Motion->Game
	SlidingMapController* slidingController =
		Eagle->AddComponent<SlidingMapController>(SlidingMapController(tileMapManager, Eagle, rightMotion, leftMotion));
	slidingController->IsControllable = false;

	tileMapManager->Player = slidingController;

	slidingController->imageComp = eagleImgComp;
	slidingController->IndicatorEnabled = true;

	// UI Setting
	Object* scoreText =
		objMgr->CreateDynamicObject(Vec2f(20, 440), Vec2f(0, 0), 0);
	TextComponent* text =
		scoreText->AddComponent<TextComponent>(TextComponent());
	text->IsEnabled = false;

	text->FontType = FONT_HERSHEY_PLAIN;
	text->Color = cv::Scalar(255, 255, 255);
	text->Scale = 1.5;

	Object* timerText =
		objMgr->CreateDynamicObject(Vec2f(320, 440), Vec2f(0, 0), 0);
	TextComponent* timertext =
		timerText->AddComponent<TextComponent>(TextComponent());
	timertext->IsEnabled = false;
	timertext->Anchor = VECTOR::vector2f(0.5, 0);
	timertext->Color = cv::Scalar(255, 255, 255);
	timertext->FontType = FONT_HERSHEY_PLAIN;
	timertext->Scale = 1.5;


	// UI Setting
	Object* speedText =
		objMgr->CreateDynamicObject(Vec2f(620, 440), Vec2f(0, 0), 0);
	TextComponent* spTxt =
		speedText->AddComponent<TextComponent>(TextComponent());
	text->IsEnabled = false;

	spTxt->Anchor = VECTOR::vector2f(1.0, 0);
	spTxt->FontType = FONT_HERSHEY_PLAIN;
	spTxt->Color = cv::Scalar(255, 255, 255);
	spTxt->Scale = 1.5;

	Object* progressBar =
		objMgr->CreateDynamicObject(Vec2f(630, 240), Vec2f(0, 0), 0);
	ImageComponent* progressBarImg = progressBar->AddComponent<ImageComponent>(ImageComponent(progress_Image));

	// 50 ~ 430
	Object* progressBarPtr =
		objMgr->CreateDynamicObject(Vec2f(629, 50), Vec2f(0, 0), 0);
	ImageComponent* progressPtrImg = progressBarPtr->AddComponent<ImageComponent>(ImageComponent(progress_ptr_Image));
	progressBarImg->IsEnabled = false;
	progressPtrImg->IsEnabled = false;

	tileMapManager->ScoreBoard = text;
	tileMapManager->TimerText = timertext;
	tileMapManager->SpeedText = spTxt;
	tileMapManager->ProgressBar = progressBarImg;
	tileMapManager->ProgressPointer = progressPtrImg;
}

void loadRodScene(ObjectManager* objMgr)
{
	/////////////////////////////////////////////////////////////////////////////////////
	// load Image
	Mat rock_Image = imread(IMG_DIR + "rock.png", IMREAD_UNCHANGED);
	Mat BackGroundImage = imread(IMG_DIR + "RollRockMap.png", IMREAD_UNCHANGED);

	Mat elf_Image = imread(IMG_DIR + "black_normal.png", IMREAD_UNCHANGED);
	Mat elf_surprised_Image = imread(IMG_DIR + "black_surprised.png", IMREAD_UNCHANGED);
	Mat elf_dead_Image = imread(IMG_DIR + "black_sad.png", IMREAD_UNCHANGED);

	/////////////////////////////////////////////////////////////////////////////////////

	bool test;
	parseDebugInformation(test, seed);
	srand(seed);

	// Main Game Manager
	Object* MapManager =
		objMgr->CreateDynamicObject(Vec2f(-480, 0), Vec2f(0, 0), 1);
	MapManager->Name = "MapManager";
	TileMapManager* tileMapManager = MapManager->AddComponent<TileMapManager>(TileMapManager(tileMap, itemList, mapOffset, TileMapManager::GameMode::Rock));

	Object* bgmObj =
		objMgr->CreateStaticObject(Vec2f(320, 240), 1);
	bgmObj->AddComponent<ImageComponent>(ImageComponent(BackGroundImage));

	// Right Input Detector and Motion
	Object* rightTrackerObj =
		objMgr->CreateDynamicObject(Vec2f(540, 240), Vec2f(0, 0), 1);

	BallTrackerComponent* rightTracker =
		rightTrackerObj->AddComponent<BallTrackerComponent>
		(BallTrackerComponent(upperRadius, lowerRadius, ColorSelectManager::RightMaxColor + colorOffset, ColorSelectManager::RightMinColor - colorOffset));
	rightTracker->LerpToPosition = true;

	// Left Input Detector and Motion 
	Object* leftTrackerObj =
		objMgr->CreateDynamicObject(Vec2f(100, 240), Vec2f(0, 0), 1);

	BallTrackerComponent* leftTracker =
		leftTrackerObj->AddComponent<BallTrackerComponent>
		(BallTrackerComponent(upperRadius, lowerRadius, ColorSelectManager::LeftMaxColor + colorOffset, ColorSelectManager::LeftMinColor - colorOffset));
	leftTracker->LerpToPosition = true;
	
	// Player, SlidingController
	Object* rod =
	objMgr->CreateDynamicObject(Vec2f(320, 100), Vec2f(0, 0), 1);
	rod->Name = "Rod";
	BalancingRod* rodComp = rod->AddComponent<BalancingRod>(BalancingRod(200));
	BalancingRodController* rodCompController =
		rod->AddComponent<BalancingRodController>(BalancingRodController(rightTracker, leftTracker , rodComp));
	rodComp->IsControllable = false;
	
	Object* rodCharacter =
	objMgr->CreateDynamicObject(Vec2f(320, 100), Vec2f(0, 0), 1);
	rod->Name = "RodCharacter";
	ImageComponent* characterImg = rodCharacter->AddComponent<ImageComponent>(ImageComponent(elf_Image));
	BalancingCharacter* character = rodCharacter->AddComponent<BalancingCharacter>(BalancingCharacter(rodComp));
	character->NormalImg = elf_Image;
	character->SurprisedImg = elf_surprised_Image;
	character->DeadImg = elf_dead_Image;
	rodCharacter->AddComponent<ImageLerper>(ImageLerper(characterImg));

	SlidingMapController* slidingController =
		rod->AddComponent<SlidingMapController>(SlidingMapController(tileMapManager, rod, nullptr, nullptr));
	slidingController->IsControllable = false;

	// UI Setting
	Object* ScoreText =
		objMgr->CreateDynamicObject(Vec2f(320, 440), Vec2f(0, 0), 0);
	TextComponent* scTxt =
		ScoreText->AddComponent<TextComponent>(TextComponent());
	scTxt->IsEnabled = true;

	scTxt->Anchor = VECTOR::vector2f(0.5, 0.5);
	scTxt->FontType = FONT_HERSHEY_PLAIN;
	scTxt->Color = cv::Scalar(255, 255, 255);
	scTxt->Scale = 1.5;

	scTxt->Text = "0";

	// UI Setting
	Object* GameTx =
		objMgr->CreateDynamicObject(Vec2f(320, 0), Vec2f(0, 0), 0);
	TextComponent* goTxt =
		GameTx->AddComponent<TextComponent>(TextComponent());
	goTxt->IsEnabled = false;

	goTxt->Anchor = VECTOR::vector2f(0.5, 0.5);
	goTxt->FontType = FONT_HERSHEY_PLAIN;
	goTxt->Color = cv::Scalar(255, 255, 255);
	goTxt->Scale = 3.0;
	goTxt->Thickness = 3;

	goTxt->Text = "GAME OVER";

	tileMapManager->MapController = slidingController;
	tileMapManager->RockPlayer = character;
	tileMapManager->PlayerRod = rodComp;
	tileMapManager->ScoreBoard = scTxt;
	tileMapManager->SpeedText = goTxt;
}

void VideoCaptureLoop(Mat &targetFrame, std::atomic_flag& targetLock, std::atomic<bool>& targetGrabbed, std::atomic<bool>& stop)
{
	VideoCapture captureDevice;
	captureDevice.open(0);

	while (!stop)
	{
		if (targetGrabbed && !targetLock.test_and_set(std::memory_order_acquire))
		{
			// nextFrameBuffer = Mat(HEIGHT, WIDTH, CV_8UC3, cv::Scalar(50, 0, 0));
			captureDevice >> targetFrame;
			flip(targetFrame, targetFrame, 1);
			targetGrabbed = false;
			targetLock.clear(std::memory_order_release);
		}
	}
}

void MainLoop()
{
	//////////////////////////// CAMERA LOOP /////////////////////////////////////
	std::atomic_flag targetLock = ATOMIC_FLAG_INIT;
	std::atomic<bool> targetGrabbed = false;
	std::atomic<bool> stop = false;

	// capture first frame
	VideoCapture captureDevice;
	captureDevice.open(0);

	Mat currFrameBuffer;
	Mat nextFrameBuffer;

	captureDevice >> nextFrameBuffer;
	flip(nextFrameBuffer, currFrameBuffer, 1);

	std::thread videoCaptureLoop
		(VideoCaptureLoop, nextFrameBuffer, std::ref(targetLock), std::ref(targetGrabbed), std::ref(stop));

	//////////////////////////// GAME MAIN LOOP /////////////////////////////////////
	double deltaTime = 0;
	float test_timer = 0;
	int currentScene = 0;
	bool IsLoadingScene = false;

	while (true){
		std::clock_t start = std::clock();

		///////////////////////// Frame Capture Logic /////////////////////////
		// Get Target Frame Pointer
		Mat* TargetFrame = scnManager->GetCurrentScene()->GetTargetFrame();

		// New Target Frame has been generated and not been grabbed
		if (!targetGrabbed && !targetLock.test_and_set(std::memory_order_acquire))
		{
			currFrameBuffer = nextFrameBuffer.clone();
			targetGrabbed = true;
			targetLock.clear(std::memory_order_release);
		}

		*TargetFrame = currFrameBuffer.clone();
		if (TargetFrame->empty()){
			cout << "Error: No camera input" << endl;
			stop = true;
			break;
		}
		///////////////////////// Game Engine Logic /////////////////////////
		scnManager->Update(deltaTime);
		scnManager->LateUpdate(deltaTime);

		///////////////////////////// Rendering /////////////////////////////
		// Mat finalFrame = scnManager->CurrentScene->GetWindowFrame();
		cv::imshow("EagleSimulator", *TargetFrame);
		if (BallTrackerComponent::thresholdTemp.size().area() > 0 && SceneManager::ShowDebugeWindow)
			cv::imshow("Threshold", BallTrackerComponent::thresholdTemp);
		if (waitKey(1) == 27)
		{
			stop = true;
			break;
		}
		/////////////////////////////////////////////////////////////////////
		if (!IsLoadingScene)
		{
			IsLoadingScene = scnManager->m_LoadSceneRequest;
			deltaTime = (std::clock() - start) / (double)CLOCKS_PER_SEC;
		}
		else
		{
			IsLoadingScene = scnManager->m_LoadSceneRequest;
			deltaTime = 0;
		}

		// while (waitKey(1) != 27);
		// std::cout << "FPS: " << 1.0 / deltaTime << std::endl;
	}
	videoCaptureLoop.join();

	char halt;
	std::cin >> halt;
	return;
}

int main(int argc, const char** argv)
{
	//////////////////////////// GAME SETTING INIT
	parseDebugInformation(SceneManager::ShowDebugeWindow, seed);
	parseColorTrackingFile(upperColor, lowerColor, upperRadius, lowerRadius);
	parseMapInform(mapOffset, mapOrigin);
	parseTileSetsFile(tileMap);
	parseSelectPosition(rightPos, leftPos);
	parseItemsFile(itemList);
	//////////////////////////// GAME ENGINE SETUP /////////////////////////////////////
	scnManager = new SceneManager();								// create manager
	scnManager->CreateNewScene(loadSceneSelector);					// create Scene0
	scnManager->CreateNewScene(loadTitleScene);						// create Scene1
	scnManager->CreateNewScene(loadSceneControl1);					// create Scene2
	scnManager->CreateNewScene(loadRodScene);						// create Scene3
	scnManager->InstantLoadScene(0);								// load Scene0

	MainLoop();
	return 0;
}




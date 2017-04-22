#ifndef DYENGINE_COMPONENT_H
#define DYENGINE_COMPONENT_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <memory>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include "Object.h"
#include "Shape.h"
#include "SceneManager.h"
#include "Collision.h"


using namespace std;
using namespace cv;

class Scene;
class Object;
class Collision;

//! Base type of abstract component
class Component
{
public:
	Object* m_Transform = nullptr;
	bool IsEnabled = true;

	Component();
	~Component();

	virtual void Start() = 0;
	virtual void Update(float timeStep) = 0;
	virtual void LateUpdate(float timeStep) {};

	virtual void AttachTo(Object* obj);
	virtual void OnCollision(Collision& col) {}
	virtual void OnTrigger(Collision& col) {}
};

//! Component that deals with drawing 2D sprite, notice that the original image is always untouched
class ImageComponent : public Component
{
public:
	bool UseGlobalMask = true;
	//! The purpose for this function due to the flawed in opencv library, applying a Affine Transformation causes memory leak
	bool RotationEnabled = true;
	bool ReqestedUpdate = false;

	Mat m_Image, m_Mask;
private:
	bool m_UseMask = true;

	int m_PreRotAngle = 0;
	VECTOR::vector2f m_PreScale = VECTOR::vector2f(1, 1);

	//! old variable, legacy
	/*
	Mat m_ResizeImage;
	Mat m_ResizeMask;

	Mat m_ResizeRotImage;
	Mat m_ResizeRotMask;

	// recalculate resize image from original image, rotated image is recaculated too
	void calculateResizeImage(VECTOR::vector2f _size);
	// recalculate rotated image from resized image
	void calculateRotateImage(float _angle);
	////////////////////////////////////////
	*/

	Mat m_OutputImage;
	Mat m_OutputMask;

public:
	static cv::Scalar GlobalMaskColor;

	// BGR color
	cv::Scalar Color = cv::Scalar(255, 255, 255);
	// color alpha
	double ColorAlpha = 255;

	ImageComponent() {}
	ImageComponent(std::string _imageFileName);
	ImageComponent(Mat _image);
	ImageComponent(Mat _image, Mat _mask, bool _useMask = true);

	~ImageComponent();

	void Start() override;
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override;
	void AttachTo(Object* obj) override;

private:
	// dummy Graphic System, to be fixed
	static std::map<std::string, Mat> m_ImageCacheSystem;

	//! New Version
	//! recalculate rotated image from original image
	void calculateRotateImage(Mat& dst, Mat& mskDst);

	void drawOnTarget(Mat* target);
};

//! Component that show text
class TextComponent : public Component
{
public:
	std::string Text = "";

	//! (0, 0) is bottom-left, (1, 1) is top-right
	VECTOR::vector2f Anchor = VECTOR::vector2f(0, 0);

	//! Scale
	float Scale = 1;

	//! Thick
	int Thickness = 2;

	//! Color
	cv::Scalar Color = cv::Scalar(255, 255, 255);

	//! Font
	//!
	//	FONT_HERSHEY_SIMPLEX = 0,
	//	FONT_HERSHEY_PLAIN = 1,
	//	FONT_HERSHEY_DUPLEX = 2,
	//	FONT_HERSHEY_COMPLEX = 3,
	//	FONT_HERSHEY_TRIPLEX = 4,
	//	FONT_HERSHEY_COMPLEX_SMALL = 5,
	//	FONT_HERSHEY_SCRIPT_SIMPLEX = 6,
	//	FONT_HERSHEY_SCRIPT_COMPLEX = 7,
	//	FONT_ITALIC = 16
	int FontType = 0;

public:
	TextComponent() {}

	void Start() override {}
	void Update(float timeStep) override {}
	void LateUpdate(float timeStep) override;

private:

};

//! Component that deals with animating 2D sprite, Image Component has to be attached first
// TO DO:
class AnimationComponent : public Component
{
public:
	ImageComponent* Sprite = nullptr;
	int SheetsCount = 2;
	float SheetDuration = 1.0f;
	// std::vector<Mat> Sheets;
public:
	//
	AnimationComponent() {}

private:
	int m_SheetCounter = 0;
};

//! Component that Lerp Image Main Color
class ImageLerper : public Component
{
public:
	ImageComponent* TargetImage = nullptr;

	float LerpTimer = 0.0f;
	bool IsLerping = false;
	cv::Scalar TargetColor;
	float TargetAlpha;

	ImageLerper(ImageComponent* targetImg) : TargetImage(targetImg) {}

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}

	void LerpColorTo(cv::Scalar col, float time, float alpha = -1.0f);
	cv::Scalar GetTargetColor() { return TargetImage->Color; }
	float GetTargetColorAlpha() { return TargetImage->ColorAlpha; }
private:

};

//! Component for debugging
class DebuggerComponent : public Component
{
public:
	int LogID;
	DebuggerComponent(int debugLogID) : LogID(debugLogID) { std::cout << "Debugger Comp ID: " << LogID << std::endl; }
	void Start() override {}
	void Update(float timeStep) override {}

};

//! Component for following Rotation
class Rotator : public Component
{
public:
	Object* Target = nullptr;
	Rotator() {}
	void Start() override {}
	void Update(float timeStep) override;
};

//! Draw a circle on the object position
class CircleIndicator : public Component
{
public:
	cv::Scalar Color = CV_RGB(255, 255, 255);
	int Radius = 20;
	int Thickness = 1;

	CircleIndicator() {}

	void Start() override {}
	void Update(float timeStep) override {}
	void LateUpdate(float timeStep) override;

};

//! Component that detects circle in the target frame within target color range
class BallTrackerComponent : public Component
{
public:
	static Mat thresholdTemp;

	bool IndicatorEnabled = true;		// draw the contour of the tracking ball on the screen
	bool FollowingEnabled = true;		// object position follows the tracking target
	bool LerpToPosition = true;			// move the position smoothly if true
	float LerpSpeed = 20.0f;

	int MaxRadius = 50;					// 
	int MinRadius = 20;

	cv::Scalar MaxHSVColor;
	cv::Scalar MinHSVColor;

	cv::Vec2f TargetCenter;
	float TargetRadius;

	bool IsTracked = false;
	
	cv::Scalar DrawColor = cv::Scalar(255, 255, 255);
	int DrawRadius = 20;
	int DrawThickness = 2;
public:
	BallTrackerComponent(int maxR, int minR, cv::Scalar maxCol, cv::Scalar minCol);

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override;

private:
	bool calculateTargetCenter(Mat& frame, cv::Vec2f &_center, float &_radius);

};

//! Identical to BallTracker, in addition to that, DuoBallTracker Detect two balls with sides, and manipulate multiple object to indicate
class DuoBallTracker : public Component
{
public:
	struct Ball
	{
		VECTOR::vector2f Center;
		float Radius;
	};

	bool IndicatorEnabled = true;			// draw the contour of the tracking ball on the screen (enable inidicator)
	bool LerpInidicatorToPosition = true;	// move the indicator position smoothly if true

	int MaxRadius = 50;					
	int MinRadius = 20;

	cv::Scalar MaxHSVColor;
	cv::Scalar MinHSVColor;

	VECTOR::vector2f DivisionAxis = VECTOR::vector2f(0, 1);			// Axis that used to identical two balls

	int QualifiedBallCount = 0;

	std::vector<Ball> BallList;
	Object* rightIndicator;
	Object* leftInidicator;
public:
	DuoBallTracker(int maxR, int minR, cv::Scalar maxCol, cv::Scalar minCol);

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}
	void AttachTo(Object* obj) override;

private:
	void init();
	int calculateTargetCenter(Mat frame, std::vector<Ball>& balls);

};

//! Select a color at transform position
class ColorSelector : public Component
{
public:
	enum SelectorState
	{
		Idle,
		Moving,
		Counting
	};

	SelectorState State = Idle;

	cv::Scalar MaxColor;
	cv::Scalar MinColor;
	cv::Scalar AverageColor = cv::Scalar(0, 0, 0);

	std::vector<cv::Scalar> AllColors;
	float m_TimerToSelectColor = 5.0f;
public:
	ColorSelector() {}
	~ColorSelector() {}

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}

	//! Clean Up All recorded color
	void Initialize()
	{
		AverageColor = cv::Scalar(0, 0, 0);
		AllColors.clear();
	}

	//! Call this to select a new color at position
	void SelectNewColor(const VECTOR::vector2f& _atPos, float _countDownAfterReached = 3.0f);

private:
	VECTOR::vector2f m_NextPosition;

	void cacheColor();
};

//! Component that uses steering behaviour to control the velocity of a dynamic object
class SteeringController : public Component
{
public:

	enum SteeringBehaviour
	{
		Seeking,
		Fleeing,
		Wandering
	};

	Object* m_Target;
	float MaxSpeed;
	float MaxAcceleration;

	SteeringBehaviour Behaviour = Seeking;
	bool UpdateRotationEnabled = true;
	float RotationOffset = 0.0f;

public:
	SteeringController(Object* _target, float _maxS, float _maxA) :
		m_Target(_target), MaxSpeed(_maxS), MaxAcceleration(_maxA) {}

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}

private:

	VECTOR::vector2f Seek(Object* Target);
	VECTOR::vector2f Flee(Object* Target);
	// Wander
	// Pursue
	// Arrive
};

//! Component that Quantify motion of a balltracker,  
class MotionQuantifiler : public Component
{
public:
	Object* TargetObj;
	VECTOR::vector2f Motion;
	float Value = 0.0f;
	VECTOR::vector2f Axis = VECTOR::vector2f(0, 1);
	bool ProjectOnAxis = true;

	MotionQuantifiler(Object* ball);

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}

private:
	std::list<VECTOR::vector2f> m_HistoryPosition;
	VECTOR::vector2f m_PrePosition;
};

//! Component that can be used to simulate slope physics
class BalancingRod : public Component
{
public:
	bool IsControllable = true;
	float Radius = 200.0f;
	VECTOR::vector2f Gravity = VECTOR::vector2f(0.0f, -200.0f);

	bool DrawLineEnabled = true;
	int LineThickness = 5;
	cv::Scalar LineColor = cv::Scalar(255, 255, 255);
	cv::Scalar InnerLineColor = cv::Scalar(0, 0, 0);

	float RotationLimit = 50;

	// Debugger
	float _timer = 0.0f;
	bool _incre = true;

public:
	BalancingRod(float _radius = 200.0f, VECTOR::vector2f _gravity = VECTOR::vector2f(0, -200))
		: Radius(_radius), Gravity(_gravity) {}

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override 
	{
		drawRod();
	}

	//! set the rotation angle based on two points
	void SetAngle(const VECTOR::vector2f &p_left, const VECTOR::vector2f & p_right);

	//! get a sliding force based on the gravity and line
	VECTOR::vector2f GetSlidingForce();

	//! get a point on the line, value ranging from -1 ~ 1
	VECTOR::vector2f GetPointAt(float value, bool clamp = true);

	//! get a vector that represents the line direction
	VECTOR::vector2f GetDirectionVector();

private:
	//! draw a line based on the rotation and position of parent object
	void drawRod();
};

//! Component that represent Dust Character. Managing Movement, Animation, Need Image Component
// TO DO: Animation
class BalancingCharacter : public Component
{
public:
	bool IsDead = false;

	bool finishDead = false;

	BalancingRod* Rod = nullptr;
	float Mass = 0.8f;
	float Value = 0.0f;
	float Speed = 0.0f;
	float BouncingLoss = 0.3f;
	float FrictionLoss = 0.5f;
	float Radius = 16;
	float RotationMultiplier = (5.0f / 96.0f) * 180 / 3.1415;

	float DeathAnimationTime;

	float MassMultiplier = 1.0f;
	float BouncingLossMultiplier = 1.0f;

	bool FaceGravityEnabled = false;

	Mat NormalImg;
	Mat SurprisedImg;
	Mat DeadImg;

	VECTOR::vector2f PosOffset = VECTOR::vector2f(0, 16);

	ImageLerper* ImgLerper;

public:
	BalancingCharacter(BalancingRod* _rod) : Rod(_rod) {}

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}

public:
	float GetForce();
	void Normal();
	void Surprised();
	void Dead();

private:
	ImageComponent* ImageComp;
};

//! Component that takes ball tracker as input to control BalancingRod
class BalancingRodController : public Component
{
public:
	BallTrackerComponent* Cursor1 = nullptr;
	BallTrackerComponent* Cursor2 = nullptr;
	BalancingRod* Rod = nullptr;

public:
	BalancingRodController(BallTrackerComponent* _cursor1, BallTrackerComponent* _cursor2, BalancingRod* _rod)
		: Cursor1(_cursor1), Cursor2(_cursor2), Rod(_rod) {}

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}

private:
};

class TileMapManager;

//! Component that takes Motion Quantifier as input to slide Map Object
class SlidingMapController : public Component
{
public:
	TileMapManager* SlidingMap;
	Object* Player;
	MotionQuantifiler* RightMotionIndicator = nullptr;
	MotionQuantifiler* LeftMotionIndicator = nullptr;

	float MinSlidingSpeed = 50.0f;
	float MaxSlidingSpeed = 900.0f;
	float SpeedLoss = 0.5f;						// Air Resistance
	float AccelMultiplier = 1.2f;
	bool IsControllable = true;
	bool IndicatorEnabled = false;

	float Speed = 50.0f;

	float Radius = 35;

	VECTOR::vector2f SlidingVelocity = VECTOR::vector2f(0.0f, -100.0f);

	float HitCoolDown = 1.0f;
	float FlickrTime = 0.05f;
	ImageComponent* imageComp;

public:
	SlidingMapController(TileMapManager* map, Object* player, MotionQuantifiler* rmotion, MotionQuantifiler* lmotion);

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override;

	VECTOR::vector2f InputMiddlePoint();
	float InputAngle();

	void AddSpeed(float speed);
	void AddScore(float score);

private:
	float coolDownTimer = 0.0f;
	float flickrTimer = 0.0f;

	void caculateInputMotion(float timeStep);
	void updateVelocity(float timeStep);

	void drawCompass();
};


class Tile;
class Item;
class MapRegion;

enum TileImageType
{
	Sliced,
	NineSliced,
	Animated
};

//! Struct that is used to pass tile generation information, char -> TileInform
struct TileInform
{
	std::string Name;
	char TileCode;								//! Tile code according to txt file
	float ScorePerSecond;						//! Tile deals damage per second
	TileImageType TileType = Sliced;
	std::vector<cv::Ptr<Mat>> TileImages;			//! Tile Animation Sheets
	/*
		TO DO: Animation Sheets frame
	*/
};

//! Struct that is used to pass item information
struct ItemInform
{
	//<Item name = "Rock01">
	//	<Image>mount_01.png< / Image>
	//	<Radius>45< / Radius>
	//	<Score>0< / Score>
	//	<Speed>-500< / Speed>
	//	<Death>0< / Death>
	//< / Item>
	std::string Name;
	std::string ImageName;
	float Radius;
	float Score;						
	float Speed;
	bool Kill = false;
	bool Eatable = false;
};
//! Component that manages map tiles and 2D grid trigger detection, also game progress
//! first element = y, second element = x
//! also manage game flow
// TO DO
class TileMapManager : public Component
{
public:
	enum GameState
	{
		Preparing,
		Playing,
		GameOver,
		Result
	};

	bool ItemPoolSpawned = false;
	bool UseRegionMode = true;
	//! Origin of Map Offset
	VECTOR::vector2f Origin = VECTOR::vector2f(24, 24);
	
	//! Loop on x coordinate, X Map is divided by 2 into Region
	bool LoopXEnabled = true;
	//!Grids pos offset
	int TileOffset;
	//!Grids count inx
	int Width;
	//! Grids count in y
	int Height;
	//! Total Length on y
	int MapLength = 100;
	//! Total play time for limit mode
	float MapTime = 50.0f;

	enum GameMode
	{
		Eagle,
		Rock
	};

	//! GameMode
	GameMode Mode = Eagle;

	// inside Timer
	float Timer = 50.0f;
	float ProgressValue = 0.0f;

	GameState State = Preparing;
	float StartDelay = 2.0f;
	VECTOR::vector2f StartPlayerPosition = VECTOR::vector2f(320, 80);

	float Score = 0.0f;
	float ScorePerSec = 150.0f;
	ImageComponent* ProgressBar = nullptr;
	ImageComponent* ProgressPointer = nullptr;
	TextComponent* ScoreBoard = nullptr;
	TextComponent* TimerText = nullptr;
	TextComponent* SpeedText = nullptr;
	SlidingMapController* Player = nullptr;

	//! List
	std::map<char, TileInform> TileSet;
	std::map<std::string, ItemInform> ItemSet;

	std::string CurrentMapName;

	//! Useless if not in Region Mode
	std::vector<std::vector<MapRegion*>> Regions;

	//! Global Bounded Tile
	std::vector<std::vector<Tile*>> TileMap;
	//! Global Bounded Items, useless
	std::vector<Item*> Items;
public:
	TileMapManager(std::map<char, TileInform> mapping, std::map<std::string, ItemInform> itemList, int _offset = 48, GameMode _mode = Eagle);

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}

	void UpdateEagleMode(float timeStep);

	void UpdateRockMode(float timeStep);

	//! Load Map
	void LoadMap(std::string _mapName);

	//! Called after LoadMap
	void InstantiateTiles(std::vector<std::vector<char>>& _charMap);

	//! Given a world space position, return the map space position
	VECTOR::vector2f MapSpacePosition(VECTOR::vector2f worldPos);

	//! Given a map space position, return the region it is at. (region(0~1, 0~1))
	VECTOR::vector2i MapSpaceAtRegion(VECTOR::vector2f mapPos);

	//! Given a world space position, return the region it is at. (region(0~1, 0~1))
	VECTOR::vector2i WorldSpaceAtRegion(VECTOR::vector2f worldPos);

	//! get region logic size
	VECTOR::vector2i GetRegionSize();

	//! get region real size in the world
	VECTOR::vector2f GetRegionRealSize();

	//! check if player reach the end (MapLength for EagleMode, Death for Rock)
	bool IsGameOver();

private:
	VECTOR::vector2i currentRegion = VECTOR::vector2i(0, 0);
	VECTOR::vector2i currentSectorInRegion = VECTOR::vector2i(0, 0);

	//! Use in RegionMode, Region is dynamically move to fit the renderering center position or procedurally generate tiles
	void handleRegionMode(float timeStep);

	//////////////////////////// EAGLE MODE ///////////////////////////////////
	VECTOR::vector2f tempSlidingSpeed;
	float finalScore;
	bool overTime = false;
	bool finishScoreCounting = false;
	float resultDelay = 5.0f;

	//! the position y where the game really starts
	float startPosY;

	
	//////////////////////////// ITEM GENERATE ///////////////////////////////////
	std::vector<Item*> ringPool;
	std::vector<Item*> rockPool;

	Item* getRing();
	Item* getRock();

	/// RING SETTING
	// Ring is always spawned with 3 ~ 8 rings, in a line or sine wave 
	//! How often ring is generated
	float ringTimer;
	float ringDuration = 2.0f;
	int ringMaxNum = 7;
	int ringMinNum = 3;
	int ringMaxWidth = 4;
	int ringMinWidth = 1;
	float minRingRadius = 400;
	float maxRingRadius = 450;
	float minRingDegree = 70;
	float maxRingDegree = 110;
	VECTOR::vector2f ringOffset = VECTOR::vector2f(30, 45);

	/// ROCK SETTING
	// Rock is spawned with 1 rocks every 2 sec
	//! 
	float rockTimer;
	float rockDuration = 1.0f;
	float minRockRadius = 500;
	float maxRockRadius = 550;
	float minDegree = 45;
	float maxDegree = 135;


	// Pointer initial Y = 50 ~ 445, 445 - 50 = 395
	///////////////////////////////////////////////////////////////////////////
	//////////////////////////// ROCK MODE ///////////////////////////////////

public:
	BalancingRod* PlayerRod;
	BalancingCharacter* RockPlayer;
	SlidingMapController* MapController;
	float DistanceWalked = 0.0f;

	float DeathAnimationTime = 1.0f;
	float rockResultDelay = 3.0f;

	//////////////////////////// ITEM GENERATE ///////////////////////////////////

private:
	std::vector<Item*> holePool;
	Item* getHole(float radius);

	/// HOLE SETTING
	//! 
	float minSlidingSpeed = 30.0f;
	float slidingSpeed = 30.0f;
	float maxSlidingSpeed = 70.0f;

	int numPerSpawn = 2;

	float holeSpawnOffsetY = 600;

	float beginHoleYMin = 240.0f;
	float beginHoleYMax = 720.0f;
	float beginHoleYOffset = 120.0f;

	int spawnCounter = 0;
	float distanceCounter = 0.0f;
	float holeMaxSpawnDistance = 120;
	float currentHoleDistance;
	float holeMinSpawnDistance = 80;
	//! PlayerRadius ~ MaxRadius
	float maxHoleRadius = 80;


	// Pointer initial Y = 50 ~ 445, 445 - 50 = 395
	///////////////////////////////////////////////////////////////////////////
	void updateProgressBar(float timeStep);
	
	//! update global items and instantiate procedurally
	void updateItemPoolStateRock(float timeStep);

	void updateItemPoolStateEagle(float timeStep);

	void beginRandomHoleSpawn(float minY, float maxY, float offsetY);

	//! generate one specified item on position, (move item to position and enabled)
	void spawnItemAt(Item* item, VECTOR::vector2f pos, float Radius = -1.0f);

	// Parse Map File to Char Arra
	std::vector<std::vector<char>> parseMap(std::string _mapName);
	int slicedCode(const std::vector<std::vector<char>>& _charMap, VECTOR::vector2i _coord);
	int coord2slicedCode(int x, int y)
	{
		if (y == 0)
		{
			return x + 1;
		}
		else if (y == 1)
		{
			return (x == 0) ? 4 : (x == 1) ? 0 : 5;
		}
		else
		{
			return x + 6;
		}
	}
	VECTOR::vector2i slicedCode2Coord(int code)
	{
		if (code == 0)
		{
			return VECTOR::vector2i(1, 1);
		}
		else if (code >= 1 && code <= 3)
		{
			return VECTOR::vector2i((code - 1), 0);
		}
		else if (code >= 6 && code <= 8)
		{
			return VECTOR::vector2i((code - 6), 2);
		}
		else
		{
			if (code == 4)
				return VECTOR::vector2i(0, 1);
			else
				return VECTOR::vector2i(2, 1);
		}
	}

	//! Debug Utility
	void printCharMap(const std::vector<std::vector<char>>& _charMap);
};

//! Used to Divide Tile Map into pieces for dynamic management
class MapRegion : public Component
{
public:
	TileMapManager* MapManager;
	VECTOR::vector2i RegionId = (0, 0);
	std::vector<std::vector<Tile*>> TileMap;
	std::vector<Item*> Items;
	int Height;
	int Width;

public:
	MapRegion() {}

	void Start() override {}
	void Update(float timeStep) override {}
	void LateUpdate(float timeStep) override {}

	void Init(TileMapManager* mgr, VECTOR::vector2i id);
	//! Restore all item state in this region to active
	void RestoreItems();

	VECTOR::vector2f CenterWorldPosition();
	VECTOR::vector2f RegionSpacePosition(const VECTOR::vector2f& worldPos);
};

//! Mpa Object Tile Grid, managed by map
class Tile : public Component
{
public:
	TileMapManager* MapManager = nullptr;
	char TileCode = ' ';					//! ASCII that is used to reprsent this tile
	float ScorePerSecond = 0.0f;			//! Score added per second if player is on it
	VECTOR::vector2i Coordinate;			//! Position coordinate in a map
	TileImageType ImageType = Sliced;

public:
	Tile(TileMapManager* _mgr, char _code = ' ', float _score = 0.0f)
		: MapManager(_mgr), TileCode(_code), ScorePerSecond(_score) {}

	void Start() override {}
	void Update(float timeStep) override {}
	void LateUpdate(float timeStep) override {}

private:

};

//! Map Object Item, eaten by Player Dust
class Item : public Component
{
public:
	TileMapManager* MapManager = nullptr;

	//! Legacy
	float ScorePerEaten = -5.0f;

	//! Legacy
	bool VanishAfterEaten = true;
public:
	Item() {}
	Item(ItemInform& inform);

	bool AutoDisableIfOutOfScreen = true;

	float Radius;
	float Score;
	float Speed;
	bool Kill = false;
	bool Eatable = false;

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override;

	//! Collide with sliding player version
	void CollideWithPlayer(SlidingMapController* player);
	//! Collide with balancing player version
	void CollideWithPlayer(BalancingCharacter* player);
	//! recover state of item, Legacy
	void Restore();
private:
	void checkCollisionWithPlayer(SlidingMapController* player);
	void checkCollisionWithPlayer(BalancingCharacter* player);
};

//! Component that manages two colors select scene (0)
class ColorSelectManager : public Component
{
public:
	//! Delay at the beginning of the scene
	float Delay = 3.0f;
	
	float SelectTime = 3.0f;

	ColorSelector* Selector;
	//! Used to remind player
	TextComponent* CounterText;

	//! 2 for each side
	int SelectTimes = 4;

	int CurrentIndex = 0;

	// Right Color Space
	static cv::Scalar RightMaxColor;
	static cv::Scalar RightMinColor;
	static cv::Scalar RightAvgColor;

	// Left Color Space
	static cv::Scalar LeftMaxColor;
	static cv::Scalar LeftMinColor;
	static cv::Scalar LeftAvgColor;

	bool IsFinished = false;

public:
	ColorSelectManager(const std::vector<VECTOR::vector2f>& _rightPos, const std::vector<VECTOR::vector2f>& _leftPos);

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override {}


private:
	std::vector<VECTOR::vector2f> m_RightTargets;
	std::vector<VECTOR::vector2f> m_LeftTargets;


	void loadTitleScene();
};

//! Component that manages title scene (1)
class TitleManager : public Component
{
public:
	enum TitleState
	{
		Preparing,
		Selecting,
		Fading,
		Loading
	};

	enum SelectState
	{
		EagleState,
		ResetState,
		RockState,
		NothingState
	};

	TitleState State = Preparing;
	SelectState SelectAt = NothingState;

	float Delay = 1.0f;
	BallTrackerComponent* LeftCursor;
	BallTrackerComponent* RightCursor;
	CircleIndicator* EdgeCircle;
	CircleIndicator* FillCircle;

	TextComponent* InformationText;
	ImageLerper* EagleImage;
	ImageLerper* ResetImage;
	ImageLerper* RockImage;

	ImageLerper* selectedImage = nullptr;

	std::string EagleInform = "Eagle Simulator";
	std::string RockInform = "Rock Roller";
	std::string ResetInform = "Reselect Color";
	std::string NothingInform = "Rotate And Stay";

	float CurrentDegree = 0.0f;

	float SelectTime = 3.0f;

	float EagleDegree = 0;
	float ResetDegree = 70;
	float RockDegree = 110;

	float FadeTime = 0.5f;

	cv::Scalar hideColor = cv::Scalar(50, 50, 50, 255);
	cv::Scalar showColor = cv::Scalar(255, 255, 255, 255);

private:
	float m_delayTimer;
	float m_selectTimer;

	SelectState currentSelectAt(float degree)
	{
		if (degree > 0.0f)
		{
			if (degree > RockDegree)
				return RockState;
			else if (degree > ResetDegree)
				return ResetState;
			else if (degree > EagleDegree)
				return EagleState;
		}
		return NothingState;
	}

public:
	TitleManager();

	void Start() override {}
	void Update(float timeStep) override;
	void LateUpdate(float timeStep) override;
	
};

#endif
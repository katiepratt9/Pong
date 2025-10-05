#pragma once
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_ttf.h"
#include <fstream>
#include <string>
#include <iostream>
using namespace std;

struct Vector2
{
	float x;
	float y;
};

class Game
{
public:
	Game();
	bool Initialize();
	void RunLoop();
	void Shutdown();
private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	SDL_Window* mWindow;
	SDL_Texture* newTexture;
	SDL_Texture* newTextureB;
	SDL_Renderer* mRenderer;
	TTF_Font* theFont;
	SDL_Color textColor;
	SDL_Surface* textSurface;
	SDL_Surface* textSurfaceB;
	string textureText;
	string textureTextB;
	ofstream outFile;
	ifstream inFile;
	Uint32 mTicksCount;
	Mix_Chunk* boing;
	Mix_Music* music;
	bool mIsRunning;
	int mPaddleDiry;
	int mPaddleDirx;
	int score; 
	int bestScore;
	int mWidth;
	int mHeight;
	int mWidthB;
	int mHeightB;
	int scoreX; 
	int scoreY;
	int scoreBY;
	Vector2 mPaddlePos;
	Vector2 mBallPos;
	Vector2 mBallVel;
};

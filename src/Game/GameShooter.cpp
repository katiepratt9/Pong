
#include "GameShooter.h"

//Need my guy can shoot in the direction of the pressed arrow keys, when hit ball makes it dissapear
//every 20 secs makes new ball with different color 

const int thickness = 15;
const float paddleH = 30.0f;
using namespace std;

Game::Game()
{
	mWindow=nullptr;
	mRenderer=nullptr;
	newTexture = NULL;
	mTicksCount=0;
	mIsRunning=true;
	score = 0;
	mPaddleDiry=0;
	mPaddleDirx = 0;
	boing = NULL;
	music = NULL;
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}

bool Game::Initialize()
{
	int sdlResult = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}
	
	mWindow = SDL_CreateWindow(
		"CMPT 1267", // Window title
		100,	// Top left x-coordinate of window
		100,	// Top left y-coordinate of window
		1024,	// Width of window
		768,	// Height of window
		0		// Flags (0 for no flags set)
	);

	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}
	mRenderer = SDL_CreateRenderer(
		mWindow, // Window to create renderer for
		-1,		 // Usually -1
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}
	TTF_Init();
	theFont = TTF_OpenFont("Dubai-Regular.ttf", 28);
	textColor = { 0, 0, 0 };
	inFile.open("bestScore.txt");
	inFile >> bestScore;
	inFile.close();
	mPaddlePos.x = thickness*3;
	mPaddlePos.y = 768.0f/2.0f;
	mBallPos.x = 1024.0f/2.0f;
	mBallPos.y = 768.0f/2.0f;
	mBallVel.x = -200.0f;
	mBallVel.y = 235.0f;
	boing = Mix_LoadWAV("Spring-Boing.wav");
	music = Mix_LoadMUS("music.wav");
	Mix_PlayMusic(music, -1);
	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
		{
			mIsRunning = false;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_z:
				if (Mix_PlayingMusic() == 0)
				{
					Mix_PlayMusic(music, -1);
				}
				else
				{
					if (Mix_PausedMusic() == 1)
					{
						Mix_ResumeMusic();
					}
					else
					{
						Mix_PauseMusic();
					}
				}
				break;
			}
		}
	}
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}
	mPaddleDiry = 0;
	mPaddleDirx = 0;
	if (state[SDL_SCANCODE_W])
	{
		mPaddleDiry -= 1;
	}
	if (state[SDL_SCANCODE_S])
	{
		mPaddleDiry += 1;
	}
	if (state[SDL_SCANCODE_A])
	{
		mPaddleDirx -= 1;
	}
	if (state[SDL_SCANCODE_D])
	{
		mPaddleDirx += 1;
	}
}

void Game::UpdateGame()
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16));

	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
	
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	mTicksCount = SDL_GetTicks();
	
	//collision
	if (mPaddlePos.x < mBallPos.x + thickness &&
		mPaddlePos.x + thickness > mBallPos.x &&
		mPaddlePos.y < mBallPos.y + thickness &&
		paddleH + mPaddlePos.y > mBallPos.y
		) 
	{
		mIsRunning = false;
	}
	else 
	{
		if (mPaddleDiry != 0)
		{
			mPaddlePos.y += mPaddleDiry * 300.0f * deltaTime;
			if (mPaddlePos.y < (paddleH / 2.0f + thickness))
			{
				mPaddlePos.y = paddleH / 2.0f + thickness;
			}
			else if (mPaddlePos.y > (768.0f - paddleH / 2.0f - thickness))
			{
				mPaddlePos.y = 768.0f - paddleH / 2.0f - thickness;
			}
		}

		if (mPaddleDirx != 0)
		{
			mPaddlePos.x += mPaddleDirx * 300.0f * deltaTime;
			if (mPaddlePos.x <= thickness)
			{
				mPaddlePos.x = thickness;
			}
			if (mPaddlePos.x >= (1024.0f - thickness))
			{
				//this is not clean at all need to fix 
				mPaddlePos.x = (1024.0f - (2.5f * thickness));
			}
		}

		mBallPos.x += mBallVel.x * deltaTime;
		mBallPos.y += mBallVel.y * deltaTime;


		if (mBallPos.x <= thickness || mBallPos.x >= (1024.0f - thickness) && mBallVel.x > 0.0f)
		{
			mBallVel.x *= -1.0f;
		}

		if (mBallPos.y <= thickness && mBallVel.y < 0.0f || mBallPos.y >= (768 - thickness) &&
			mBallVel.y > 0.0f)
		{
			mBallVel.y *= -1;
		}
	}
}

void Game::GenerateOutput()
{
		SDL_SetRenderDrawColor(
			mRenderer,
			0,		// R
			200,		// G 
			0,	// B
			0		// A
		);
	
	SDL_RenderClear(mRenderer);

	SDL_SetRenderDrawColor(
		mRenderer,
		250,		// R
		250,		// G 
		0,	// B
		255		// A
	);
	
	SDL_Rect wall{
		0,			// Top left x
		0,			// Top left y
		1024,		// Width
		thickness	// Height
	};
	SDL_RenderFillRect(mRenderer, &wall);
	
	wall.y = 768 - thickness;
	SDL_RenderFillRect(mRenderer, &wall);
	
	wall.x = 1024 - thickness;
	wall.y = 0;
	wall.w = thickness;
	wall.h = 1024;
	SDL_RenderFillRect(mRenderer, &wall);
	
	wall.x = 0;
	wall.y = thickness;
	wall.w = thickness;
	wall.h = 1024;
	SDL_RenderFillRect(mRenderer, &wall);

	SDL_Rect paddle{
		static_cast<int>(mPaddlePos.x),
		static_cast<int>(mPaddlePos.y - paddleH/2),
		thickness,
		static_cast<int>(paddleH)
	};
	SDL_RenderFillRect(mRenderer, &paddle);
	
	SDL_Rect ball{	
		static_cast<int>(mBallPos.x - thickness/2),
		static_cast<int>(mBallPos.y - thickness/2),
		thickness,
		thickness
	};
	SDL_RenderFillRect(mRenderer, &ball);
	//score
	textureText = "Score: " + to_string(score);
	textSurface = TTF_RenderText_Solid(theFont, textureText.c_str(), textColor);
	newTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
	mWidth = textSurface->w;
	mHeight = textSurface->h;
	scoreX = (1024 - (mWidth*2));
	scoreY = mHeight;
	SDL_Rect* clip = NULL;
	SDL_Rect renderQuad = { scoreX, scoreY, mWidth, mHeight };
	SDL_RenderCopyEx(mRenderer, newTexture, clip, &renderQuad, 0, NULL, SDL_FLIP_NONE);

	//best score
	textureTextB = "Best Score: " + to_string(bestScore);
	textSurfaceB = TTF_RenderText_Solid(theFont, textureTextB.c_str(), textColor);
	newTextureB = SDL_CreateTextureFromSurface(mRenderer, textSurfaceB);
	mWidthB = textSurfaceB->w;
	mHeightB = textSurfaceB->h;
	scoreBY = (mHeightB*2);
	SDL_Rect* clipB = NULL; 

	SDL_Rect renderQuadB = { scoreX, scoreBY, mWidthB, mHeightB};
	SDL_RenderCopyEx(mRenderer, newTextureB, clipB, &renderQuadB, 0, NULL, SDL_FLIP_NONE);

	SDL_RenderPresent(mRenderer);
}

void Game::Shutdown()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}
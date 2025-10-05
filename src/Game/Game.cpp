
#include "Game.h"



const int thickness = 15;
const float paddleH = 30.0f;
using namespace std;
enum class GameState
{
	StartScreen,
	Playing,
	GameOver
};
GameState mGameState;
Game::Game()
{
	mGameState = GameState::StartScreen;
	mWindow=nullptr;
	mRenderer=nullptr;
	newTexture = nullptr;
	newTexture = nullptr;
	newTextureB = nullptr;
	textSurface = nullptr;
	textSurfaceB = nullptr;
	theFont = nullptr;
	mTicksCount=0;
	mIsRunning=true;
	score = 0;
	mPaddleDiry=0;
	mPaddle2Diry = 0;
	boing = nullptr;
	music = nullptr;
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
		"Pong", // Window title
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
	mPaddle2Pos.x = 1024.0f - (thickness * 4);
	mPaddle2Pos.y = 768.0f / 2.0f;
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
		switch (mGameState)
		{
		case GameState::StartScreen:
			ShowStartScreen();
			break;
		case GameState::Playing:
			ProcessInput();
			UpdateGame();
			GenerateOutput();
			break;
		case GameState::GameOver:
			ShowEndScreen();
			break;
		}
	}
}

void Game::UpdateBallForScreens(float deltaTime)
{
	// Update ball position
	mBallPos.x += mBallVel.x * deltaTime;
	mBallPos.y += mBallVel.y * deltaTime;

	// Bounce on walls
	if (mBallPos.y <= thickness && mBallVel.y < 0.0f)
		mBallVel.y *= -1;
	else if (mBallPos.y >= (768 - thickness) && mBallVel.y > 0.0f)
		mBallVel.y *= -1;

	if (mBallPos.x <= thickness && mBallVel.x < 0.0f)
		mBallVel.x *= -1;
	else if (mBallPos.x >= (1024 - thickness) && mBallVel.x > 0.0f)
		mBallVel.x *= -1;
}

void Game::RenderBallOnly()
{
	SDL_SetRenderDrawColor(mRenderer, 250, 250, 0, 255);
	SDL_Rect ball{
		static_cast<int>(mBallPos.x - thickness / 2),
		static_cast<int>(mBallPos.y - thickness / 2),
		thickness,
		thickness
	};
	SDL_RenderFillRect(mRenderer, &ball);
}

void Game::ShowStartScreen()
{
	SDL_Event e;
	bool waiting = true;

	// Clear screen
	SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
	SDL_RenderClear(mRenderer);

	// Texts
	SDL_Color white = { 255, 255, 255 };

	string instructions[] = {
		"Welcome to Pong!",
		"Player 1: W/S to move",
		"Player 2: Up/Down Arrows to move",
		"Press Z to pause/resume music",
		"Press ESC anytime to quit",
		"Press ENTER to start"
	};

	int yOffset = 100;
	for (string& line : instructions)
	{
		SDL_Surface* surface = TTF_RenderText_Solid(theFont, line.c_str(), white);
		SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);

		int w = surface->w;
		int h = surface->h;
		SDL_Rect rect = { (1024 - w) / 2, yOffset, w, h };
		SDL_RenderCopy(mRenderer, texture, NULL, &rect);

		yOffset += h + 20;

		SDL_FreeSurface(surface);
		SDL_DestroyTexture(texture);
	}

	SDL_RenderPresent(mRenderer);

	Uint32 lastTick = SDL_GetTicks();

	while (waiting)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				mIsRunning = false;
				waiting = false;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_RETURN)
				{
					mGameState = GameState::Playing;
					//reset ball to center
					mBallPos.x = 1024.0f / 2.0f;
					mBallPos.y = 768.0f / 2.0f;
					mBallVel.x = -200.0f;
					mBallVel.y = 235.0f;
					waiting = false;
				}
				else if (e.key.keysym.sym == SDLK_ESCAPE)
				{
					mIsRunning = false;
					waiting = false;
				}
			}
		}

		// Timing
		Uint32 currentTick = SDL_GetTicks();
		float deltaTime = (currentTick - lastTick) / 1000.0f;
		lastTick = currentTick;

		// move ball
		UpdateBallForScreens(deltaTime);

		// Clear screen
		SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
		SDL_RenderClear(mRenderer);

		// Render ball in background
		RenderBallOnly();

		// Draw text again
		SDL_Color white = { 255, 255, 255 };
		string instructions[] = {
			"Welcome to Pong!",
			"Player 1: W/S to move",
			"Player 2: Up/Down Arrows to move",
			"Press Z to pause/resume music",
			"Press ESC anytime to quit",
			"Press ENTER to start"
		};

		int yOffset = 100;
		for (string& line : instructions)
		{
			SDL_Surface* surface = TTF_RenderText_Solid(theFont, line.c_str(), white);
			SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
			int w = surface->w;
			int h = surface->h;
			SDL_Rect rect = { (1024 - w) / 2, yOffset, w, h };
			SDL_RenderCopy(mRenderer, texture, NULL, &rect);
			yOffset += h + 20;
			SDL_FreeSurface(surface);
			SDL_DestroyTexture(texture);
		}

		SDL_RenderPresent(mRenderer);
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
	mPaddle2Diry = 0;
	if (state[SDL_SCANCODE_W])
	{
		mPaddleDiry -= 1;
	}
	if (state[SDL_SCANCODE_S])
	{
		mPaddleDiry += 1;
	}
	if (state[SDL_SCANCODE_UP])
	{
		mPaddle2Diry -= 1;
	}
	if (state[SDL_SCANCODE_DOWN])
	{
		mPaddle2Diry += 1;
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
	if (mPaddle2Diry != 0)
	{
		mPaddle2Pos.y += mPaddle2Diry * 300.0f * deltaTime;
		if (mPaddle2Pos.y < (paddleH / 2.0f + thickness))
		{
			mPaddle2Pos.y = paddleH / 2.0f + thickness;
		}
		else if (mPaddle2Pos.y > (768.0f - paddleH / 2.0f - thickness))
		{
			mPaddle2Pos.y = 768.0f - paddleH / 2.0f - thickness;
		}
	}

		// Update ball position based on ball velocity
		mBallPos.x += mBallVel.x * deltaTime;
		mBallPos.y += mBallVel.y * deltaTime;

		// Bounce if needed
		// Did we intersect with the paddle1?
		float diff = mPaddlePos.y - mBallPos.y;
		diff = (diff > 0.0f) ? diff : -diff;
		// Did we intersect with the paddle2?
		float diff2 = mPaddle2Pos.y - mBallPos.y;
		diff2 = (diff2 > 0.0f) ? diff2 : -diff2;
		// Check for collision with the paddle
		if (diff <= paddleH / 2.0f &&                            // Y overlap
			mBallPos.x - thickness / 2 <= mPaddlePos.x + thickness && // X overlap
			mBallPos.x >= mPaddlePos.x &&                        // Ball to the right of paddle
			mBallVel.x < 0.0f)                                   // Ball moving left
		{
			mBallVel.x *= -1.0f;  // Bounce horizontally
			Mix_PlayChannel(-1, boing, 0);  // Play boing sound
			score++;  // Increment score

			// Update best score if needed
			if (score > bestScore)
			{
				bestScore = score;
				ofstream outFile("bestScore.txt");
				outFile << bestScore;
				outFile.close();
			}
		}
		else if (diff2 <= paddleH / 2.0f &&                            // Y overlap
			mBallPos.x + thickness / 2 >= mPaddle2Pos.x && // X overlap
			mBallPos.x >= mPaddle2Pos.x &&                        // Ball to the right of paddle
			mBallVel.x > 0.0f)                                   // Ball moving left
		{
			mBallVel.x *= -1.0f;  // Bounce horizontally
			Mix_PlayChannel(-1, boing, 0);  // Play boing sound
			score++;  // Increment score

			// Update best score if needed
			if (score > bestScore)
			{
				bestScore = score;
				ofstream outFile("bestScore.txt");
				outFile << bestScore;
				outFile.close();
			}
		}
		// Did the ball collide with left wall? (if so, end game)
		else if (mBallPos.x <= 0.0f)
		{
			mGameState = GameState::GameOver;
		}
		// Did the ball collide with the right wall?
		else if (mBallPos.x >= (1024.0f - thickness) && mBallVel.x > 0.0f)
		{
			mGameState = GameState::GameOver;
		}

		// Did the ball collide with the top wall?
		if (mBallPos.y <= thickness && mBallVel.y < 0.0f)
		{
			mBallVel.y *= -1;
		}
		// Did the ball collide with the bottom wall?
		else if (mBallPos.y >= (768 - thickness) &&
			mBallVel.y > 0.0f)
		{
			mBallVel.y *= -1;
		}
	}


	void Game::GenerateOutput()
	{
		SDL_SetRenderDrawColor(mRenderer, 0, 200, 0, 0);
		SDL_RenderClear(mRenderer);

		// Draw walls
		SDL_SetRenderDrawColor(mRenderer, 250, 250, 0, 255);
		SDL_Rect wall{ 0, 0, 1024, thickness };
		SDL_RenderFillRect(mRenderer, &wall);

		wall.y = 768 - thickness;
		SDL_RenderFillRect(mRenderer, &wall);

		wall.x = 1024 - thickness;
		wall.y = 0;
		wall.w = thickness;
		wall.h = 768;
		SDL_RenderFillRect(mRenderer, &wall);

		wall.x = 0;
		wall.y = thickness;
		wall.w = thickness;
		wall.h = 768;
		SDL_RenderFillRect(mRenderer, &wall);

		// Draw paddles
		SDL_Rect paddle{
			static_cast<int>(mPaddlePos.x),
			static_cast<int>(mPaddlePos.y - paddleH / 2),
			thickness,
			static_cast<int>(paddleH)
		};
		SDL_RenderFillRect(mRenderer, &paddle);

		SDL_Rect paddle2{
			static_cast<int>(mPaddle2Pos.x),
			static_cast<int>(mPaddle2Pos.y - paddleH / 2),
			thickness,
			static_cast<int>(paddleH)
		};
		SDL_RenderFillRect(mRenderer, &paddle2);

		// Draw ball
		SDL_Rect ball{
			static_cast<int>(mBallPos.x - thickness / 2),
			static_cast<int>(mBallPos.y - thickness / 2),
			thickness,
			thickness
		};
		SDL_RenderFillRect(mRenderer, &ball);

		// Destroy previous textures
		if (newTexture)
		{
			SDL_DestroyTexture(newTexture);
			newTexture = nullptr;
		}
		if (newTextureB)
		{
			SDL_DestroyTexture(newTextureB);
			newTextureB = nullptr;
		}

		// Score Text
		textureText = "Score: " + to_string(score);
		textSurface = TTF_RenderText_Solid(theFont, textureText.c_str(), textColor);
		newTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
		mWidth = textSurface->w;
		mHeight = textSurface->h;
		scoreX = (1024 - (mWidth * 2));
		scoreY = mHeight;
		SDL_Rect renderQuad = { scoreX, scoreY, mWidth, mHeight };
		SDL_RenderCopyEx(mRenderer, newTexture, NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
		SDL_FreeSurface(textSurface);

		// Best Score Text
		textureTextB = "Best Score: " + to_string(bestScore);
		textSurfaceB = TTF_RenderText_Solid(theFont, textureTextB.c_str(), textColor);
		newTextureB = SDL_CreateTextureFromSurface(mRenderer, textSurfaceB);
		mWidthB = textSurfaceB->w;
		mHeightB = textSurfaceB->h;
		scoreBY = (mHeightB * 2);
		SDL_Rect renderQuadB = { scoreX, scoreBY, mWidthB, mHeightB };
		SDL_RenderCopyEx(mRenderer, newTextureB, NULL, &renderQuadB, 0, NULL, SDL_FLIP_NONE);
		SDL_FreeSurface(textSurfaceB);

		SDL_RenderPresent(mRenderer);
	}

	void Game::ShowEndScreen()
	{
		SDL_Event e;
		bool waiting = true;

		// Determine the winner based on ball direction
		string result;
		if (mBallVel.x < 0)
			result = "Player 2 Wins!";
		else
			result = "Player 1 Wins!";

		string finalScore = "Score: " + to_string(score);
		string best = "Best Score: " + to_string(bestScore);
		string restartPrompt = "Press ENTER to restart";
		string exitPrompt = "Press ESC to quit";

		string lines[] = { result, finalScore, best, restartPrompt, exitPrompt };

		Uint32 lastTick = SDL_GetTicks();

		while (waiting)
		{
			// Process events
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
				{
					mIsRunning = false;
					waiting = false;
				}
				else if (e.type == SDL_KEYDOWN)
				{
					if (e.key.keysym.sym == SDLK_ESCAPE)
					{
						mIsRunning = false;
						waiting = false;
					}
					else if (e.key.keysym.sym == SDLK_RETURN)
					{
						// Reset game state for restart
						score = 0;
						mBallPos.x = 1024.0f / 2.0f;
						mBallPos.y = 768.0f / 2.0f;
						mBallVel.x = -200.0f;
						mBallVel.y = 235.0f;

						// Reset paddles to center
						mPaddlePos.y = 768.0f / 2.0f;
						mPaddle2Pos.y = 768.0f / 2.0f;

						mGameState = GameState::Playing;
						waiting = false;
					}
				}
			}

			// Timing for smooth animation
			Uint32 currentTick = SDL_GetTicks();
			float deltaTime = (currentTick - lastTick) / 1000.0f;
			lastTick = currentTick;

			// Animate ball
			UpdateBallForScreens(deltaTime);

			// Clear screen
			SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
			SDL_RenderClear(mRenderer);

			// Render the ball (use your existing render function)
			RenderBallOnly();

			// Draw end screen text
			SDL_Color white = { 255, 255, 255 };
			int yOffset = 150;

			for (string& line : lines)
			{
				SDL_Surface* surface = TTF_RenderText_Solid(theFont, line.c_str(), white);
				SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);

				int w = surface->w;
				int h = surface->h;
				SDL_Rect rect = { (1024 - w) / 2, yOffset, w, h };
				SDL_RenderCopy(mRenderer, texture, NULL, &rect);

				yOffset += h + 30;

				SDL_FreeSurface(surface);
				SDL_DestroyTexture(texture);
			}

			SDL_RenderPresent(mRenderer);
		}
	}


void Game::Shutdown()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}
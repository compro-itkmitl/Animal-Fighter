/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, vectors, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
//A circle stucture
struct Circle
{
	int x, y;
	int r;
};

//Texture wrapper class
class LTexture
{
public:
	//Initializes variables
	LTexture();

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path);


	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
	void render2(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
	//Gets image dimensions
	int getWidth();
	int getHeight();

private:
	//The actual hardware texture
	SDL_Texture * mTexture;

	//Image dimensions
	int mWidth;
	int mHeight;
};

//The dot that will move around on the screen
class Dot
{
public:
	//The dimensions of the dot
	static const int DOT_WIDTH = 20;
	static const int DOT_HEIGHT = 20;
	static const int count999 = 0;
	//Maximum axis velocity of the dot
	float DOT_VEL = 0.3;

	//Initializes the variables
	Dot(float x, float y);
	//Takes key presses and adjusts the dot's velocity
	void handleEvent(SDL_Event& e);

	//Moves the dot and checks collision
	void move(SDL_Rect& square, Circle& circle);
	int move2(SDL_Rect& square, Circle& circle);
	//Shows the dot on the screen
	void render();
	void render2();
	//Gets collision circle
	Circle& getCollider();



private:
	//The X and Y offsets of the dot
	float mPosX, mPosY;
	//The velocity of the dot
	float mVelX, mVelY;

	//Dot's collision circle
	Circle mCollider;

	//Moves the collision circle relative to the dot's offset
	void shiftColliders();
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();
bool loadMedia2();
//Frees media and shuts down SDL
void close();

//Circle/Circle collision detector
bool checkCollision(Circle& a, Circle& b);

//Circle/Box collision detector
bool checkCollision(Circle& a, SDL_Rect& b);

//Calculates distance squared between two points
double distanceSquared(int x1, int y1, int x2, int y2);
//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gDotTexture;
LTexture gBGTexture;
LTexture Oreo;
LTexture mon;
LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Color key image
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}


void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if (clip != NULL)
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}
int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

Dot::Dot(float x, float y)
{
	//Initialize the offsets
	mPosX = x;
	mPosY = y;

	//Set collision circle size
	mCollider.r = DOT_WIDTH / 2;

	//Initialize the velocity
	mVelX = 0;
	mVelY = 0;

	//Move collider relative to the circle
	shiftColliders();
}
void Dot::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY -= DOT_VEL; break;
		case SDLK_DOWN: mVelY += DOT_VEL; break;
		case SDLK_LEFT: mVelX -= DOT_VEL; break;
		case SDLK_RIGHT: mVelX += DOT_VEL; break;
		}
	}
	//If a key was released
	else if (e.type == SDL_KEYUP && e.key.repeat == 0)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
		case SDLK_UP: mVelY += DOT_VEL; break;
		case SDLK_DOWN: mVelY -= DOT_VEL; break;
		case SDLK_LEFT: mVelX += DOT_VEL; break;
		case SDLK_RIGHT: mVelX -= DOT_VEL; break;
		}
	}
}
int y = 0;
void Dot::move(SDL_Rect& square, Circle& circle)
{
	int x = 0;
	//Move the dot left or right
	mPosX += mVelX;
	shiftColliders();

	//If the dot collided or went too far to the left or right
	if ((mPosX - mCollider.r < 0) || (mPosX + mCollider.r > SCREEN_WIDTH) || checkCollision(mCollider, square) || checkCollision(mCollider, circle))
	{
		//Move back
		mPosX -= mVelX + 10;
		x++;
		y++;
		if (mPosX <= 0) {
			mPosX = 15;
		}
		/*if (y >= 3) {
			printf("gameover");
		}*/
		shiftColliders();
	}

	//Move the dot up or down
	mPosY += mVelY;
	shiftColliders();
	//If the dot collided or went too far up or down
	if ((mPosY - mCollider.r < 0) || (mPosY + mCollider.r > SCREEN_HEIGHT) || checkCollision(mCollider, square) || checkCollision(mCollider, circle))
	{
		//Move back
		mPosY -= mVelY + 10;
		x++;
		y++;
		if (mPosY <= 0) {
			mPosY = 15;
		}
		/*if (y >= 3) {
			printf("gameover");
		}*/
		shiftColliders();
	}
	
}
int Dot::move2(SDL_Rect& square, Circle& circle)
{
	int x = 0;
	//Move the dot left or right
	mPosX += mVelX;
	shiftColliders();
	//If the dot collided or went too far to the left or right
	if ((mPosX - mCollider.r < 0) || (mPosX + mCollider.r > SCREEN_WIDTH))
	{
		//Move back
		mPosX -= mVelX + 10;
		y++;
		if (mPosX <= 0) {
			mPosX = 15;
		}
		/*if (y >= 3) {
		printf("gameover");
		}*/
		shiftColliders();
	}
	if (checkCollision(mCollider, square) || checkCollision(mCollider, circle)) {
		x = 1;
	}
	//Move the dot up or down
	mPosY += mVelY;
	shiftColliders();
	//If the dot collided or went too far up or down
	if ((mPosY - mCollider.r < 0)||(mPosY + mCollider.r > SCREEN_HEIGHT))
	{
		//Move back
		mPosY -= mVelY + 10;
		y++;
		if (mPosY <= 0) {
			mPosY = 15;
		}
		/*if (y >= 3) {
		printf("gameover");
		}*/
		shiftColliders();
	}
	return x;
}
void Dot::render()
{
	//Show the dot
	gDotTexture.render(mPosX - mCollider.r, mPosY - mCollider.r);
}
void Dot::render2()
{
	//Show the dot
	mon.render(mPosX - mCollider.r, mPosY - mCollider.r);
}
Circle& Dot::getCollider()
{
	return mCollider;
}

void Dot::shiftColliders()
{
	//Align collider to center of dot
	mCollider.x = mPosX;
	mCollider.y = mPosY;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}
const int WALKING_ANIMATION_FRAMES = 4;
SDL_Rect gSpriteClips[WALKING_ANIMATION_FRAMES];
bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load dot texture
	if (!mon.loadFromFile("animal/pig.png"))
	{
		printf("Failed to load walking animation texture!\n");
		success = false;
	}
	else
	{
		//Set sprite clips
		gSpriteClips[0].x = 0;
		gSpriteClips[0].y = 0;
		gSpriteClips[0].w = 37;
		gSpriteClips[0].h = 37;

		gSpriteClips[1].x = 0;
		gSpriteClips[1].y = 37;
		gSpriteClips[1].w = 37;
		gSpriteClips[1].h = 37;

		gSpriteClips[2].x = 0;
		gSpriteClips[2].y = 74;
		gSpriteClips[2].w = 37;
		gSpriteClips[2].h = 37;

		gSpriteClips[3].x = 0;
		gSpriteClips[3].y = 111;
		gSpriteClips[3].w = 37;
		gSpriteClips[3].h = 37;
	}
	if (!gDotTexture.loadFromFile("animal/dot.bmp"))
	{
		printf("Failed to load dot texture!\n");
		success = false;
	}
	//Load dot texture
	if (!Oreo.loadFromFile("animal/dot.bmp"))
	{
		printf("Failed to load dot texture!\n");
		success = false;
	}

	//Load background texture
	if (!gBGTexture.loadFromFile("backgrounds/bg3.png"))
	{
		printf("Failed to load background texture!\n");
		success = false;
	}
	return success;
}

//Walking animation
void close()
{
	//Free loaded images
	gDotTexture.free();

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision(Circle& a, Circle& b)
{
	//Calculate total radius squared
	int totalRadiusSquared = a.r + b.r;
	totalRadiusSquared = totalRadiusSquared * totalRadiusSquared;

	//If the distance between the centers of the circles is less than the sum of their radii
	if (distanceSquared(a.x, a.y, b.x, b.y) < (totalRadiusSquared))
	{
		//The circles have collided
		return true;
	}

	//If not
	return false;
}

bool checkCollision(Circle& a, SDL_Rect& b)
{
	//Closest point on collision box
	int cX, cY;

	//Find closest x offset
	if (a.x < b.x)
	{
		cX = b.x;
	}
	else if (a.x > b.x + b.w)
	{
		cX = b.x + b.w;
	}
	else
	{
		cX = a.x;
	}

	//Find closest y offset
	if (a.y < b.y)
	{
		cY = b.y;
	}
	else if (a.y > b.y + b.h)
	{
		cY = b.y + b.h;
	}
	else
	{
		cY = a.y;
	}

	//If the closest point is inside the circle
	if (distanceSquared(a.x, a.y, cX, cY) < a.r * a.r)
	{
		//This box and the circle have collided
		return true;
	}

	//If the shapes have not collided
	return false;
}

double distanceSquared(int x1, int y1, int x2, int y2)
{
	int deltaX = x2 - x1;
	int deltaY = y2 - y1;
	return deltaX * deltaX + deltaY * deltaY;
}
int main(int argc, char* args[])
{	//Start up SDL and create window
	int count = 0, count2 = 0, count3 = 0, count4 = 2, level = 2, value = 0, count5 = 4 ,frame = 0,time=0,time1=-1,time2=-1,damage = 0,bossmove=2;
	struct aa {
		SDL_Rect a{ 150,0,30,30 };
		SDL_Rect a1{ 200,100,30,30 };
		SDL_Rect a2{ 350,225,30,30 };
		SDL_Rect a3{ 470,350,30,30 };
		SDL_Rect a4{ 580,450,30,30 };
		SDL_Rect ha{ 680,0,30,30 };
		SDL_Rect h1{ 680,50,30,30 };
		SDL_Rect h2{ 680,110,30,30 };
		SDL_Rect h3{ 680,180,30,30 };
		SDL_Rect h4{ 680,260,30,30 };
		SDL_Rect h5{ 680,300,30,30 };
		SDL_Rect h6{ 680,380,30,30 };
		SDL_Rect h7{ 680,410,30,30 };
		SDL_Rect boss{ 590,50,50,400 };
	}a[99];
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		//Load media
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		else
		{
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//The background scrolling offset
			int scrollingOffset = 0;

			//The dot that will be moving around on the screen
			Dot dot(Dot::DOT_WIDTH / 2, Dot::DOT_HEIGHT / 2);
			Dot otherDot(3000,3000);
			//While application is running
			a[count + 1].a4.y = 30;
			a[count + 1].h6.y = 65;
			a[count + 2].a4.y = 150;
			a[count + 3].a4.y = 30;
			a[count + 3].h6.y = 65;
			a[count + 4].a4.y = 150;
			a[count + 3].a4.x = 900;
			a[count + 3].h6.x = 900;
			a[count + 4].a4.x = 900;
			a[count + 5].a4.y = 210;
			a[count + 5].h6.y = 250;
			a[count + 6].a4.y = 280;
			a[count + 5].a4.x = 900;
			a[count + 5].h6.x = 900;
			a[count + 6].a4.x = 900;
			a[count + 7].a4.x = 500;
			while (!quit)
			{
				//Handle events on queue
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}

					//Handle input for the dot
					dot.handleEvent(e);
				}
				//boss
				
				a[0].boss.y -= bossmove;
				if (a[0].boss.y <= 3)
				{
					bossmove = -2;
				}
				else if (a[0].boss.y == 80)
				{
					bossmove = 2;
				}
				//
				a[count].a.x -= level;
				a[count].a1.x -= level;
				a[count].a2.x -= level;
				a[count].a3.x -= level;
				a[count].a4.x -= level;
				a[count].h3.x -= level;
				a[count].ha.x -= level;
				a[count + 1].a4.x -= level;
				a[count + 1].h6.x -= level;
				a[count + 2].a4.x -= level;
				a[count + 3].a4.x -= level;
				a[count + 3].h6.x -= level;
				a[count + 4].a4.x -= level;
				a[count + 5].a4.x -= level;
				a[count + 5].h6.x -= level;
				a[count + 6].a4.x -= level;
				//Scroll background
				--scrollingOffset;
				if (scrollingOffset < -gBGTexture.getWidth())
				{
					scrollingOffset = 0;
				}
				//Move the dot and check collision
				dot.move(a[count].boss, otherDot.getCollider());
				dot.move(a[count].a, otherDot.getCollider());
				dot.move(a[count].a1, otherDot.getCollider());
				dot.move(a[count].a2, otherDot.getCollider());
				dot.move(a[count].a3, otherDot.getCollider());
				dot.move(a[count].a4, otherDot.getCollider());
				dot.move(a[count].ha, otherDot.getCollider());
				dot.move(a[count].h3, otherDot.getCollider());
				dot.move(a[count+1].a4, otherDot.getCollider());
				dot.move(a[count+1].h6, otherDot.getCollider());
				dot.move(a[count+2].a4, otherDot.getCollider());
				dot.move(a[count + 3].a4, otherDot.getCollider());
				dot.move(a[count + 3].h6, otherDot.getCollider());
				dot.move(a[count + 4].a4, otherDot.getCollider());
				dot.move(a[count + 5].a4, otherDot.getCollider());
				dot.move(a[count + 5].h6, otherDot.getCollider());
				dot.move(a[count + 6].a4, otherDot.getCollider());

				//Level
				if (count2 >2&& count2 < 5) {
					level = 4;
				}
				if (count2 > 5) {
					level = 7;
				}
				//This is a Bomb
				if (count2%4==0){
					time = 1;
				}
				if (count2 % 4 == 1) {
					time1 = 1;
				}
				if (count2 % 4 == 3) {
					time2 = 1;
				}
				if (time >= 0) {

					a[count + 6].a.x -= 0;
					a[count + 6].a1.x -= 0;
					dot.move2(a[count+6].a, otherDot.getCollider());
					if (dot.move2(a[count + 6].a, otherDot.getCollider()) == 1) {
						a[count + 6].a.x = 5000;
						a[count + 6].a.y = 5000;
						damage++;
					}
					dot.move2(a[count+6].a1, otherDot.getCollider());
					if (dot.move2(a[count + 6].a1, otherDot.getCollider()) == 1) {
						a[count + 6].a1.x = 5000;
						a[count + 6].a1.y = 5000;
						damage++;
					}
				}
				printf("%d", damage);
				if (time1 >= 0) {
					a[count + 6].a2.x -= 0;
					dot.move2(a[count+6].a2, otherDot.getCollider());
					if (dot.move2(a[count + 6].a2, otherDot.getCollider()) == 1) {
						a[count + 6].a2.x = 5000;
						a[count + 6].a2.y = 5000;
						damage++;
					}
				}
				if (time2 >= 0) {
					a[count + 6].a3.x -= 0;
					a[count + 7].a4.x -= 0;
					dot.move2(a[count + 6].a3, otherDot.getCollider());
					if (dot.move2(a[count + 6].a3, otherDot.getCollider()) == 1) {
						a[count + 6].a3.x = 5000;
						a[count + 6].a3.y = 5000;
						damage++;
					}
					dot.move2(a[count + 7].a4, otherDot.getCollider());
					if (dot.move2(a[count + 7].a4, otherDot.getCollider()) == 1) {
						a[count + 7].a4.x = 5000;
						a[count + 7].a4.y = 5000;
						damage++;
					}
				}
				//kill boss
				if (damage == 4) {
					a[0].boss.x = 5000;
				}
				//
				if (a[count].a.x < -30) {
					count2++;
					time = -1;
					time1 = -1;
					time2 = -1;
					a[count].a.x =	1400;
				}
				if (a[count].a1.x < -30) {
					a[count].a1.x = 680;
				}
				if (a[count].a2.x < -30) {
					a[count].a2.x = 680;
				}
				if (a[count].a3.x < -30) {
					a[count].a3.x = 680;
				}
				if (a[count].a4.x < -30) {
					a[count].a4.x = 680;
				}
				if (a[count].ha.x < -30) {
					a[count].ha.x = 680;
				}
				if (a[count].h3.x < -30) {
					a[count].h3.x = 680;
				}
				if (a[count+1].a4.x < -30) {
					a[count+1].a4.x = 1200;
				}
				if (a[count+1].h6.x < -30) {
					a[count+1].h6.x = 1400;
				}
				if (a[count+2].a4.x < -30) {
					a[count+2].a4.x = 2000;
				}
				if (a[count + 3].a4.x < -30) {
					a[count + 3].a4.x = 3000;
				}
				if (a[count + 3].h6.x < -30) {
					a[count + 3].h6.x = 3000;
				}
				if (a[count + 4].a4.x < -30) {
					a[count + 4].a4.x = 3000;
				}
				if (a[count + 5].a4.x < -30) {
					a[count + 5].a4.x = 3800;
				}
				if (a[count + 5].h6.x < -30) {
					a[count + 5].h6.x = 3800;
				}
				if (a[count + 6].a4.x < -30) {
					a[count + 6].a4.x = 3800;
				}
				if (count2 > 3) {
					a[count].h5.x -= count4;
					a[count].h6.x -= count4;
					a[count].h7.x -= count4;
					dot.move(a[count].h5, otherDot.getCollider());
					dot.move(a[count].h6, otherDot.getCollider());
					dot.move(a[count].h7, otherDot.getCollider());
					if (a[count].a.x < -30) {
						count2++;
						a[count].h5.x = 2100;
					}
					if (a[count].h6.x < -30) {
						a[count].h6.x = 1300;
					}
					if (a[count].h7.x < -30) {
						a[count].h7.x = 1300;
					}
				}
				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);

				//Render background
				gBGTexture.render(scrollingOffset, 0);
				gBGTexture.render(scrollingOffset + gBGTexture.getWidth(), 0);
				

				//Render wall
				//Render current frame
				SDL_Rect* currentClip = &gSpriteClips[frame / 4];
				//boss
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderDrawRect(gRenderer, &a[0].boss);
				//
				mon.render(a[count].a.x, a[count].a.y , currentClip);
				mon.render(a[count].a1.x, a[count].a1.y , currentClip);
				mon.render(a[count].a2.x, a[count].a2.y , currentClip);
				mon.render(a[count].a3.x, a[count].a3.y , currentClip);
				mon.render(a[count].a4.x, a[count].a4.y , currentClip);
				mon.render(a[count].ha.x, a[count].ha.y , currentClip);
				mon.render(a[count].h3.x, a[count].h3.y , currentClip);
				mon.render(a[count+1].a4.x, a[count+1].a4.y , currentClip);
				mon.render(a[count+1].h6.x, a[count+1].h6.y , currentClip);
				mon.render(a[count+2].a4.x, a[count+2].a4.y , currentClip);
				mon.render(a[count+3].h6.x, a[count+3].h6.y , currentClip);
				mon.render(a[count+3].a4.x, a[count+3].a4.y , currentClip);
				mon.render(a[count+4].a4.x, a[count+4].a4.y , currentClip);
				mon.render(a[count+5].h6.x, a[count+5].h6.y , currentClip);
				mon.render(a[count+5].a4.x, a[count+5].a4.y , currentClip);
				mon.render(a[count+6].a4.x, a[count+6].a4.y , currentClip);
				if (count2 > 3) {
					mon.render(a[count].h5.x, a[count].h5.y, currentClip);
					mon.render(a[count].h6.x, a[count].h6.y, currentClip);
					mon.render(a[count].h7.x, a[count].h7.y, currentClip);
				}
				if (time >= 0) {
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderDrawRect(gRenderer, &a[count+6].a);
					SDL_RenderDrawRect(gRenderer, &a[count+6].a1);
				}
				if (time1 >= 0) {
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderDrawRect(gRenderer, &a[count+6].a2);
				}
				if (time2 >= 0) {
					SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
					SDL_RenderDrawRect(gRenderer, &a[count + 6].a3);
					SDL_RenderDrawRect(gRenderer, &a[count + 7].a4);
				}
				//Go to next frame
				++frame;

				//Cycle animation
				if (frame / 4 >= WALKING_ANIMATION_FRAMES)
				{
					frame = 0;
				}
				//Render dots
				dot.render();
				otherDot.render();
				//Update screen
				SDL_RenderPresent(gRenderer);
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
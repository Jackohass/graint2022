// Introduction lab that covers:
// * C++
// * SDL
// * 2D graphics
// * Plotting pixels
// * Video memory
// * Color representation
// * Linear interpolation
// * glm::vec3 and std::vector

#include "SDL.h"
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "SDLauxiliary.h"

using namespace std;
using glm::vec3;

// --------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
SDL_Surface* screen;
vector<vec3> stars(1000);
int ticks;
float velocity = 0.0001f;

// --------------------------------------------------------
// FUNCTION DECLARATIONS

void Draw();

// --------------------------------------------------------
// FUNCTION DEFINITIONS

/*
	Fills the vector result with interpolated values between the float values a & b
	If the vector is of size 1, then we return the mean value between a & b
*/
void Interpolate(float a, float b, vector<float>& result)
{
	if (result.size() == 1) {
		result[0] = (b - a) / 2;
		return;
	}

	for (int i = 0; i < result.size(); i++)
	{
		// Add the fraction between the difference of a & b, divided by the number of steps, multiplied with current step
		result[i] = a + (b - a)/(result.size()-1) * i;
	}
}

/*
	Fills the vector result with interpolated values between the rgb values a & b
	If the vector is of size 1, then we return the mean value between a & b
*/
void Interpolate(vec3 a, vec3 b, vector<vec3>& result)
{
	if (result.size() == 1) {
		result[0].r = (b.r - a.r) / 2;
		result[0].g = (b.g - a.g) / 2;
		result[0].b = (b.b - a.b) / 2;
		return;
	}

	for (int i = 0; i < result.size(); i++)
	{
		// Add the fraction between the difference of a & b, divided by the number of steps, multiplied with current step
		result[i].r = a.r + (b.r - a.r) / (result.size() - 1) * i;
		result[i].g = a.g + (b.g - a.g) / (result.size() - 1) * i;
		result[i].b = a.b + (b.b - a.b) / (result.size() - 1) * i;
	}
}

/*
	Update the locations of the stars
*/
void update(){
	//Calculate the amount of time that has passed
	int curTick = SDL_GetTicks();
	float dT = (float) (curTick - ticks);
	ticks = curTick;

	//Update the star locations
	for(int i = 0; i < stars.size(); i++){
		//Move the star according to Equation 1
		stars[i].z -= velocity * dT;

		//Wrap the star around if outside of the drawing volume
		if(stars[i].z <= 0) stars[i].z += 1;
		if(stars[i].z > 1) stars[i].z -= 1;
	}
}

int main( int argc, char* argv[] )
{
	for (int i = 0; i < stars.size(); i++)
	{
		stars[i].x = 1 - 2 * float(rand()) / float(RAND_MAX);
		stars[i].y = 1 - 2 * float(rand()) / float(RAND_MAX);
		while ((stars[i].z = float(rand()) / float(RAND_MAX)) == 0);
	}

	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	ticks = SDL_GetTicks();
	while( NoQuitMessageSDL() )
	{
		update();
		Draw();
	}
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Draw()
{
	SDL_FillRect(screen, 0, 0);
	if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

	for (size_t i = 0; i < stars.size(); ++i)
	{
		//Calculate window coordinates from system coordinates
		float f = (float) SCREEN_HEIGHT / 2.0f;
		int u = (float) f * (stars[i].x / stars[i].z) + (SCREEN_WIDTH / 2.0f);
		int v = (float) f * (stars[i].y / stars[i].z) + (SCREEN_HEIGHT / 2.0f);

		//Calculate the colour
		vec3 colour = 0.2f * (vec3(1, 1, 1) / (stars[i].z * stars[i].z));

		PutPixelSDL(screen, u, v, colour);
	}

	if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

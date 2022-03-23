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

int main( int argc, char* argv[] )
{
	vector<float> result(10); // Create a vector width 10 floats
	Interpolate(5, 14, result); // Fill it with interpolated values
	for (int i = 0; i < result.size(); ++i)
		cout << result[i] << " "; // Print the result to the terminal
	cout << endl;

	vector<vec3> result3(4);
	vec3 a(1, 4, 9.2);
	vec3 b(4, 1, 9.8);
	Interpolate(a, b, result3);
	for (int i = 0; i < result3.size(); ++i)
	{
		cout << "( "
			<< result3[i].x << ", "
			<< result3[i].y << ", "
			<< result3[i].z << " ) ";
	}



	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	while( NoQuitMessageSDL() )
	{
		Draw();
	}
	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Draw()
{
	const vec3 topLeft(1, 0, 0); // red
	const vec3 topRight(0, 0, 1); // blue
	const vec3 bottomLeft(1, 1, 0); // yellow
	const vec3 bottomRight(0, 1, 0); // green


	vector<vec3> leftSide(SCREEN_HEIGHT);
	vector<vec3> rightSide(SCREEN_HEIGHT);

	//Interpolate twice in y-direction
	Interpolate(topLeft, bottomLeft, leftSide);
	Interpolate(topRight, bottomRight, rightSide);

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		vector<vec3> row(SCREEN_WIDTH);
		//Interpolate once in x-direction (do it for each row)
		Interpolate(leftSide[y], rightSide[y], row);
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			PutPixelSDL( screen, x, y, row[x] );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}
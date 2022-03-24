#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::mat3;

// ----------------------------------------------------------------------------
// Structs
struct Intersection
{
	vec3 position;
	float distance;
	int triangleIndex;
};


// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 500;
SDL_Surface* screen;
int tick;
float focalLength = SCREEN_HEIGHT / 2;
vec3 cameraPos(0, 0, -1.75);

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw(const vector<Triangle>& triangles);

bool CheckTriangleIntersection(
	const vec3& start,
	const vec3& dir,
	const Triangle& triangle,
	Intersection& intersection,
	float best
)
{
	vec3 e1 = triangle.v1 - triangle.v0;
	vec3 e2 = triangle.v2 - triangle.v0;
	vec3 b = start - triangle.v0;

	float detA = glm::determinant(mat3(-dir, e1, e2));
	float detA1 = glm::determinant(mat3(b, e1, e2));

	float t = detA1 / detA;

	if(t < 0 || t >= best) return false;

	float detA2 = glm::determinant(mat3(-dir, b, e2));
	float u = detA2 / detA;

	if(u < 0 || u > 1) return false;

	float detA3 = glm::determinant(mat3(-dir, e1, b));
	float v = detA3 / detA;

	if(v < 0 || u + v > 1) return false;

	//Put into intersection
	intersection.distance = t;
	intersection.position = start + t * dir;
	//NOTE: We don't set index here, we set it when it returns.
	intersection.triangleIndex = -1;
	return true;
}

bool ClosestIntersection(
	const vec3& start,
	const vec3& dir,
	const vector<Triangle>& triangles,
	Intersection& closestIntersection
)
{
	//Assume there isn't any intersections
	bool hasIntersection = false;
	Intersection intersection;
	float smallestDistance = std::numeric_limits<float>::max();

	//Go through all the triangles and check if it is the closest intersection.
	for (int i = 0; i < triangles.size(); i++)
	{
		if(CheckTriangleIntersection(start, dir, triangles[i],
						intersection, smallestDistance)){
			//We have found an intersection
			if (intersection.distance < smallestDistance)
			{
				hasIntersection = true;
				//If it is closer than the last one we checked,
				// save it and assign its triangle index.
				smallestDistance = intersection.distance;
				closestIntersection = intersection;
				closestIntersection.triangleIndex = i;
			}
		}
	}
	return hasIntersection;
}


int main( int argc, char* argv[] )
{
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );

	// Load the scene
	vector<Triangle> triangles;
	LoadTestModel(triangles);

	tick = SDL_GetTicks();	// Set start value for timer.

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw(triangles);
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update()
{
	// Compute frame time:
	int curTick = SDL_GetTicks();
	float dt = float(curTick - tick);
	tick = curTick;
	cout << "Render time: " << dt << " ms." << endl;
}

void Draw(const vector<Triangle>& triangles)
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			vec3 color(0, 0, 0);

			vec3 d(x - SCREEN_WIDTH / 2,
				y - SCREEN_HEIGHT / 2,
				focalLength
			);

			Intersection intersection;
			if(ClosestIntersection(cameraPos, d, triangles, intersection)){
				color = triangles[intersection.triangleIndex].color;
			}

			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

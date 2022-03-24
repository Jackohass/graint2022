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
int t;

// ----------------------------------------------------------------------------
// FUNCTIONS

void Update();
void Draw();

bool CheckTriangleIntersection(
	const vec3& start,
	const vec3& dir,
	const Triangle& triangle,
	Intersection& intersection
)
{
	vec3 t1 = triangle.v1 - triangle.v0;
	vec3 t2 = triangle.v2 - triangle.v0;

	vec3 normal_ = triangle.normal;
	vec3 normalized = normal_ / glm::length(normal_);

	float d = glm::dot(dir, normalized);
	float epsilon = 0.0001f;

	if (fabs(d) < epsilon) {
		return false;
	}

	float t = glm::dot((triangle.v0 - start), normalized);

	float lambda = t / d;
	if (lambda < 0 || lambda + epsilon > 1/*maxLambda?*/) {
		return false;
	}

	vec3 p = start + lambda * dir;
	double area = glm::length(normal_) / 2.0;

	double beta = glm::dot(glm::cross(t1, p - triangle.v0), normal_) / glm::dot(normal_, normal_);
	if (!(beta <= 1 && beta >= 0)) return false;
	double gamma = glm::dot(glm::cross(p - triangle.v0, t2), normal_) / glm::dot(normal_, normal_);
	if (!(gamma <= 1 && gamma >= 0)) return false;
	double alpha = 1 - gamma - beta;
	if (!(alpha <= 1 && alpha >= 0)) return false;

	intersection.distance = glm::length(p);
	intersection.position = p;
	intersection.triangleIndex = 0;
	return true;
}

bool ClosestIntersection(
	const vec3& start,
	const vec3& dir,
	const vector<Triangle>& triangles,
	Intersection& closestIntersection
)
{
	bool hasIntersection = false;
	Intersection intersection;
	float smallestDistance = std::numeric_limits<float>::max();

	for (int i = 0; i < triangles.size(); i++)
	{
		if (CheckTriangleIntersection(start, dir, triangles[i], intersection))
		{
			hasIntersection = true;
			if (intersection.distance < smallestDistance)
			{
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
	t = SDL_GetTicks();	// Set start value for timer.

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void Update()
{
	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	cout << "Render time: " << dt << " ms." << endl;
}

void Draw()
{
	if( SDL_MUSTLOCK(screen) )
		SDL_LockSurface(screen);

	for( int y=0; y<SCREEN_HEIGHT; ++y )
	{
		for( int x=0; x<SCREEN_WIDTH; ++x )
		{
			vec3 color( 1, 0.5, 0.5 );
			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}
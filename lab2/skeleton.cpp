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
	//Get the axises of the triangles.
	vec3 t1 = triangle.v1 - triangle.v0;
	vec3 t2 = triangle.v2 - triangle.v0;

	float d = glm::dot(dir, triangle.normal);
	float epsilon = 0.0001f;

	//Make sure that the ray isn't parallel to surface within an epsilon.
	if (fabs(d) < epsilon) {
		return false;
	}

	//Calculate intersection
	float inter = glm::dot((triangle.v0 - start), triangle.normal);
	float t = inter / d;

	//Check that the intersection isn't too close or too far away
	if (t < 0 || t + epsilon > 1/*maxLambda?*/) {
		return false;
	}

	//Calculate point of intersection
	vec3 p = start + t * dir;
	
	//### Cramer's rule ###
	//Area of triangle
	double area = glm::length(triangle.normal) / 2.0;

	//Area of beta part of the triangle. Check that it is within parameter, otherwise we end early.
	double beta = glm::dot(glm::cross(t1, p - triangle.v0), triangle.normal) / glm::dot(triangle.normal, triangle.normal);
	if (!(beta <= 1 && beta >= 0)) return false;

	//Area of gamma part of the triangle. Check that it is within parameter, otherwise we end early.
	double gamma = glm::dot(glm::cross(p - triangle.v0, t2), triangle.normal) / glm::dot(triangle.normal, triangle.normal);
	if (!(gamma <= 1 && gamma >= 0)) return false;

	//Area of gamma part of the triangle. Check that it is within parameter, otherwise we end early.
	double alpha = 1 - gamma - beta;
	if (!(alpha <= 1 && alpha >= 0)) return false;

	//Put into intersection
	intersection.distance = glm::length(p);
	intersection.position = p;
	//NOTE: We don't set index here, we set it when it returns.
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
	//Assume there isn't any intersections
	bool hasIntersection = false;
	Intersection intersection;
	float smallestDistance = std::numeric_limits<float>::max();

	//Go through all the triangles and check if it is the closest intersection.
	for (int i = 0; i < triangles.size(); i++)
	{
		if (CheckTriangleIntersection(start, dir, triangles[i], intersection))
		{
			//We have found an intersection
			hasIntersection = true;
			if (intersection.distance < smallestDistance)
			{
				//If it is closer than the last one we checked, save it and assign its triangle index.
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
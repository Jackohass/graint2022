#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/constants.hpp>
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
float focalLength = SCREEN_HEIGHT;
vec3 cameraPos(0, 0, -3);
mat3 R;
float yaw = 0;
vec3 lightPos(0, -0.5, -0.7);
vec3 lightColor = 14.f * vec3(1, 1, 1);
vec3 indirectLight = 0.5f * vec3(1, 1, 1);

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
	float epsilon = 0.00001;
	if (t - epsilon < 0 || t >= best) return false;

	float detA2 = glm::determinant(mat3(-dir, b, e2));
	float u = detA2 / detA;

	if (u < 0 || u > 1) return false;

	float detA3 = glm::determinant(mat3(-dir, e1, b));
	float v = detA3 / detA;

	if (v < 0 || u + v > 1) return false;

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
		if (CheckTriangleIntersection(start, dir, triangles[i],
			intersection, smallestDistance)) {
			//We have found an intersection
			if (intersection.distance < smallestDistance )
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

vec3 DirectLight(const Intersection& i, const vector<Triangle>& triangles)
{

	vec3 n = triangles[i.triangleIndex].normal;
	vec3 r = lightPos - i.position;
	vec3 rN = glm::normalize(r);
	Intersection inter;
	if (ClosestIntersection(i.position, r, triangles, inter))
	{
		if(inter.distance < 1) return vec3(0, 0, 0);
	}
	return (lightColor * glm::max(glm::dot(rN, n), 0.0f)) / (4 * glm::pi<float>() * glm::dot(r, r));
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

	vec3 forward(R[2][0], R[2][1], R[2][2]);
	vec3 right(R[0][0], R[0][1], R[0][2]);
	vec3 down(R[1][0], R[1][1], R[1][2]);

	Uint8* keystate = SDL_GetKeyState( 0 );
	if( keystate[SDLK_UP] )
	{
		// Move camera forward
		cameraPos += 0.1f * forward;
	}
	if( keystate[SDLK_DOWN] )
	{
		// Move camera backward
		cameraPos -= 0.1f * forward;
	}
	if( keystate[SDLK_LEFT] )
	{
		// Move camera to the left
		//cameraPos[0] -= 0.1;
		yaw += 5;
		float rad = glm::radians(yaw);
		R = mat3(glm::cos(rad), 0, glm::sin(rad),
			 0, 1, 0,
			-glm::sin(rad), 0, glm::cos(rad));
	}
	if( keystate[SDLK_RIGHT] )
	{
		// Move camera to the right
		//cameraPos[0] += 0.1;
		yaw -= 5;
		float rad = glm::radians(yaw);
		R = mat3(glm::cos(rad), 0, glm::sin(rad),
			 0, 1, 0,
			-glm::sin(rad), 0, glm::cos(rad));
	}
	if (keystate[SDLK_w]) lightPos += 0.1f * forward;
	if (keystate[SDLK_s]) lightPos -= 0.1f * forward;
	if (keystate[SDLK_d]) lightPos += 0.1f * right;
	if (keystate[SDLK_a]) lightPos -= 0.1f * right;
	if (keystate[SDLK_q]) lightPos += 0.1f * down;
	if (keystate[SDLK_e]) lightPos -= 0.1f * down;
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
			vec3 normD = glm::normalize(R * d);

			Intersection intersection;
			if(ClosestIntersection(cameraPos, normD,
							triangles, intersection)){
				vec3 colorT = triangles[intersection.triangleIndex].color;
				color = colorT * (DirectLight(intersection, triangles) + indirectLight);
			}

			PutPixelSDL( screen, x, y, color );
		}
	}

	if( SDL_MUSTLOCK(screen) )
		SDL_UnlockSurface(screen);

	SDL_UpdateRect( screen, 0, 0, 0, 0 );
}

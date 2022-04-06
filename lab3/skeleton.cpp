#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModel.h"

using namespace std;
using glm::vec3;
using glm::ivec2;
using glm::mat3;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;
SDL_Surface* screen;
int t;
vector<Triangle> triangles;
vec3 cameraPos(0, 0, -3.001);
const float focalLength = SCREEN_WIDTH;
mat3 R;
float yaw = 0;

// ----------------------------------------------------------------------------
// FUNCTIONS

void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels,
	vector<ivec2>& rightPixels);
void Update();
void Draw();
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);

int main( int argc, char* argv[] )
{
	LoadTestModel( triangles );
	screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT );
	t = SDL_GetTicks();	// Set start value for timer.

	vector<ivec2> vertexPixels(3);
	vertexPixels[0] = ivec2(10, 5);
	vertexPixels[1] = ivec2(5, 10);
	vertexPixels[2] = ivec2(15, 15);
	vector<ivec2> leftPixels;
	vector<ivec2> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	for (int row = 0; row < leftPixels.size(); ++row)

	while( NoQuitMessageSDL() )
	{
		//Update();
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

	Uint8* keystate = SDL_GetKeyState(0);

	vec3 forward(R[2][0], R[2][1], R[2][2]);
	vec3 right(R[0][0], R[0][1], R[0][2]);
	vec3 down(R[1][0], R[1][1], R[1][2]);

	if (keystate[SDLK_UP])
	{
		// Move camera forward
		cameraPos += 0.1f * forward;
	}
	if (keystate[SDLK_DOWN])
	{
		// Move camera backward
		cameraPos -= 0.1f * forward;
	}
	if (keystate[SDLK_LEFT])
	{
		// Move camera to the left
		//cameraPos[0] -= 0.1;
		yaw += 5;
		float rad = glm::radians(yaw);
		R = mat3(glm::cos(rad), 0, glm::sin(rad),
			0, 1, 0,
			-glm::sin(rad), 0, glm::cos(rad));
	}
	if (keystate[SDLK_RIGHT])
	{
		// Move camera to the right
		//cameraPos[0] += 0.1;
		yaw -= 5;
		float rad = glm::radians(yaw);
		R = mat3(glm::cos(rad), 0, glm::sin(rad),
			0, 1, 0,
			-glm::sin(rad), 0, glm::cos(rad));
	}
	//Move forwad, back, right, left, down and up respectively.
	/*if (keystate[SDLK_w]) lightPos += 0.1f * forward;
	if (keystate[SDLK_s]) lightPos -= 0.1f * forward;
	if (keystate[SDLK_d]) lightPos += 0.1f * right;
	if (keystate[SDLK_a]) lightPos -= 0.1f * right;
	if (keystate[SDLK_q]) lightPos += 0.1f * down;
	if (keystate[SDLK_e]) lightPos -= 0.1f * down;*/

	if( keystate[SDLK_RSHIFT] )
		;

	if( keystate[SDLK_RCTRL] )
		;

	if( keystate[SDLK_w] )
		;

	if( keystate[SDLK_s] )
		;

	if( keystate[SDLK_d] )
		;

	if( keystate[SDLK_a] )
		;

	if( keystate[SDLK_e] )
		;

	if( keystate[SDLK_q] )
		;
}

void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels,
	vector<ivec2>& rightPixels)
{
	// 1. Find max and min y-value of the polygon
	// and compute the number of rows it occupies.

	// 2. Resize leftPixels and rightPixels
	// so that they have an element for each row.

	// 3. Initialize the x-coordinates in leftPixels
	// to some really large value and the x-coordinates
	// in rightPixels to some really small value.

	// 4. Loop through all edges of the polygon and use
	// linear interpolation to find the x-coordinate for
	// each row it occupies. Update the corresponding
	// values in rightPixels and leftPixels.

	int min = numeric_limits<int>::max();
	int max = -numeric_limits<int>::max();

	for (int i = 0; i < vertexPixels.size(); i++)
	{
		if (min > vertexPixels[i].y) min = vertexPixels[i].y;
		if (max < vertexPixels[i].y) max = vertexPixels[i].y;
	}
	int rows = max-min;

	leftPixels.resize(rows);
	rightPixels.resize(rows);

	for (int i = 0; i < rows; ++i)
	{
		leftPixels[i].x = +numeric_limits<int>::max();
		rightPixels[i].x = -numeric_limits<int>::max();
	}

	for (int i = 0; i < vertexPixels.size(); i++)
	{
		ivec2 a = vertexPixels[i];
		ivec2 b = vertexPixels[(i+1) % vertexPixels.size()];
		ivec2 delta = glm::abs(a - b);
		int pixels = glm::max(delta.x, delta.y) + 1;
		vector<ivec2> line(pixels);
		Interpolate(a, b, line);
		for (int j = 0; j < line.size(); j++)
		{
			int y = line[j].y-min;
			if (line[j].x < leftPixels[y].x) leftPixels[y].x = line[j].x;
			if (line[j].x > rightPixels[y].x) rightPixels[y].x = line[j].x;
		}
	}
}

void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result)
{
	int N = result.size();
	glm::vec2 step = glm::vec2(b - a) / float(glm::max(N - 1, 1));
	glm::vec2 current(a);
	for (int i = 0; i < N; ++i)
	{
		result[i] = current;
		current += step;
	}
}

void VertexShader(const vec3& v, ivec2& p)
{
	vec3 t = R*(v - cameraPos);
	p.x = focalLength * t.x / t.z + SCREEN_WIDTH / 2;
	p.y = focalLength * t.y / t.z + SCREEN_HEIGHT / 2;
}

void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color)
{
	ivec2 delta = glm::abs(a - b);
	int pixels = glm::max(delta.x, delta.y) + 1;
	vector<ivec2> line(pixels);
	Interpolate(a, b, line);
	for (int i = 0; i < line.size(); i++) PutPixelSDL(surface, line[i].x, line[i].y, color);
}

void DrawPolygonEdges(const vector<vec3>& vertices)
{
	int V = vertices.size();
	// Transform each vertex from 3D world position to 2D image position:
	vector<ivec2> projectedVertices(V);
	for (int i = 0; i < V; ++i)
	{
		VertexShader(vertices[i], projectedVertices[i]);
	}
	// Loop over all vertices and draw the edge from it to the next vertex:
	for (int i = 0; i < V; ++i)
	{
		int j = (i + 1) % V; // The next vertex
		vec3 color(1, 1, 1);
		DrawLineSDL(screen, projectedVertices[i], projectedVertices[j],
			color);
	}
}

void Draw()
{
	SDL_FillRect(screen, 0, 0);
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);
	for (int i = 0; i < triangles.size(); ++i)
	{
		vector<vec3> vertices(3);
		vertices[0] = triangles[i].v0;
		vertices[1] = triangles[i].v1;
		vertices[2] = triangles[i].v2;
		DrawPolygonEdges(vertices);
	}
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}
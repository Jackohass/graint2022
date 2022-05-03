#include <iostream>
#include<sstream>
#include<fstream>
#include<string>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include <glm/gtx/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "TestModel.h"
#include "glad.h"

struct Pixel{
	int x;
	int y;
	float zinv;
	glm::vec3 pos3d; //Pixel Illumination
	//glm::vec3 illumination; //Vertex Illumination
};

struct Vertex{
	glm::vec3 position;
	//glm::vec3 normal; //Vertex Illumination
	//glm::vec3 reflectance; //Vertex Illumination
};

using namespace std;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::mat3;
using glm::mat4;

// ----------------------------------------------------------------------------
// GLOBAL VARIABLES

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;
SDL_Surface* screen;
int t;
vector<Object> objects;
vec4 cameraPos(0, 0, -3.001, 1);
mat4 cameraMatrix(1);
mat4 projectionMatrix = glm::perspective(53.0f,
			(float) SCREEN_WIDTH / (float) SCREEN_HEIGHT,
			0.1f, 100.0f);
const float focalLength = SCREEN_WIDTH;
mat4 R(1);
float yaw = 0;
vec3 currentColor(1, 1, 1);
vec3 currentNormal; //Per Pixel Illumination
vec3 currentReflectance; //Per Pixel Illunination
float depthBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
vec3 lightPos(0, 0.5, 0.7);
vec3 lightPower = 14.1f * vec3(1, 1, 1);
vec3 indirectLightPowerPerArea = 0.5f * vec3(1, 1, 1);
unsigned int shader;
unsigned int VAO;

// ----------------------------------------------------------------------------
// FUNCTIONS

void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels,
	vector<Pixel>& rightPixels);
void Update();
void Draw();
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result);
void VertexShader(const Vertex& v, Pixel& p);
void PixelShader(const Pixel& p);



void updateShaders(mat4 model, vec3 objectColor)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader, "camera"), 1, GL_FALSE, &cameraMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

	glUniform3fv(glGetUniformLocation(shader, "objectColor"), 1, &objectColor[0]);
	glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, &lightPos[0]);
	glUniform3fv(glGetUniformLocation(shader, "lightPower"), 1, &lightPower[0]);
	glUniform3fv(glGetUniformLocation(shader, "indirectLightPowerPerArea"), 1, &indirectLightPowerPerArea[0]);
}

void checkShaderErrors(int checkShader) {
	int success;
	glGetShaderiv(checkShader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		const int maxLength = 1024;
		//glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
		GLchar errorLog[maxLength];
		glGetShaderInfoLog(shader, maxLength, NULL, errorLog);
		std::cout << "ERROR::SHADER_COMPILATION_ERROR: \n";
		for (int i = 0; i < maxLength; i++)
		{
			cout << errorLog[i];
		}
		std::cout << "\n -- --------------------------------------------------- -- " << std::endl;
	}
	else {
		std::cout << "Compilation was successful" << std::endl;
	}
}

void setupShaders()
{
	string vertexString;
	string pixelString;
	ifstream vertexFile;
	ifstream pixelFile;
	std::stringstream vShaderStream, fShaderStream;
	vertexFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	pixelFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		vertexFile.open("shader/vertexShader.vert");
		pixelFile.open("shader/pixelShader.frag");
		vShaderStream << vertexFile.rdbuf();
		fShaderStream << pixelFile.rdbuf();
		vertexString = vShaderStream.str();
		pixelString = fShaderStream.str();
		vertexFile.close();
		pixelFile.close();
	} catch (std::ifstream::failure& e){
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
	}
	//std::cout << "vertex shader: " << vertexString << std::endl;
	//std::cout << "pixel shader: " << pixelString << std::endl;
	
	

	const char* codeVertex = vertexString.c_str();
	const char* codePixel = pixelString.c_str();
	

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &codeVertex, NULL);
	glCompileShader(vertexShader);
	checkShaderErrors(vertexShader);

	unsigned int pixelShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(pixelShader, 1, &codePixel, NULL);
	glCompileShader(pixelShader);
	checkShaderErrors(pixelShader);

	shader = glCreateProgram();
	glAttachShader(shader, vertexShader);
	glAttachShader(shader, pixelShader);

	glLinkProgram(shader);
	glUseProgram(shader);
	glDeleteShader(vertexShader);
	glDeleteShader(pixelShader);
}

int main(int argc, char* argv[])
{
	LoadTestModel(objects);
	screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	//Add depth buffer testing
	glEnable(GL_DEPTH_TEST);

	setupShaders();

	//Prepare the triangle buffer
	const int numVerts = 30 /*HARDCODE BE CAREFUL, SPICY*/ * 3 * 3 * 2;
	float vertices[numVerts];
	int curObjOffset = 0;
	for (int objIndex = 0; objIndex < objects.size(); objIndex++)
	{
		vector<Triangle>& triangles = objects[objIndex].triangles;
		for (int i = 0; i < triangles.size(); i++) {
			int curTriOffset = (curObjOffset + i) * 6 * 3;

			for (int j = 0; j < 3; j++) {
				int curVertOffset = curTriOffset + j * 6;

				for (int k = 0; k < 3; k++) {
					vertices[curVertOffset + k] = triangles[i].v[j][k];
				}
				for (int k = 0; k < 3; k++) {
					vertices[curVertOffset + 3 + k] = triangles[i].normal[k];
				}
			}
		}
		curObjOffset += triangles.size();
	}
	

	unsigned int VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(VAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	
	t = SDL_GetTicks();	// Set start value for timer.

	/*vector<ivec2> vertexPixels(3);
	vertexPixels[0] = ivec2(10, 5);
	vertexPixels[1] = ivec2(5, 10);
	vertexPixels[2] = ivec2(15, 15);
	vector<ivec2> leftPixels;
	vector<ivec2> rightPixels;
	ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
	for (int row = 0; row < leftPixels.size(); ++row)
	{
		cout << "Start: ("
			<< leftPixels[row].x << ","
			<< leftPixels[row].y << "). "
			<< "End: ("
			<< rightPixels[row].x << ","
			<< rightPixels[row].y << "). " << endl;
	}*/

	while( NoQuitMessageSDL() )
	{
		Update();
		Draw();
	}

	SDL_SaveBMP( screen, "screenshot.bmp" );
	return 0;
}

void handleInput(){
	Uint8* keystate = SDL_GetKeyState(0);

	vec4 forward(R[2][0], R[2][1], R[2][2], 0);
	vec4 right(R[0][0], R[0][1], R[0][2], 0);
	vec4 down(R[1][0], R[1][1], R[1][2], 0);

	if (keystate[SDLK_UP])
	{
		// Move camera forward
		cameraPos += 0.001f * forward;
	}
	if (keystate[SDLK_DOWN])
	{
		// Move camera backward
		cameraPos -= 0.001f * forward;
	}
	if (keystate[SDLK_LEFT])
	{
		// Move camera to the left
		//cameraPos[0] -= 0.1;
		yaw += 0.05f;
		float rad = glm::radians(yaw);
		R = mat4(glm::cos(rad), 0, glm::sin(rad), 0,
			0, 1, 0, 0,
			-glm::sin(rad), 0, glm::cos(rad), 0,
			0, 0, 0, 1);
	}
	if (keystate[SDLK_RIGHT])
	{
		// Move camera to the right
		//cameraPos[0] += 0.1;
		yaw -= 0.05f;
		float rad = glm::radians(yaw);
		R = mat4(glm::cos(rad), 0, glm::sin(rad), 0,
			0, 1, 0, 0, 
			-glm::sin(rad), 0, glm::cos(rad), 0,
			0, 0, 0, 1);
	}
	//Move forwad, back, right, left, down and up respectively.
	vec3 f(forward);
	vec3 r(right);
	vec3 d(down);

	if (keystate[SDLK_w]) lightPos += 0.001f * f;
	if (keystate[SDLK_s]) lightPos -= 0.001f * f;
	if (keystate[SDLK_d]) lightPos += 0.001f * r;
	if (keystate[SDLK_a]) lightPos -= 0.001f * r;
	if (keystate[SDLK_q]) lightPos += 0.001f * d;
	if (keystate[SDLK_e]) lightPos -= 0.001f * d;

	if( keystate[SDLK_RSHIFT] )
		;

	if( keystate[SDLK_RCTRL] )
		;

	mat4 trans(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, 1, 0), cameraPos);
	cameraMatrix = trans * R * glm::rotate(mat4(1), 180.0f, vec3(1, 0, 0));
}

void Update()
{
	//updateShaders(? , ? );

	// Compute frame time:
	int t2 = SDL_GetTicks();
	float dt = float(t2-t);
	t = t2;
	//cout << "Render time: " << dt << " ms." << endl;

	handleInput();
}

/*
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels,
	vector<Pixel>& rightPixels)
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
	int rows = max-min+1;

	leftPixels.resize(rows);
	rightPixels.resize(rows);

	for (int i = 0; i < rows; ++i)
	{
		leftPixels[i].x = +numeric_limits<int>::max();
		rightPixels[i].x = -numeric_limits<int>::max();
		leftPixels[i].y = i + min;
		rightPixels[i].y = i + min;
	}

	for (int i = 0; i < vertexPixels.size(); i++)
	{
		Pixel a = vertexPixels[i];
		Pixel b = vertexPixels[(i+1) % vertexPixels.size()];

		Pixel delta;
		delta.x = glm::abs(a.x - b.x);
		delta.y = glm::abs(a.y - b.y);
		int pixels = glm::max(delta.x, delta.y) + 1;

		vector<Pixel> line(pixels);
		Interpolate(a, b, line);
		for (int j = 0; j < line.size(); j++)
		{
			int y = line[j].y - min;
			if (line[j].x < leftPixels[y].x) leftPixels[y] = line[j];
			if (line[j].x > rightPixels[y].x) rightPixels[y] = line[j];
		}
	}
}

void DrawRows(const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels){
	for(int i = 0; i < leftPixels.size(); i++){
		int rowLength = rightPixels[i].x - leftPixels[i].x + 1;

		vector<Pixel> rowPixel(rowLength);
		Interpolate(leftPixels[i], rightPixels[i], rowPixel);

		for(int j = 0; j < rowLength; j++){
			PixelShader(rowPixel[j]);
		}
	}
}

void DrawPolygon( const vector<Vertex>& vertices ){
	int V = vertices.size();

	//Project the corners
	vector<Pixel> vertexPixels(V);
	for(int i = 0; i < V; ++i) VertexShader(vertices[i], vertexPixels[i]);

	//Compute the rows of the polygon
	vector<Pixel> leftPixels;
	vector<Pixel> rightPixels;
	ComputePolygonRows( vertexPixels, leftPixels, rightPixels );

	//Draw the polygon
	DrawRows(leftPixels, rightPixels);
}

void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result)
{
	int N = result.size();
	glm::vec2 step = glm::vec2(b - a) / float(glm::max(N - 1, 1));
	glm::vec2 current(a);
	for (int i = 0; i < N; ++i)
	{
		result[i] = glm::round(current);
		current += step;
	}
}

void Interpolate(Pixel a, Pixel b, vector<Pixel>& result){
	int N = result.size();

	//Convert the pixels to float vectors in 3d space
	glm::vec3 aVec(a.x, a.y, a.zinv);
	glm::vec3 bVec(b.x, b.y, b.zinv);

	//glm::vec3 aColour = a.illumination;
	//glm::vec3 bColour = b.illumination;
	
	glm::vec3 aPos3d = a.pos3d * a.zinv;
	glm::vec3 bPos3d = b.pos3d * b.zinv;

	glm::vec3 step = (bVec - aVec) / float(glm::max(N - 1, 1));
	//glm::vec3 colourStep = (bColour - aColour) / float(glm::max(N - 1, 1));
	//glm::vec3 pos3dStep = (bPos3d * b.zinv - aPos3d * a.zinv) / float(glm::max(N - 1, 1));
	glm::vec3 pos3dStep = (bPos3d - aPos3d) / float(glm::max(N - 1, 1));

	glm::vec3 current(aVec);
	//glm::vec3 currentColour(aColour);
	glm::vec3 currentPos3d(aPos3d);
	for (int i = 0; i < N; ++i)
	{
		//Convert the result back to a pixel
		Pixel res;
		res.x = glm::round(current.x);
		res.y = glm::round(current.y);
		res.zinv = current.z;
		//res.illumination = currentColour;
		res.pos3d = currentPos3d / res.zinv; //Pixel illumination
		result[i] = res;

		current += step;
		//currentColour += colourStep;
		currentPos3d += pos3dStep;
	}
}

void VertexShader(const Vertex& v, Pixel& p)
{
	//Convert the 3D coordinate to camera coordinates
	vec3 t = (v.position - cameraPos) * R;

	p.x = focalLength * t.x / t.z + SCREEN_WIDTH / 2;
	p.y = focalLength * t.y / t.z + SCREEN_HEIGHT / 2;
	p.zinv = 1.0f / t.z; //Calculate the inverse of z for the depth buffer

	//Calculate the illumination information for the vertex pixel
	p.pos3d = v.position;
*/
	/*
	vec3 r = lightPos - v.position;
	float sphereArea = 4.0f * glm::pi<float>() * glm::dot(r, r);
	vec3 rNorm = glm::normalize(r);
	vec3 D = (lightPower * glm::max(glm::dot(rNorm, v.normal), 0.0f)) / sphereArea;
	p.illumination = v.reflectance * (D + indirectLightPowerPerArea);
	*/
/*
}
*/

/*
void PixelShader(const Pixel& p){
	int x = p.x;
	int y = p.y;

	if(x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;

	if(depthBuffer[y][x] < p.zinv){
		depthBuffer[y][x] = p.zinv;

		vec3 r = lightPos - p.pos3d;
		float sphereArea = 4.0f * glm::pi<float>() * glm::dot(r, r);
		vec3 rNorm = glm::normalize(r);
		vec3 D = (lightPower * glm::max(glm::dot(rNorm, currentNormal), 0.0f)) / sphereArea;
		vec3 illumination = currentReflectance * (D + indirectLightPowerPerArea);

		//PutPixelSDL(screen, x, y, illumination);
	}
}

void DrawLineSDL(SDL_Surface* surface, ivec2 a, ivec2 b, vec3 color)
{
	ivec2 delta = glm::abs(a - b);
	int pixels = glm::max(delta.x, delta.y) + 1;
	vector<ivec2> line(pixels);
	Interpolate(a, b, line);
	for (int i = 0; i < line.size(); i++) PutPixelSDL(surface, line[i].x, line[i].y, color);
}

void DrawPolygonEdges(const vector<Vertex>& vertices)
{
	int V = vertices.size();
	// Transform each vertex from 3D world position to 2D image position:
	vector<Pixel> projectedVertices(V);
	for (int i = 0; i < V; ++i)
	{
		VertexShader(vertices[i], projectedVertices[i]);
	}
	// Loop over all vertices and draw the edge from it to the next vertex:
	for (int i = 0; i < V; ++i)
	{
		int j = (i + 1) % V; // The next vertex
		vec3 color(1, 1, 1);
		
		ivec2 first(projectedVertices[i].x, projectedVertices[i].y);
		ivec2 second(projectedVertices[j].x, projectedVertices[j].y);

		DrawLineSDL(screen, first, second, color);
	}
}
*/

void Draw(){
	/*SDL_FillRect(screen, 0, 0);
	for(int y = 0; y < SCREEN_HEIGHT; y++)
		for(int x = 0; x < SCREEN_WIDTH; x++) depthBuffer[y][x] = 0;

	if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

	for (int i = 0; i < triangles.size(); ++i)
	{
		//Update the draw colour
		//currentColor = triangles[i].color;
		currentNormal = triangles[i].normal;
		currentReflectance = triangles[i].color;

		vector<Vertex> vertices(3);
		vertices[0].position = triangles[i].v[0];
		//vertices[0].normal = triangles[i].normal;
		//vertices[0].reflectance = triangles[i].color;

		vertices[1].position = triangles[i].v[1];
		//vertices[1].normal = triangles[i].normal;
		//vertices[1].reflectance = triangles[i].color;

		vertices[2].position = triangles[i].v[2];
		//vertices[2].normal = triangles[i].normal;
		//vertices[2].reflectance = triangles[i].color;

		DrawPolygon(vertices);
	}
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	SDL_UpdateRect(screen, 0, 0, 0, 0);*/

	glClearColor(0.0f, 0.0f, 0.0f, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Update our shaders based on the camera positions and stuff
	glUseProgram(shader); //TODO: Do once

	//Render
	glBindVertexArray(VAO);
	int currObjOffset = 0;
	for (int i = 0; i < objects.size(); i++)
	{
		int numtri = objects[i].triangles.size() * 3;
		updateShaders(mat4(1), objects[i].colour); //TODO: By copy or by reference?
		glDrawArrays(GL_TRIANGLES, currObjOffset, numtri);
		currObjOffset += numtri;
	}
	

	SDL_GL_SwapBuffers();
}

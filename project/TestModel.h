#ifndef TEST_MODEL_CORNEL_BOX_H
#define TEST_MODEL_CORNEL_BOX_H

// Defines a simple test model: The Cornel Box

#include <glm/glm.hpp>
#include <vector>

// Used to describe a triangular surface:
class Triangle
{
public:
	std::vector<glm::vec3> v;
	glm::vec3 normal;
	glm::vec3 color;

	Triangle( glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 color )
		: color(color)
	{
		v.push_back(v0);
		v.push_back(v1);
		v.push_back(v2);
		ComputeNormal();
	}

	void ComputeNormal()
	{
		glm::vec3 e1 = v[1]-v[0];
		glm::vec3 e2 = v[2]-v[0];
		normal = glm::normalize( glm::cross( e2, e1 ) );
	}
};

struct Object {
	std::vector<Triangle> triangles;
	glm::vec3 colour;
};

// Loads the Cornell Box. It is scaled to fill the volume:
// -1 <= x <= +1
// -1 <= y <= +1
// -1 <= z <= +1
void LoadTestModel( std::vector<Object>& objects )
{
	using glm::vec3;

	// Defines colors:
	vec3 red(    0.75f, 0.15f, 0.15f );
	vec3 yellow( 0.75f, 0.75f, 0.15f );
	vec3 green(  0.15f, 0.75f, 0.15f );
	vec3 cyan(   0.15f, 0.75f, 0.75f );
	vec3 blue(   0.15f, 0.15f, 0.75f );
	vec3 purple( 0.75f, 0.15f, 0.75f );
	vec3 white(  0.75f, 0.75f, 0.75f );

	objects.clear();
	objects.resize( 7 );

	// ---------------------------------------------------------------------------
	// Room

	float L = 555;			// Length of Cornell Box side.

	vec3 A(L,0,0);
	vec3 B(0,0,0);
	vec3 C(L,0,L);
	vec3 D(0,0,L);

	vec3 E(L,L,0);
	vec3 F(0,L,0);
	vec3 G(L,L,L);
	vec3 H(0,L,L);

	// Floor:
	objects[0].triangles.push_back( Triangle( C, B, A, green ) );
	objects[0].triangles.push_back( Triangle( C, D, B, green ) );
	objects[0].colour = green;

	// Left wall
	objects[1].triangles.push_back( Triangle( A, E, C, purple ) );
	objects[1].triangles.push_back( Triangle( C, E, G, purple ) );
	objects[1].colour = purple;

	// Right wall
	objects[2].triangles.push_back( Triangle( F, B, D, yellow ) );
	objects[2].triangles.push_back( Triangle( H, F, D, yellow ) );
	objects[2].colour = yellow;

	// Ceiling
	objects[3].triangles.push_back( Triangle( E, F, G, cyan ) );
	objects[3].triangles.push_back( Triangle( F, H, G, cyan ) );
	objects[3].colour = cyan;

	// Back wall
	objects[4].triangles.push_back( Triangle( G, D, C, white ) );
	objects[4].triangles.push_back( Triangle( G, H, D, white ) );
	objects[4].colour = white;

	// ---------------------------------------------------------------------------
	// Short block

	A = vec3(290,0,114);
	B = vec3(130,0, 65);
	C = vec3(240,0,272);
	D = vec3( 82,0,225);

	E = vec3(290,165,114);
	F = vec3(130,165, 65);
	G = vec3(240,165,272);
	H = vec3( 82,165,225);

	// Front
	objects[5].triangles.push_back( Triangle(E,B,A,red) );
	objects[5].triangles.push_back( Triangle(E,F,B,red) );

	// Front
	objects[5].triangles.push_back( Triangle(F,D,B,red) );
	objects[5].triangles.push_back( Triangle(F,H,D,red) );

	// BACK
	objects[5].triangles.push_back( Triangle(H,C,D,red) );
	objects[5].triangles.push_back( Triangle(H,G,C,red) );

	// LEFT
	objects[5].triangles.push_back( Triangle(G,E,C,red) );
	objects[5].triangles.push_back( Triangle(E,A,C,red) );

	// TOP
	objects[5].triangles.push_back( Triangle(G,F,E,red) );
	objects[5].triangles.push_back( Triangle(G,H,F,red) );
	objects[5].colour = red;

	// ---------------------------------------------------------------------------
	// Tall block

	A = vec3(423,0,247);
	B = vec3(265,0,296);
	C = vec3(472,0,406);
	D = vec3(314,0,456);

	E = vec3(423,330,247);
	F = vec3(265,330,296);
	G = vec3(472,330,406);
	H = vec3(314,330,456);

	// Front
	objects[6].triangles.push_back( Triangle(E,B,A,blue) );
	objects[6].triangles.push_back( Triangle(E,F,B,blue) );

	// Front
	objects[6].triangles.push_back( Triangle(F,D,B,blue) );
	objects[6].triangles.push_back( Triangle(F,H,D,blue) );

	// BACK
	objects[6].triangles.push_back( Triangle(H,C,D,blue) );
	objects[6].triangles.push_back( Triangle(H,G,C,blue) );

	// LEFT
	objects[6].triangles.push_back( Triangle(G,E,C,blue) );
	objects[6].triangles.push_back( Triangle(E,A,C,blue) );

	// TOP
	objects[6].triangles.push_back( Triangle(G,F,E,blue) );
	objects[6].triangles.push_back( Triangle(G,H,F,blue) );
	objects[6].colour = blue;

	// ----------------------------------------------
	// Scale to the volume [-1,1]^3

	for( size_t j=0; j< objects.size(); ++j )
	{
		std::vector<Triangle>& triangles = objects[j].triangles;
		for (int i = 0; i < triangles.size(); i++)
		{
			
			triangles[i].v[0] *= 2 / L;
			triangles[i].v[1] *= 2 / L;
			triangles[i].v[2] *= 2 / L;

			triangles[i].v[0] -= vec3(1, 1, 1);
			triangles[i].v[1] -= vec3(1, 1, 1);
			triangles[i].v[2] -= vec3(1, 1, 1);

			triangles[i].v[0].x *= -1;
			triangles[i].v[1].x *= -1;
			triangles[i].v[2].x *= -1;

			triangles[i].v[0].y *= -1;
			triangles[i].v[1].y *= -1;
			triangles[i].v[2].y *= -1;

			triangles[i].ComputeNormal();
		}
		
	}
}

#endif

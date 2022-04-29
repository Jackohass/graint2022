#version 330 core
out vec4 illumination;

in vec3 normal;  
in vec3 pos3d;  
  
uniform vec3 lightPos;
uniform vec3 lightPower;
uniform vec3 objectColor;
uniform	vec3 indirectLightPowerPerArea;

void main()
{
	vec3 r = lightPos - pos3d;
	float sphereArea = 4.0f * pi<float>() * dot(r, r);
	vec3 rNorm = normalize(r);
	vec3 D = (lightPower * max(dot(rNorm, normal), 0.0f)) / sphereArea;
	vec3 temp = objectColor * (D + indirectLightPowerPerArea);
	illumination = vec4(temp, 1.0f);
} 
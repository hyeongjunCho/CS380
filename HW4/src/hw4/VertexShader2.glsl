#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
//TODO: grab color value from the application
layout(location = 2) in vec3 vertexColor;
//TODO: grab normal value from the application
layout(location = 1) in vec3 vertexNormal_modelspace;


// Output data ; will be interpolated for each fragment.
out vec3 fragmentPosition;
out vec3 fragmentColor;
flat out vec3 fragmentNormal;

uniform mat4 ModelTransform;
uniform mat4 Eye;
uniform mat4 Projection;
//uniform vec3 uLight;
uniform vec3 Color;
uniform vec3 pColor;
uniform vec3 sColor;
uniform float Intensity;
uniform float pIntensity;
uniform float sIntensity;
uniform vec3 Direction;
uniform vec3 Axis;
uniform vec3 pLocation;
uniform vec3 sLocation;
uniform float Radius;
uniform int dL;
uniform int pL;
uniform int sL;

void main(){

	// Output position of the vertex, in clip space : MVP * position
	mat4 MVM = inverse(Eye) * ModelTransform;
	vec4 wPosition = MVM * vec4(vertexPosition_modelspace, 1);
	fragmentPosition = wPosition.xyz;
	gl_Position = Projection * wPosition;

	//gl_Position = Projection * ModelTransform * vec4(vertexPosition_modelspace,1);
	
	//TODO: pass the interpolated color value to fragment shader 
	fragmentColor = vertexColor;
	//TODO: Calculate/Pass normal of the the vertex
	//transpose of inversed model view matrix
	mat4 invm = inverse(MVM);
	invm[0][3] = 0; invm[1][3] = 0; invm[2][3] = 0;
	mat4 NVM = transpose(invm);
	vec4 tnormal = vec4(vertexNormal_modelspace, 0.0);
	fragmentNormal = vec3(NVM * tnormal);
}
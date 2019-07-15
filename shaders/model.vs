R""(
#version 140
uniform mat4 uWorldMatrix;
uniform mat4 uViewProjMatrix;

in vec3 aPosition;
in vec3 aNormal;

smooth out vec3 vNormalW;

void main() 
{
	vNormalW = mat3(uWorldMatrix) * aNormal;
	gl_Position = uViewProjMatrix * (uWorldMatrix * vec4(aPosition, 1.0));
}
)""
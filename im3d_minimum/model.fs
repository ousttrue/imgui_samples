R""(
#version 140
smooth in vec3 vNormalW;

out vec3 fResult;

void main() 
{
	vec3 nrm = normalize(vNormalW);
	vec3 ldir = normalize(vec3(1.0));
	float ndotl = max(dot(nrm, ldir), 0.0) * 0.5;
	fResult = vec3(ndotl);
}
)""
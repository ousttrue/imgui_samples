R""(
#version 140
#define kAntialiasing 2.0

noperspective in vec2 vUv;
noperspective in float vSize;
smooth in vec4 vColor;

out vec4 fResult;

void main()
{
    fResult = vColor;
    float d = length(vUv - vec2(0.5));
    d = smoothstep(0.5, 0.5 - (kAntialiasing / vSize), d);
    fResult.a *= d;
}
)""
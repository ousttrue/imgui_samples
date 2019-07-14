R""(
#version 140
#define kAntialiasing 2.0

noperspective in float vEdgeDistance;
noperspective in float vSize;
smooth in vec4 vColor;

out vec4 fResult;

void main()
{
    fResult = vColor;
    float d = abs(vEdgeDistance) / vSize;
    d = smoothstep(1.0, 1.0 - (kAntialiasing / vSize), d);
    fResult.a *= d;
}
)""
R""(
#version 140
#define kAntialiasing 2.0

noperspective in float vSize;
smooth in vec4 vColor;

out vec4 fResult;

void main()
{
    fResult = vColor;
}
)""
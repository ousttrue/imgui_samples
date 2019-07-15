R""(
#version 140
#define kAntialiasing 2.0

/*	This vertex shader fetches Im3d vertex data manually from a uniform buffer (uVertexData). It assumes that the bound vertex buffer contains 4 vertices as follows:

	 -1,1       1,1
	   2 ------- 3
	   | \       |
	   |   \     |
	   |     \   |
	   |       \ |
	   0 ------- 1
	 -1,-1      1,-1
	 
	Both the vertex ID and the vertex position are used for point/line expansion. This vertex buffer is valid for both triangle strip rendering (points/lines), and
	triangle render using vertices 0,1,2.
	
	See im3d_opengl31.cpp for more details.
*/

struct VertexData
{
    vec4 m_positionSize;
    uint m_color;
};
uniform VertexDataBlock
{
    VertexData uVertexData[(64 * 1024) / 32]; // assume a 64kb block size, 32 is the aligned size of VertexData
};

uniform mat4 uViewProjMatrix;
uniform vec2 uViewport;

in vec4 aPosition;

noperspective out vec2 vUv;
noperspective out float vSize;
smooth out vec4 vColor;

vec4 UintToRgba(uint _u)
{
    vec4 ret = vec4(0.0);
    ret.r = float((_u & 0xff000000u) >> 24u) / 255.0;
    ret.g = float((_u & 0x00ff0000u) >> 16u) / 255.0;
    ret.b = float((_u & 0x0000ff00u) >> 8u) / 255.0;
    ret.a = float((_u & 0x000000ffu) >> 0u) / 255.0;
    return ret;
}

void main()
{
    int vid = gl_InstanceID;

    vSize = max(uVertexData[vid].m_positionSize.w, kAntialiasing);
    vColor = UintToRgba(uVertexData[vid].m_color);
    vColor.a *= smoothstep(0.0, 1.0, vSize / kAntialiasing);

    gl_Position = uViewProjMatrix * vec4(uVertexData[vid].m_positionSize.xyz, 1.0);
    vec2 scale = 1.0 / uViewport * vSize;
    gl_Position.xy += aPosition.xy * scale * gl_Position.w;
    vUv = aPosition.xy * 0.5 + 0.5;
}
)""
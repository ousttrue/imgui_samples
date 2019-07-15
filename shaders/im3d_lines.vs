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

noperspective out float vEdgeDistance;
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
    int vid0 = gl_InstanceID * 2;                   // line start
    int vid1 = vid0 + 1;                            // line end
    int vid = (gl_VertexID % 2 == 0) ? vid0 : vid1; // data for this vertex

    vColor = UintToRgba(uVertexData[vid].m_color);
    vSize = uVertexData[vid].m_positionSize.w;
    vColor.a *= smoothstep(0.0, 1.0, vSize / kAntialiasing);
    vSize = max(vSize, kAntialiasing);
    vEdgeDistance = vSize * aPosition.y;

    vec4 pos0 = uViewProjMatrix * vec4(uVertexData[vid0].m_positionSize.xyz, 1.0);
    vec4 pos1 = uViewProjMatrix * vec4(uVertexData[vid1].m_positionSize.xyz, 1.0);
    vec2 dir = (pos0.xy / pos0.w) - (pos1.xy / pos1.w);
    dir = normalize(vec2(dir.x, dir.y * uViewport.y / uViewport.x)); // correct for aspect ratio
    vec2 tng = vec2(-dir.y, dir.x) * vSize / uViewport;

    gl_Position = (gl_VertexID % 2 == 0) ? pos0 : pos1;
    gl_Position.xy += tng * aPosition.y * gl_Position.w;
}
)""
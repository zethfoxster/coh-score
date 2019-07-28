/** @file default_vs.fx
    @brief Substance Tutorial, Direct3D 9 default Pixel Shader
    @author Christophe Soum
    @date 20080918
    @copyright Allegorithmic. All rights reserved.
	
	TBN
*/

// matrices
uniform float4x4 g_mWorldViewProjection : WORLDVIEWPROJECTION;
uniform float4x4 g_mWorld : WORLD;

// positions
uniform float3 g_fObsPos,g_fLightPos;

// vertex Shader Input
struct VSDefaultIn
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 tangent	: TANGENT;
    float3 binormal	: BINORMAL;    
};

// vertex Shader Output
struct VSDefaultOut
{
    float4 position		: POSITION;
	float4 color		: COLOR;
    float2 texcoord		: TEXCOORD0;
    float3 normal		: TEXCOORD1;
    float3 viewDir		: TEXCOORD2;
    float3 lightDir		: TEXCOORD3;
    float3 tangent		: TEXCOORD4;
    float3 binormal		: TEXCOORD5;
};

//the vertex shader
VSDefaultOut main(VSDefaultIn In)
{
    VSDefaultOut Out;

	//transform the vertex and its properties (normal, binormal, tangent)
    Out.position = mul(float4(In.position,1.0), g_mWorldViewProjection);
    Out.normal = mul(In.normal, g_mWorld);
    Out.tangent = mul(In.tangent, g_mWorld);
	Out.binormal = mul(In.binormal, g_mWorld);
    
    //texcoords and vertex color just passthrough
    Out.texcoord = float2(In.texcoord.x,1.0-In.texcoord.y);
    Out.color = (1.0).xxxx;//In.color;
    
    //compute the viewDir and the lightDir
    float4 fObjectPos = mul(float4(In.position,1.0), g_mWorld);
    Out.viewDir = g_fObsPos - fObjectPos.xyz;
    Out.lightDir = -g_fLightPos + fObjectPos.xyz;
    
    return Out;
}

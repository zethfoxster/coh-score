/** @file default_fs.fx
    @brief Substance Tutorial, Direct3D 9 default Pixel Shader
    @author Christophe Soum
    @date 20080918
    @copyright Allegorithmic. All rights reserved.
	
	Parallax
	Diffuse + Emissive + Specular
*/


// Samplers
uniform sampler2D specularTexSampler;
uniform sampler2D normalTexSampler;
uniform sampler2D emissiveTexSampler;
uniform sampler2D diffuseTexSampler;

//Pixel Shader Input
struct PSDefaultIn
{
	float4 color		: COLOR;
    float2 texcoord		: TEXCOORD0;
    float3 normal		: TEXCOORD1;
    float3 viewDir		: TEXCOORD2;
    float3 lightDir		: TEXCOORD3;
    float3 tangent		: TEXCOORD4;
    float3 binormal		: TEXCOORD5;
};

float4 main(PSDefaultIn In) : COLOR
{
	//normalize the intepolated lights and view vectors
    In.viewDir = normalize(In.viewDir);
    In.lightDir = normalize(In.lightDir);

	float3x3 tbn = float3x3(In.tangent,In.binormal,In.normal);
	float3 vTan = mul(tbn,In.viewDir);

	//compute the offset for parallax
	float4 height = tex2D(normalTexSampler, In.texcoord);
	float2 texUV =  In.texcoord - (vTan.xy/vTan.z) * (height.a  - 0.5) * 0.0;

	//sample the normalmap with the offset and clip it to [-1,1]
	float3 normalMap = tex2D(normalTexSampler, texUV) * 2.0 - 1.0;
	normalMap.z = -normalMap.z;

	//bumpmapping
	In.normal = normalize(normalMap.x*In.tangent + 
		normalMap.y*In.binormal + 
		normalMap.z*In.normal);

	//pre-sample the diffuse map if a diffuse texture is loaded
    float4 color = tex2D(diffuseTexSampler, texUV);

	// phong
    float NDotL = dot(In.normal, In.lightDir);
	float NDotH = dot(normalize(In.lightDir+In.viewDir),In.normal);
	float4 litres = lit(NDotL,NDotH,16.0);
	float4 emissive = tex2D(emissiveTexSampler, texUV);

	// now combine every component
	color = saturate(
		tex2D(specularTexSampler, texUV)*(litres.z*color.a) + 
		color * litres.y + 
		emissive);

	// returning the final result
	return float4(color.rgb,1);
};

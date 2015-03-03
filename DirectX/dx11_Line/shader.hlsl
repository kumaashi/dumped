
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

cbuffer cbNeverChanges : register( b0 )
{
    matrix View;
};

cbuffer cbChangeOnResize : register( b1 )
{
    matrix Projection;
};

cbuffer cbChangesEveryFrame : register( b2 )
{
    matrix World;
    float4 vMeshColor;
};


//--------------------------------------------------------------------------------------
// OUTPUT
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4                Pos : POSITION;
	float4                Tex : TEXCOORD;
	float4                Col : COLOR;
	column_major float4x4 M   : MATRIX;
	uint                  iid : SV_InstanceID;
};

struct GS_INPUT
{
	float4                Pos : POSITION;
	float4                Tex : TEXCOORD;
	float4                Col : COLOR;
	column_major float4x4 M   : MATRIX;
};

struct PS_INPUT
{
	float4 Pos   : SV_POSITION;
	float4 Tex   : TEXCOORD0;
	float4 Col   : COLOR0;
	float4 Depth : COLOR1;
};

//----------------------------------------------------------------------------------
// Vertex Shader 
//----------------------------------------------------------------------------------
GS_INPUT vs_main( VS_INPUT input )
{
	GS_INPUT output = (GS_INPUT)0;
	output.Pos = input.Pos;
	output.Tex = input.Tex;
	output.Col = input.Col;
	output.M   = input.M;
	return output;
}

//----------------------------------------------------------------------------------
// Geometry Shader 
//----------------------------------------------------------------------------------
[maxvertexcount(3)]
void gs_main(inout TriangleStream<PS_INPUT> gsstream, triangle GS_INPUT input[3])
{
	for(int i = 0 ; i < 3; i++)
	{
		PS_INPUT output = (PS_INPUT)0;
		output.Pos   = mul(input[i].Pos, input[i].M);
		output.Tex   = input[i].Tex;
		output.Col   = input[i].Col;
		output.Depth = output.Pos.z / output.Pos.w;
		gsstream.Append(output);
	}
}

//----------------------------------------------------------------------------------
// Pixel Shader 
//----------------------------------------------------------------------------------
float4 ps_main( PS_INPUT input) : SV_Target
{
	return float4(1, 2, 3, 4) * 0.1 * input.Col;
	float2 uv = -1.0 + 2.0 * input.Tex.xy;
	float delta = 0.1;
	float val0 = 1 - pow(dot(uv + delta, uv + delta), 5);
	float val1 = 1 - pow(dot(uv + delta, uv - delta), 5);
	float val2 = 1 - pow(dot(uv - delta, uv + delta), 5);
	float val3 = 1 - pow(dot(uv - delta, uv - delta), 5);
	float val = (val0 + val1 + val2 + val3) / 4;
	if(val <= 0) discard;
	return val * float4(1, 2, 3, 4) * 0.1;
	//return gants * float4( pow((input.Depth.xyz).xyz, 64.0), 1.0);// * input.Col;
}


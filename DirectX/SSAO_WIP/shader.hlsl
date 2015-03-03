//--------------------------------------------------------------------------------------
// OUTPUT
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4                Pos :	POSITION;
	float4                Tex :	TEXCOORD;
	float4                Col :	COLOR;
	column_major float4x4 M   :	MATRIX;
	uint                  iid :	SV_InstanceID;
};

struct GS_INPUT
{
	float4                Pos :	POSITION;
	float4                Tex :	TEXCOORD;
	float4                Col :	COLOR;
	column_major float4x4 M   :	MATRIX;
};

struct PS_INPUT
{
	float4 Pos   : SV_POSITION;
	float4 Tex   : TEXCOORD0;
	float4 Col   : COLOR0;
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
[maxvertexcount(6)]
void gs_main(inout TriangleStream<PS_INPUT> gsstream, triangle GS_INPUT input[3])
{
	{
		for(int i = 0 ; i < 3; i++)
		{
			float4 Pos = input[i].Pos;
			PS_INPUT output = (PS_INPUT)0;
			Pos.y += 1.0;
			if(Pos.y > 0) Pos.xz *= 0.5;
			output.Pos    = mul(Pos, input[i].M);
			output.Tex    = input[i].Tex;
			output.Col    = input[i].Col;
			output.Col.w  = output.Pos.z / output.Pos.w;
			gsstream.Append(output);
		}
	}
	gsstream.RestartStrip();
	{
		for(int i = 0 ; i < 3; i++)
		{
			float4 Pos = input[i].Pos;
			PS_INPUT output = (PS_INPUT)0;
			Pos.y += 5.0;
			//Pos.y *= 0.33;
			if(Pos.y > 4) Pos.xz *= 0.1;
			else          Pos.xz *= 0.5;
			output.Pos    = mul(Pos, input[i].M);
			output.Tex    = input[i].Tex;
			output.Col    = input[i].Col;
			output.Col.w  = output.Pos.z / output.Pos.w;
			gsstream.Append(output);
		}
	}
}

//----------------------------------------------------------------------------------
// Pixel Shader
//----------------------------------------------------------------------------------
float4 ps_main( PS_INPUT input) : SV_Target
{
	return float4(input.Col);
	//return gants * float4( pow((input.Depth.xyz).xyz, 64.0), 1.0);// * input.Col;
}

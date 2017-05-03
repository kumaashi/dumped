#include "common.hlsl"


float2 rotate(float2 p, float a) {
	return float2(
		p.x * cos(a) - p.y * sin(a),
		p.x * sin(a) + p.y * cos(a));
}

[RootSignature(RSDEF)]
VSOutput VSMain(const VSInput input)
{
	VSOutput output = (VSOutput)0;
	InstancingData instancingdata  = instdata[input.Id];
	float4 apos          = float4(input.Position, 1.0f);
	apos.xy = rotate(apos.xy, matrixdata.Time.x);
	apos.yz = rotate(apos.yz, matrixdata.Time.x * 2.0);
	apos = mul(instancingdata.world, apos);
	output.Position      = mul(matrixdata.Proj, mul(matrixdata.View, apos));
	output.Normal        = input.Normal;
	output.TexCoord      = input.TexCoord;
	output.Color         = instancingdata.color;
	output.WorldPosition = apos;
	output.TransPosition = output.Position;
	return output;
}

/*
[maxvertexcount(6 * 5 * 2)]
void GSMain(point float4 pos[1] : SV_POSITION, inout TriangleStream<VSOutput> gsstream ) {
	VSOutput output = (VSOutput)0;
	gsstream.Append(output);
	gsstream.Append(output);
	gsstream.Append(output);
  	gsstream.RestartStrip();
}
*/

[RootSignature(RSDEF)]
PSOutput PSMain(const VSOutput input)
{
    PSOutput output = (PSOutput)0;
	float3 N = input.Normal;
	float Z  = input.TransPosition.z / input.TransPosition.w;
	output.Color0 = input.Color;
	output.Color1 = float4(N, 1.0);
	output.Color2 = float4(input.WorldPosition.xyz, 1.0);
	output.Color3 = float4(Z, Z, Z, 1.0);
    return output;
}



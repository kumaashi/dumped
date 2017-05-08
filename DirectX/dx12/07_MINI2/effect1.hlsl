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
	output.Position      = float4(input.Position, 1.0f);
	output.Normal        = input.Normal;
	output.TexCoord      = input.TexCoord;
	output.Color         = instancingdata.color;
	output.WorldPosition = input.Id;
	output.TransPosition = output.Position;
	output.Id            = input.Id;
	return output;
}

[maxvertexcount(3)]
void GSMain(triangle VSOutput inputvertex[3], inout TriangleStream<VSOutput> gsstream ) {
	VSOutput output = (VSOutput)0;
	float3 v0 = inputvertex[0].Position;
	float3 v1 = inputvertex[1].Position;
	float3 v2 = inputvertex[2].Position;
	float3 N = normalize(cross(v1 - v0, v2 - v0));
	for(int i = 0; i < 3; i++) {
		VSOutput input = inputvertex[i];
		InstancingData instancingdata  = instdata[input.Id];
		float4 apos          = input.Position;
		apos.xy = rotate(apos.xy, matrixdata.Time.x + input.Id);
		apos.yz = rotate(apos.yz, matrixdata.Time.x * 2.0 + input.Id);
		apos = mul(instancingdata.world, apos);
		output.Position      = mul(matrixdata.Proj, mul(matrixdata.View, apos));
		output.Normal        = normalize(N);

		output.TexCoord      = input.TexCoord;
		output.Color         = instancingdata.color;
		output.WorldPosition = apos;
		output.TransPosition = output.Position;
		gsstream.Append(output);
  	}
  	gsstream.RestartStrip();
}

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



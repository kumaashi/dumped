#include "common.hlsl"

[RootSignature(RSDEF)]
VSOutput VSMain(const VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.Position      = float4(input.Position * float3(1,1,0), 1.0f);
	output.Normal        = input.Normal;
	output.Color         = float4(1,1,1,1);
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
	float3 texinfo0;
	ColorTexture0.GetDimensions(0, texinfo0.x, texinfo0.y, texinfo0.z);
	float2 uv = input.Position.xy / texinfo0.xy;
	output.Color0 = ColorTexture2.Sample(ColorSmp, uv);
    return output;
}



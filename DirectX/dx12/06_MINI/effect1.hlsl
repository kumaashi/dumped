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
	float2 duv = (uv * 2.0) - 1.0;
	float4 color2 = saturate(ColorTexture2.Sample(ColorSmp, uv));
	output.Color0 = color2 * (1.0 - dot(duv * 0.5, duv));
	
	//debug
	if(uv.x > 0.5) {
		float4 color3 = saturate(ColorTexture3.Sample(ColorSmp, uv));
		output.Color0 = color3 * (1.0 - dot(duv * 0.5, duv));
	}
	
    return output;
}



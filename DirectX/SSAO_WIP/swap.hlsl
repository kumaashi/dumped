Texture2D             tex  : register(t0);
Texture2DMS<float4> mstex  : register(t1);
SamplerState        sample : register(s0);

struct VS_INPUT
{
	float4 pos : POSITION;
	float4 tex : TEXCOORD;
};

struct PS_INPUT
{
	float4 pos  : SV_POSITION;
	float4 tex  : TEXCOORD0;
	float4 misc : TEXCOORD1;
};

PS_INPUT vs_main( VS_INPUT input )
{
	PS_INPUT output = (PS_INPUT)0;
	output.pos      = input.pos;
	output.tex      = input.tex;
	output.misc     = output.pos;
	return output;
}

float rand( float2 p )
{
	return frac( sin( dot(p, float2(5.2312, 3.3217) ) ) * 23327.32348) * 2 - 1;
}

float4 ps_main(PS_INPUT input) : SV_Target
{
	uint width, height, samples;
	mstex.GetDimensions(width, height, samples);
	float2 uv  = input.pos.xy / float2(width, height);
	float2 ouv = 2*uv-1;
	float  dep = tex.SampleLevel(sample, uv, 0).w;
	if(abs(ouv.y) > 0.8) return 0;
	if(uv.y < 0.5 && uv.x > 0.5) return tex.SampleLevel(sample, uv, 0);
	float4 c  = float4(0,0,0,0);
	samples = 8;
	for(uint i = 0 ; i < samples; i++)
	{
		c += tex.SampleLevel(sample, uv + float2(rand(input.misc.xy), rand(input.misc.yx)) * 0.00075, i * 0.25);
		input.misc.yx = -input.misc.yx;
	}
	c /= float(samples);
	c.xyz *= 0.717;
	float g = 1;
	
	//fake SSAO
	float depth = c.w;
	if(uv.x       < 0.5)
	{
		const float AO_DRIVE  =  512.0;
		const float AO_STRONG =   16.0;
		const float AO_REP    =   32.0;
		const float RADIUS    =   0.05;
		const float DIST      =  0.001;
		const float NESS      =   16.0;
		const float LODS      =    5.0;
		float2 vp  = input.pos.xy;
		float2 tvp = vp;
		for(int i = 0; i < AO_REP; i++){
			float  h   = i * NESS / AO_REP + 1;
			float  lod = i * LODS / AO_REP;
			float2 tuv = (vp + sin(input.misc.y * input.misc.x + AO_DRIVE * tvp + i) * h) / float2(width, height);
			float  pd  = tex.SampleLevel(sample, tuv, lod).w;
			float  k   = (1 - pd / depth) / h;
			g -= clamp(AO_STRONG * k, 0, (1.0 / AO_REP));
			tvp = -tvp.yx;
			input.misc.xy = -input.misc.yx;
		}
	}
	else 
	{
		const float AO_DRIVE  =  512.0;
		const float AO_STRONG =   16.0;
		const float AO_REP    =   16.0;
		const float RADIUS    =   0.03;
		const float DIST      =  0.005;
		const float NESS      =   15.0;
		const float LODS      =    8.0;
		float2 vp  = input.pos.xy;
		float2 tvp = vp;
		for(int i = 0; i < AO_REP; i++){
			float  h   = i * NESS / AO_REP + 1;
			float  lod = i * LODS / AO_REP;
			float2 tuv   = uv + float2(sin(i * RADIUS), -cos(i * RADIUS)) * DIST;
			float  pd  = tex.SampleLevel(sample, tuv, lod).w;
			float  k   = (1 - pd / depth) / h;
			g -= clamp(AO_STRONG * k, 0, (1.0 / AO_REP));
			tvp = -tvp.yx;
			input.misc.xy = -input.misc.yx;
		}
	}
	g = clamp(g, 00, 1.0);
	float vin = (1.0 - dot(ouv, ouv) * 0.44);
	return sqrt(g * vin * g);
}


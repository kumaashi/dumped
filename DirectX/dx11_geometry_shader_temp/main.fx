#include "param.h"

//--------------------------------------------------------------------------------------
// constant
//--------------------------------------------------------------------------------------
float4       Time;
float4x4     mW, mV, mP;
float        Indicate;
Texture2D    TexDepth;
SamplerState State;

//--------------------------------------------------------------------------------------
// gs data
//--------------------------------------------------------------------------------------
struct GS_OUTPUT {
  float4  pos    : SV_POSITION;
  float3  nor    : NORMAL;
};

struct VS_INPUT {
  float3 pos     : POSITION;
  float3 nor     : NORMAL;
};


//--------------------------------------------------------------------------------------
// output sid
//--------------------------------------------------------------------------------------
GS_OUTPUT vs_main(VS_INPUT input)
{
  GS_OUTPUT ret = (GS_OUTPUT)0;
  ret.pos = float4(input.pos, 1);
  ret.nor = input.nor;
  if(Indicate == 2) return ret;
  
  ret.pos = mul(mW, ret.pos);
  ret.pos = mul(mV, ret.pos);
  ret.pos = mul(mP, ret.pos);
  
  ret.nor = mul(mW, ret.nor);
  ret.nor = mul(mV, ret.nor);
  ret.nor = mul(mP, ret.nor);
 
  return ret;
}

//--------------------------------------------------------------------------------------
// test sprite
//--------------------------------------------------------------------------------------

void App(inout TriangleStream<GS_OUTPUT>c, float3 b){
  GS_OUTPUT a;
  a.nor = b;
  a.pos = float4(b, 1);
  a.pos = mul(mP, a.pos);
  c.Append(a);
}

[maxvertexcount(22)]
void gs_main(inout TriangleStream<GS_OUTPUT> gsstream, triangle GS_OUTPUT v[3]) {
  float3 c = (normalize(cross(v[1].pos - v[0].pos, v[2].pos - v[0].pos)));
  float3 H = c * abs(sin(Time.x) * 5);// * 0.125;
  gsstream.RestartStrip();
  App(gsstream, v[0].pos + H);
  App(gsstream, v[0].pos);
  App(gsstream, v[1].pos + H);
  App(gsstream, v[1].pos);
  App(gsstream, v[2].pos + H);
  App(gsstream, v[2].pos);
  App(gsstream, v[0].pos + H);
  App(gsstream, v[0].pos);
  gsstream.RestartStrip();
  
  App(gsstream, v[0].pos + H);
  App(gsstream, v[1].pos + H);
  App(gsstream, v[2].pos + H);
  gsstream.RestartStrip();
}


//--------------------------------------------------------------------------------------
// put color
//--------------------------------------------------------------------------------------
float4 ps_main( GS_OUTPUT p ) : SV_Target {
  if(Indicate == 0) return float4(p.pos.xyzw);
  float  g = 1;
  float4 tj = TexDepth.Sample(State, p.pos.xy / float2(ScreenX, ScreenY));
  float   j = tj.w;

  for(int b = 0; b < 32;  b++){
    float h = b * 50 / 32.0 + 1;
    g -= clamp(62 * (1 - TexDepth.SampleLevel(State, (p.pos.xy + sin(p.pos.xy + b) * h) / float2(ScreenX, ScreenY),b * 5 / 32.0).w / j ) / h, 0, 1/32.0);
  }

  return g * lerp(
    float4(.1, .3 + sin(Time.x) * 0.1, .5, 1),
    float4(.8, .7, .6, 1),
    pow(j * 0.1, 32) * 0.1);
}


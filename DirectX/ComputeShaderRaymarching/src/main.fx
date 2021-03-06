#include "param.h"

//--------------------------------------------------------------------------------------
// tool
//--------------------------------------------------------------------------------------
void rot(inout float2 d, float a) {
  d = float2(cos(a) * d.x - sin(a) * d.y, sin(a) * d.x + cos(a) * d.y);
}

//--------------------------------------------------------------------------------------
// constant
//--------------------------------------------------------------------------------------
RWStructuredBuffer <float4> buffer  : register(u0);

//http://msdn.microsoft.com/ja-jp/library/ee419707(v=vs.85).aspx (see Start UAV Slot)
RWStructuredBuffer <float4> cstex   : register(u1);
float4                 Time         : register(c0);

//--------------------------------------------------------------------------------------
// gs data
//--------------------------------------------------------------------------------------
struct VS_INPUT {
  float3 pos     : POSITION;
  float3 nor     : NORMAL;
};

//--------------------------------------------------------------------------------------
// output sid
//--------------------------------------------------------------------------------------
float4 vs_main(float3 pos : POSITION) : SV_POSITION {
  return float4(pos, 1);
}

//--------------------------------------------------------------------------------------
//
// raymarching
//
//--------------------------------------------------------------------------------------
//iq's hash
float  hash(float n )         { return frac(sin(n)*43758.5453123); }


float2 rep(float2 p, float x)  { return (abs( (p) % (x*2)) - x);     }
float3 rep(float3 p, float x)  { return (abs( (p) % (x*2)) - x);     }
float2 irep(float2 p, float x) { return (( (p) % (x*2)) - x);     }
float3 irep(float3 p, float x) { return (( (p) % (x*2)) - x);     }

float  fgen(float3 p, float fq, float gain) {
  return gain * (sin(p.x * fq / 2) +  sin(p.y * fq / 2) + sin( 1.7* sin(p.z * fq)));
}
float map(in float3 p) {
  float   k = 2560;
  float3 tp = p + hash(p) * 0.002;
  float  fq = 0;
  fq = 11; float kk  = 0.0125 * (sin(p.x * fq) +  sin(p.y * fq) +  sin(p.z * fq));
  fq  = 6; float kk2 = 0.0325 * (sin(p.x * fq) +  sin(p.y * fq) +  sin(sin(p.z * fq)));
  if(1) {
    float3 ap = tp;
    rot(ap.xy, Time.x * 0.3);
    k = (length(abs(ap % 20) - 10) - 3) + kk + kk2;
  }
  if(0) {
    float k1 = (length(abs(tp.yz % 50) - 25) - 4.5) + kk + kk2;
    float k2 = (length(abs((30. + tp.zx) % 50) - 25) - 4.5) + kk + kk2;
    float k3 = (length(abs((40. + tp.xy) % 50) - 25) - 4.5) + kk + kk2;
    k = min(k1, min(k2, min(k3, k)));
  }
  
  if(1) {
    tp += 5.5;
    float k1 = (length(abs((tp.yz) % 80) - 40) - 5.5) + kk + kk2;
    float k2 = (length(abs((tp.zx) % 100) - 50) - 20.5) + kk + kk2;
    tp.x += sin( 5.4 * sin(tp.z * 0.14 )) * 0.3;
    tp.y += sin( 7.4 * cos(tp.z * 0.12 ));
    float k3 = (length(abs((tp.xy) % 30) - 15) - 2.5) + kk + kk2; 
    k = min(k1, min(k2, min(k3, k)));
    
    if(0)
    {
      float k6 = (length(abs((tp.xy) % 20) - 10) - 5.25) + kk + kk2;;
      float k7 = (length(abs((tp.yz) % 20) - 10) - 7.25) + kk + kk2;;
      float k8 = (length(abs((tp.zx) % 20) - 10) - 5.25) + kk + kk2;;
      k = max(-k6, k);
      k = max(-k7, k);
      k = max(-k8, k);
    }
  }
  return k;
}

float inter(float3 ro, float3 dir, int ite, float cstart, float cend, float mult = 0.95) {
  float  d = cstart;
  for(int i = 0 ; i < ite; i++) {
    float temp = map(ro + (dir * d));
    if(temp < cend) break;
    d += temp * mult;
  }
  return d;
}


float3 getnormal(float3 ip) {
  float2 h      = float2(0.0, 0.01);
  return normalize(
    float3(
      map(ip + h.yxx),
      map(ip + h.xyx),
      map(ip + h.xxy)) - map(ip));
}

void getcamera(in float2 suv, out float3 pos, out float3 dir) {
  dir = normalize(float3(suv * float2(1.25,  1), 1.0));
  float  tm  = Time.x;
  if(1)
  {
    rot(dir.xz, tm * 0.042);
    rot(dir.yz, tm * 0.05);
    pos = float3(0, 5, tm * 13);
  }
  
  if(0)
  {
    rot(dir.xz, 1.2);
    
    pos = float3(0, 2, 13);
  }
}

//--------------------------------------------------------------------------------------
// Compute Shader
//--------------------------------------------------------------------------------------
[numthreads(GroupX, GroupY, 1)]  //tekitou
void cs_main(uint3 tid : SV_DispatchThreadID)
{
  uint   index = tid.x + (tid.y * ScreenX);
  float  tm  = Time.x;
  float  tx  = float(tid.x);
  float  ty  = float(tid.y);
  float x = -1 + (2 * tx / float(ScreenX));
  float y = -1 + (2 * ty / float(ScreenY));
  
	/*
  if(abs(y) > 0.8) {
    buffer[index] = 0;
    return ;
  }
	*/

  float3 pos, dir;
  float2 uv     = float2(x, -y);
  getcamera(uv, pos, dir);
  
  float  d      = inter(pos, dir, 64, 0, 0.03, 1.0);
  if(d > 1024) {
    buffer[index] = d;
    return ;
  }
  float3 ip     = pos + dir * d;
  float3 L1      = normalize(float3(-0.7, -0.5, 0.3)); //dir - > v
  float3 N      = getnormal(ip);
  float  S      = 1;
  if(dot(N, L1) < 0) S += max(inter(ip + N * 1.5, -L1, 8, 0, 0.1), 1); 
  float  D      = max(pow(dot(N, L1), 2), 0.01);
  float3 col    = float3(4, 2, 1) + dir * 0.5;
  float4 result = float4( D * col * S * 0.01, d);
  buffer[index] = result;
}
//--------------------------------------------------------------------------------------
// put color
//--------------------------------------------------------------------------------------
float4 ps_main( float4 p : SV_POSITION ) : SV_Target {
  float2  uv = -1 + 2 * float2(p.x, p.y) / float2(ScreenX, ScreenY);
  float   vignette = (1 - dot(uv * 0.5, uv)) * 0.7;
  uint   index  = uint(p.x + 0) + uint(p.y  + 0) * ScreenX;
  uint   index1 = uint(p.x + 1) + uint(p.y  + 1) * ScreenX;
  uint   index2 = uint(p.x + 1) + uint(p.y  +-1) * ScreenX;
  uint   index3 = uint(p.x +-1) + uint(p.y  + 1) * ScreenX;
  uint   index4 = uint(p.x +-1) + uint(p.y  +-1) * ScreenX;
  float3 result = (cstex[index1] + cstex[index2] + cstex[index3] + cstex[index4]).xyz / 4;
  result = lerp( result, cstex[index].xyz, 0.5 );
  
  float depth = cstex[index].w;
  //result     *= float3(1.4, 1.04, 1.0);
  result    = pow(result, 0.4545);
  
  //fog
  result   = result + (float3(1,2,3) * depth) * 0.004;
  
  //tekitou
  result.x += hash(result.x)  * 0.015;
  result.y += hash(result.y)  * 0.022;
  result.z += hash(result.z)  * 0.031;
  
  return float4(result, 1) * vignette;
}




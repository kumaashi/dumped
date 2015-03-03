#include "param.h"

//--------------------------------------------------------------------------------------
// constant
//--------------------------------------------------------------------------------------
float4 constant[SHADER_Cx_MAX] : register(c0); //b0Ç≈Ç‡OK


//--------------------------------------------------------------------------------------
// gs data
//--------------------------------------------------------------------------------------
struct GS_OUTPUT {
  float4  pos    : SV_POSITION;
  float4  color  : COLOR;
};


//--------------------------------------------------------------------------------------
// output sid
//--------------------------------------------------------------------------------------
float4 vs_main( uint id : SV_VertexID, uint sid : SV_InstanceID ) : POSITION
{
  return float4(sid, sid, sid, sid);
}

//--------------------------------------------------------------------------------------
// test sprite
//--------------------------------------------------------------------------------------
[maxvertexcount(6 * 5 * 2)]
void gs_main(point float4 pos[1] : SV_POSITION, inout TriangleStream<GS_OUTPUT> gsstream ) {
  GS_OUTPUT  op;
  int    size_obj = 2; //float4 * 2...
  int    ip   = OBJ_START_INDEX + (size_obj * pos[0].x);
  int    cp   = OBJ_START_INDEX + (size_obj * pos[0].x) + 1;
  float2 xy   = constant[ip].xy;
  float3 col  = constant[cp].rgb;
  op.color    = float4(col, 1);
  
  float  ang  = constant[cp].w * 5;
  float2x2 rotz = {
    cos(ang), -sin(ang),
    sin(ang),  cos(ang),
  };
  float  scale = 0.02;
  float2 p[4];
  p[0] = mul(rotz, float2(-1,  1)) * scale;
  p[1] = mul(rotz, float2( 1,  1)) * scale;
  p[2] = mul(rotz, float2(-1, -1)) * scale;
  p[3] = mul(rotz, float2( 1, -1)) * scale;
  
  //ê^ÇÒíÜÇÃéläp
  op.pos   = float4(xy + p[0], 0, 1);  gsstream.Append( op );
  op.pos   = float4(xy + p[2], 0, 1);  gsstream.Append( op );
  op.pos   = float4(xy + p[1], 0, 1);  gsstream.Append( op );
  gsstream.RestartStrip();
  op.pos   = float4(xy + p[1], 0, 1);  gsstream.Append( op );
  op.pos   = float4(xy + p[2], 0, 1);  gsstream.Append( op );
  op.pos   = float4(xy + p[3], 0, 1);  gsstream.Append( op );
  gsstream.RestartStrip();

  //é¸ÇËÇÃéläp
  op.color    = float4(1, 1, 1, 1);
  float cscale = 0.7;
  for(int j = 1; j <= 2; j++) {
    for(int i = 0 ; i < 4; i++) {
      float2 k = p[i] * j * 1.5;
      op.pos   = float4(xy + cscale * p[0] + k, 0, 1);  gsstream.Append( op );
      op.pos   = float4(xy + cscale * p[2] + k, 0, 1);  gsstream.Append( op );
      op.pos   = float4(xy + cscale * p[1] + k, 0, 1);  gsstream.Append( op );
      gsstream.RestartStrip();
      op.pos   = float4(xy + cscale * p[1] + k, 0, 1);  gsstream.Append( op );
      op.pos   = float4(xy + cscale * p[2] + k, 0, 1);  gsstream.Append( op );
      op.pos   = float4(xy + cscale * p[3] + k, 0, 1);  gsstream.Append( op );
      gsstream.RestartStrip();
    }
    cscale -= 0.25;
    op.color.xy *= 0.5;
  }
}


//--------------------------------------------------------------------------------------
// put color
//--------------------------------------------------------------------------------------
float4 ps_main( GS_OUTPUT p ) : SV_Target {
  return p.color;
}

//--------------------------------------------------------------------------------------
technique11 Render
{
  pass P0
  {
    SetVertexShader( CompileShader(     vs_4_0, vs_main() ) );
    SetGeometryShader( CompileShader(   gs_4_0, gs_main() ) );
    SetPixelShader( CompileShader(      ps_4_0, ps_main() ) );
  }
}


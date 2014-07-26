//-----------//-----------//-----------//-----------//-----------//-----------
//
//  scene.cpp
//
//-----------//-----------//-----------//-----------//-----------//-----------
#include "common.h"

struct Color {
  float r, g, b, a;
};



//-----------//-----------//-----------//-----------//-----------//-----------
// d3d resource
//-----------//-----------//-----------//-----------//-----------//-----------
//Resource
static ID3D11ComputeShader       *pCShader       = NULL;
static ID3D11Buffer              *pCSBuffer      = NULL;
static ID3D11UnorderedAccessView *pCSUAV         = NULL;
static ShaderConst                constant[4];

static const char *fxfilename = SHADER_FILENAME;

void TermScene() {
  D3DTermPixelShader();
  RELEASE(pCShader);
  RELEASE(pCSBuffer);
  RELEASE(pCSUAV);
}

//-----------//-----------//-----------//-----------//-----------//-----------
//  InitScene
//-----------//-----------//-----------//-----------//-----------//-----------
void InitScene() {
  HRESULT hRet = E_FAIL;
  TermScene();
  D3DInitPixelShader(fxfilename);
  
  //Create Compute Shader
  ID3DBlob  *pBlob = NULL;
  S_RETURN("CompileShaderFromFile CS", D3DCompileShaderFromFile(fxfilename, "cs_main", "cs_5_0", &pBlob));
  S_RETURN("CreateComputeShader",      d3ddevice->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pCShader));
  RELEASE(pBlob);
  
  //Create UAV
  UINT Stride = sizeof(Color);
  S_RETURN("D3DCreateBufferUAV CS", D3DCreateBufferUAV(&pCSBuffer, &pCSUAV, ScreenX * ScreenY * Stride, Stride));
  printf("Loaded.\n");
  
  //--------------------------------------------------------------------------------------------------------------
  //Create pConstant Buffer
  //--------------------------------------------------------------------------------------------------------------
  S_RETURN("D3DCreateBuffer:pConstant", D3DCreateBuffer(&pConstant, D3D11_BIND_CONSTANT_BUFFER, sizeof(constant)));

}



//-----------//-----------//-----------//-----------//-----------//-----------
// UpdateScene
//-----------//-----------//-----------//-----------//-----------//-----------
void UpdateScene() {
  static unsigned long t = timeGetTime();
  unsigned long tt = timeGetTime();
  FLOAT delta = (FLOAT)(tt - t) / 1000.0f;
  t = tt;
  constant[0].Const.x += delta;
  //constant[0].Const.x += 0.0166666666666666666666666;
  
}

//-----------//-----------//-----------//-----------//-----------//-----------
// RenderScene
//-----------//-----------//-----------//-----------//-----------//-----------
void RenderScene() {
  HRESULT hRet = 0;
  d3dcontext->UpdateSubresource(pConstant, 0, NULL, constant, 0, 0);
  d3dcontext->CSSetConstantBuffers(0, 1, &pConstant);
  d3dcontext->CSSetShader(pCShader, NULL, 0);
  d3dcontext->CSSetUnorderedAccessViews(0, 1, &pCSUAV, NULL);
  
  printf("Dispatch X=%d,  y=%d\r", ScreenX / GroupX, ScreenY / GroupY);
  d3dcontext->Dispatch(ScreenX / GroupX, ScreenY / GroupY, 1 );
  
  d3dcontext->OMSetRenderTargetsAndUnorderedAccessViews( 1, &pBackRTV, NULL, 1, 1, &pCSUAV, NULL);
  D3DPresent(0);
}

//-----------//-----------//-----------//-----------//-----------//-----------
//
// DoScene
//
//-----------//-----------//-----------//-----------//-----------//-----------
void DoScene() {
  show_fps();
  UpdateScene();
  if(GetAsyncKeyState(VK_F5)) {
    InitScene();
  }
  if(pVShader && pPShader && pConstant && pCShader) {
    RenderScene();
  }
}


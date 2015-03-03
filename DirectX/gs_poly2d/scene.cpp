//------------------------------------------------------------------------------
//
//  scene.cpp
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  include
//------------------------------------------------------------------------------
#include "common.h"

ID3D11VertexShader    *vshader   = NULL;
ID3D11GeometryShader  *gshader   = NULL;
ID3D11PixelShader     *pshader   = NULL;
ID3DBlob              *pBlobVS   = NULL;
ID3DBlob              *pBlobGS   = NULL;
ID3DBlob              *pBlobPS   = NULL;
ID3D11Buffer          *pConstant = NULL;
ID3D11RasterizerState *pRSStage  = NULL;
char *fxfilename = "main.fx";
XMFLOAT4  constant[SHADER_Cx_MAX];

//------------------------------------------------------------------------------
// random
//------------------------------------------------------------------------------
int _rand() {
  static int a = 1;
  static int b = 234567;
  static int c = 890123;
  a += b; b += c; c += a;
  return (a >> 16);
}

float frand() {
  return (float)(_rand()) / 32767.0f;
}


//------------------------------------------------------------------------------
//  test obj
//------------------------------------------------------------------------------
struct Obj {
  XMFLOAT2 pos;
  XMFLOAT2 dir;
  XMFLOAT4 data;
};
Obj obj[OBJ_MAX];
void obj_init() {
  for(int i = 0 ; i < OBJ_MAX; i++) {
    Obj *w = &(obj[i]);
    w->pos  = XMFLOAT2(frand(), frand());
    w->dir  = XMFLOAT2(frand() * 0.005, frand() * 0.005);
    w->data = XMFLOAT4(abs(frand()), abs(frand()), abs(frand()), abs(frand()));
  }
}

void obj_update() {
  Obj *dest = (Obj *)&(constant[OBJ_START_INDEX]);
  for(int i = 0 ; i < OBJ_MAX; i++) {
    Obj *w = &(obj[i]);
    w->pos.x  += w->dir.x;
    w->pos.y  += w->dir.y;
    w->data.w += 0.01;
    if(abs(w->pos.x) > 1) w->dir.x = -w->dir.x;
    if(abs(w->pos.y) > 1) w->dir.y = -w->dir.y;
    dest[i] = *w;
  }
}


//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut ) {
  HRESULT hr = S_OK;
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
  ID3DBlob* pErrorBlob = NULL;
  hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
      dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
  if( FAILED(hr) ) {
    char *errstr = (char*)pErrorBlob->GetBufferPointer();
    OutputDebugString(errstr);
    printf("ERROR : %s\n", errstr);
  }
  RELEASE( pErrorBlob );
  return hr;
}


//--------------------------------------------------------------------------------------
// D3DXCreateBuffer
//--------------------------------------------------------------------------------------
HRESULT D3DXCreateBuffer(UINT BindFlags, UINT ByteWidth, ID3D11Buffer **ppBuffer, D3D11_SUBRESOURCE_DATA *data) {
  HRESULT hRet = E_FAIL;
  D3D11_BUFFER_DESC bd = { ByteWidth, D3D11_USAGE_DEFAULT, BindFlags, 0, 0, 0  };
  hRet = d3ddevice->CreateBuffer( &bd, data, ppBuffer );
  return hRet;
}


//------------------------------------------------------------------------------
//
//  function definition
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  InitScene
//------------------------------------------------------------------------------
void InitScene() {
  HRESULT hRet = E_FAIL;
  TermScene();
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
  S_RETURN("CompileShaderFromFile VS", CompileShaderFromFile(fxfilename, "vs_main", "vs_4_0", &pBlobVS));
  S_RETURN("CompileShaderFromFile GS", CompileShaderFromFile(fxfilename, "gs_main", "gs_4_0", &pBlobGS));
  S_RETURN("CompileShaderFromFile PS", CompileShaderFromFile(fxfilename, "ps_main", "ps_4_0", &pBlobPS));
  S_RETURN("CreateVertexShader", d3ddevice->CreateVertexShader(pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &vshader));
  S_RETURN("CreateGeometryShader", d3ddevice->CreateGeometryShader(pBlobGS->GetBufferPointer(), pBlobGS->GetBufferSize(), NULL, &gshader));
  S_RETURN("CreatePixelShader", d3ddevice->CreatePixelShader(pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &pshader));
  RELEASE(pBlobVS);
  RELEASE(pBlobGS);
  RELEASE(pBlobPS);
  
  D3D11_RASTERIZER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.FillMode = D3D11_FILL_SOLID;
  desc.CullMode = D3D11_CULL_NONE; //D3D11_CULL_BACK
  desc.DepthClipEnable = TRUE;
  S_RETURN("CreateRasterizerState", d3ddevice->CreateRasterizerState(&desc, &pRSStage));

  ZeroMemory(constant, sizeof(constant));
  S_RETURN("D3DXCreateBuffer : pConstant", D3DXCreateBuffer(D3D11_BIND_CONSTANT_BUFFER, sizeof(constant), &pConstant, NULL));
  printf("Loaded.\n");
  
  obj_init();
  printf("Obj Initialize.\n");
}

//------------------------------------------------------------------------------
//  TermScene
//------------------------------------------------------------------------------
void TermScene() {
  RELEASE(pConstant);
  RELEASE(pRSStage);
  RELEASE(pBlobPS);
  RELEASE(pBlobGS);
  RELEASE(pBlobVS);
  RELEASE(pshader);
  RELEASE(gshader);
  RELEASE(vshader);
}


//------------------------------------------------------------------------------
//  DoScene
//------------------------------------------------------------------------------
void DoScene() {
  if(GetAsyncKeyState(VK_F5)) {
    InitScene();
  }
  
  if(vshader && gshader && pshader && pConstant && pRSStage) {
    static unsigned long t = timeGetTime();
    unsigned long now = timeGetTime();
    if( (now - t) > 1000) {
      t = timeGetTime();
    }
    obj_update();
    constant[0].x += 0.016666666;
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
    d3dcontext->ClearRenderTargetView(d3drtv, ClearColor );
    d3dcontext->RSSetState(pRSStage);
    d3dcontext->UpdateSubresource(pConstant, 0, NULL, constant, 0, 0);
    d3dcontext->VSSetShader(vshader, NULL, 0 );
    d3dcontext->GSSetShader(gshader, NULL, 0 );
    d3dcontext->PSSetShader(pshader, NULL, 0 );
    d3dcontext->VSSetConstantBuffers(0, 1, &pConstant );
    d3dcontext->GSSetConstantBuffers(0, 1, &pConstant );
    d3dcontext->PSSetConstantBuffers(0, 1, &pConstant );
    d3dcontext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
    d3dcontext->IASetInputLayout(NULL);
    d3dcontext->DrawInstanced(1, OBJ_MAX, 0, 0);
    d3dswapchain->Present(0, 0 );
  } else {
    printf("vshader = %08X\n", vshader);
    printf("gshader   =%08X\n",gshader  );
    printf("pshader   =%08X\n",pshader  );
    printf("pConstant =%08X\n",pConstant);
    printf("pRSStage  =%08X\n",pRSStage );
  }
}


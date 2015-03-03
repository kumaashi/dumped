//-----------//-----------//-----------//-----------//-----------//-----------
//
//
//  scene.cpp
//
//
//-----------//-----------//-----------//-----------//-----------//-----------
#include "common.h"

//-----------//-----------//-----------//-----------//-----------//-----------
//
// d3d resource
//
//-----------//-----------//-----------//-----------//-----------//-----------
//d3d11 device
static ID3D11Device              *d3ddevice            = NULL;
static ID3D11DeviceContext       *d3dcontext           = NULL;
static IDXGISwapChain            *d3dswapchain         = NULL;
static ID3D11Texture2D           *pBackTexture         = NULL;
static ID3D11RenderTargetView    *pBackRTV             = NULL;
static ID3D11Texture2D           *pDSVTexture          = NULL;
static ID3D11DepthStencilView    *pDSV                 = NULL;
static ID3D11Texture2D           *pDepthTexture        = NULL;
static ID3D11RenderTargetView    *pDepthRTV            = NULL;
static ID3D11ShaderResourceView  *pDepthSRV            = NULL;
static ID3D11SamplerState        *pSamplerLinear       = NULL;

//d3d9
static LPDIRECT3D9               d3d9                  = NULL;
static LPDIRECT3DDEVICE9         d3d9device            = NULL;


//Resource
static ID3D11VertexShader        *pVShader             = NULL;
static ID3D11GeometryShader      *pGShader             = NULL;
static ID3D11PixelShader         *pPShader             = NULL;
static ID3DBlob                  *pBlobVS              = NULL;
static ID3DBlob                  *pBlobGS              = NULL;
static ID3DBlob                  *pBlobPS              = NULL;
static ID3D11Buffer              *pConstant            = NULL;
static ID3D11RasterizerState     *pRSStage             = NULL;
static ID3D11InputLayout         *pVLayout             = NULL;
static ID3D11Buffer              *pVertexScreen        = NULL;


HRESULT D3DX11CreateBuffer(UINT BindFlags, UINT ByteWidth, ID3D11Buffer **ppBuffer, D3D11_SUBRESOURCE_DATA *data);


//-----------//-----------//-----------//-----------//-----------//-----------
//
// struct
//
//-----------//-----------//-----------//-----------//-----------//-----------
struct ShaderConst {
  XMFLOAT4 Const;
  XMMATRIX mWorld;
  XMMATRIX mView;
  XMMATRIX mProj;
  FLOAT    Indicate;
};

struct VertexData {
  D3DXVECTOR3 Position;
  D3DXVECTOR3 Color;
};


//-----------//-----------//-----------//-----------//-----------//-----------
//
// table
//
//-----------//-----------//-----------//-----------//-----------//-----------
DXGI_SWAP_CHAIN_DESC    d3dsddesc = {
  //DXGI_MODE_DESC BufferDesc
  { ScreenX, ScreenY, { 60, 1 },
    DXGI_FORMAT_R8G8B8A8_UNORM, 
    DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
    DXGI_MODE_SCALING_UNSPECIFIED,  },
  {1, 0},                                   //DXGI_SAMPLE_DESC SampleDesc
  DXGI_USAGE_RENDER_TARGET_OUTPUT,          //DXGI_USAGE BufferUsage
  1,                                        //UINT  BufferCount
  NULL,                                     //HWND  OutputWindow
  TRUE,                                     //BOOL  Windowed
  DXGI_SWAP_EFFECT_DISCARD,                 //DXGI_SWAP_EFFECT SwapEffect
  DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH,   //UINT Flags -> DXGI_SWAP_CHAIN_FLAG
};

static const  D3D11_INPUT_ELEMENT_DESC layout[] = {
  { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static const FLOAT ScreenVertex[] = {
  -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
   1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
  -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
   1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
};

static const  D3D11_VIEWPORT vp = {
  0.0f, 0.0f, (FLOAT)ScreenX, (FLOAT)ScreenY, 0.0f, 1.0f,
};

static const D3D11_SAMPLER_DESC sampDesc = {
  D3D11_FILTER_MIN_MAG_MIP_LINEAR,
  D3D11_TEXTURE_ADDRESS_WRAP,
  D3D11_TEXTURE_ADDRESS_WRAP,
  D3D11_TEXTURE_ADDRESS_WRAP,
  0, 0, D3D11_COMPARISON_NEVER,
  {0.0f, 0.0f, 0.0f, 0.0f},
  0,
  D3D11_FLOAT32_MAX
};

static const D3D11_TEXTURE2D_DESC    descDSVTexture = {
  ScreenX, ScreenY, 1, 1,
  DXGI_FORMAT_D32_FLOAT,
  {1, 0},
  D3D11_USAGE_DEFAULT,
  D3D11_BIND_DEPTH_STENCIL,
  0,  0,
};

static const  D3D11_TEXTURE2D_DESC   descDepthTexture = {
  ScreenX, ScreenY, 1, 1,
  //DXGI_FORMAT_R32_FLOAT,
  DXGI_FORMAT_R32G32B32A32_FLOAT,
  {1, 0},
  D3D11_USAGE_DEFAULT,
  D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
  0,  0,
};

static const D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {
  descDSVTexture.Format,  D3D11_DSV_DIMENSION_TEXTURE2D,
  //ZI tabun.
};

static const D3D11_SHADER_RESOURCE_VIEW_DESC descDepthSRV = {
  descDepthTexture.Format, D3D11_SRV_DIMENSION_TEXTURE2D,
  {0, descDepthTexture.MipLevels} //Texture2D : damekamo
  /*
  descDepthSRV.Texture2D.MostDetailedMip = 0;
  descDepthSRV.Texture2D.MipLevels       = descDepthTexture.MipLevels;
  */
};

static D3DPRESENT_PARAMETERS d3dpp = {
  0, 0, D3DFMT_X8R8G8B8, 1, D3DMULTISAMPLE_NONE, 0,
  D3DSWAPEFFECT_DISCARD, 0, TRUE, TRUE, D3DFMT_D16, 0, 0, TRUE,
};


//-----------//-----------//-----------//-----------//-----------//-----------
//
// custom
//
//-----------//-----------//-----------//-----------//-----------//-----------
static const char *fxfilename = SHADER_FILENAME;
static ShaderConst constant[4];
static D3DX11MeshData mesh;


//-----------//-----------//-----------//-----------//-----------//-----------
//
//  func
//
//-----------//-----------//-----------//-----------//-----------//-----------

HRESULT TermDevice() {
  D3DX11ReleaseMesh(&mesh);
  RELEASE(d3d9device);
  RELEASE(d3d9);
  RELEASE(pSamplerLinear);
  RELEASE(pDepthSRV);
  RELEASE(pDepthRTV);
  RELEASE(pDepthTexture);
  RELEASE(pDSV);
  RELEASE(pDSVTexture);
  RELEASE(pBackRTV);
  RELEASE(pBackTexture);
  RELEASE(d3dswapchain);
  RELEASE(d3dcontext);
  RELEASE(d3ddevice);
  return S_OK;
}

void TermScene() {
  D3DX11ReleaseMesh(&mesh);
  RELEASE(pVertexScreen);
  RELEASE(pVShader);
  RELEASE(pGShader);
  RELEASE(pPShader);
  RELEASE(pBlobVS);
  RELEASE(pBlobGS);
  RELEASE(pBlobPS);
  RELEASE(pConstant);
  RELEASE(pRSStage);
  RELEASE(pVLayout);
}

//-----------//-----------//-----------//-----------//-----------//-----------
// util
//-----------//-----------//-----------//-----------//-----------//-----------
DXGI_SWAP_CHAIN_DESC D3DXGetSwapChainDesc(HWND hWnd, UINT Width, UINT Height) {
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory( &sd, sizeof( sd ) );
  sd.BufferCount        = 1;
  sd.BufferDesc.Width   = Width;
  sd.BufferDesc.Height  = Height;
  sd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferDesc.RefreshRate.Numerator   = 60;
  sd.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow       = hWnd;
  sd.SampleDesc.Count   = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed           = TRUE;
  sd.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
  sd.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  return sd;
}

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

//-----------//-----------//-----------//-----------//-----------//-----------
//
// D3DXMESH -> D3DX11MeshData
//
//-----------//-----------//-----------//-----------//-----------//-----------
void D3DX11ReleaseMesh(D3DX11MeshData *data) {
  if(!data) return;
  RELEASE(data->pIB);
  RELEASE(data->pVB);
  ZeroMemory(data, sizeof(D3DX11MeshData));
}


#define D3DX11SCRATCH_BUFFER (sizeof(FLOAT) * 65536 * 3)
static FLOAT scratch_float[D3DX11SCRATCH_BUFFER];
static WORD  scratch_index[D3DX11SCRATCH_BUFFER];
HRESULT D3DX11CreateMesh(int type, D3DX11MeshData *data) {
  if(!data || data->pVB || data->pIB) return E_FAIL;
  LPVOID lp;
  D3D11_SUBRESOURCE_DATA InitData;
  LPD3DXMESH             lpTemp = NULL;
  LPD3DXMESH             lpMesh = NULL;
  
  /*
  D3DXCreateTeapot(d3d9device, &lpTemp, 0);
  D3DXTessellateNPatches(lpTemp, NULL, 2.0f, FALSE, &lpMesh, NULL);
  D3DXComputeNormals(lpMesh, NULL);
  */
  D3DXCreateTeapot(d3d9device, &lpMesh, 0);
  if(lpMesh) {
    data->NumVertices       = lpMesh->GetNumVertices();
    data->NumIndex          = lpMesh->GetNumFaces() * 3;
    
    //Vertex
    if(lpMesh->LockVertexBuffer(D3DLOCK_READONLY, &lp) == S_OK) {
      memcpy(scratch_float, lp, lpMesh->GetNumBytesPerVertex() * data->NumVertices);
      lpMesh->UnlockVertexBuffer();
      VertexData *p = (VertexData *)scratch_float;
      for(DWORD i = 0 ; i < data->NumVertices; i++) {
        p->Position += D3DXVECTOR3(frand(), frand(), frand()) * 0.0005f;
        p++;
      }
    }
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = scratch_float;
    D3DX11CreateBuffer(D3D11_BIND_VERTEX_BUFFER, lpMesh->GetNumBytesPerVertex() * data->NumVertices, &data->pVB, &InitData);
    
    //Index
    if(lpMesh->LockIndexBuffer(D3DLOCK_READONLY, &lp) == S_OK) {
      memcpy(scratch_index, lp, data->NumIndex * sizeof(WORD));
      lpMesh->UnlockIndexBuffer();
    }
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = scratch_index;
    D3DX11CreateBuffer(D3D11_BIND_INDEX_BUFFER, sizeof(WORD) * data->NumIndex , &data->pIB, &InitData);
  }
  RELEASE(lpMesh);
  RELEASE(lpTemp);
  return S_OK;
}


//-----------//-----------//-----------//-----------//-----------//-----------
// Find and compile the specified shader
//-----------//-----------//-----------//-----------//-----------//-----------
HRESULT CompileShaderFromFile( LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut ) {
  HRESULT hr = S_OK;
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
  ID3DBlob* pErrorBlob = NULL;
  hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
  if(FAILED(hr)) {
    printf("ERROR:%s : %s\n", __FUNCTION__, pErrorBlob->GetBufferPointer());
  }
  RELEASE( pErrorBlob );
  return hr;
}

//-----------//-----------//-----------//-----------//-----------//-----------
// D3DX11CreateBuffer
//-----------//-----------//-----------//-----------//-----------//-----------
HRESULT D3DX11CreateBuffer(UINT BindFlags, UINT ByteWidth, ID3D11Buffer **ppBuffer, D3D11_SUBRESOURCE_DATA *data) {
  HRESULT hRet = E_FAIL;
  D3D11_BUFFER_DESC bd = { ByteWidth, D3D11_USAGE_DEFAULT, BindFlags, 0, 0, 0  };
  hRet = d3ddevice->CreateBuffer( &bd, data, ppBuffer );
  return hRet;
}

//------------------------------------------------------------------------------
// Initialize DirectX11
//------------------------------------------------------------------------------
HRESULT InitDevice(HWND hWnd, UINT width ,UINT height) {
  HRESULT hRet = E_FAIL;
  
  //------------//------------//------------//------------
  //Create DirectX11
  //------------//------------//------------//------------
  //d3dsddesc = D3DXGetSwapChainDesc(hWnd, width, height);
  d3dsddesc.OutputWindow = hWnd;
  E_RETURN("D3D11CreateDeviceAndSwapChain",
    D3D11CreateDeviceAndSwapChain(
      NULL, D3D_DRIVER_TYPE_HARDWARE,
      NULL, 0, NULL, 0,
      D3D11_SDK_VERSION,
      &d3dsddesc, &d3dswapchain, &d3ddevice, NULL, &d3dcontext));
  
  //------------//------------//------------//------------
  //Setup BackBuffer
  //------------//------------//------------//------------
  E_RETURN("d3dswapchain GetBuffer",
    d3dswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackTexture ));
  
  E_RETURN("CreateRenderTargetView Back Buffer",
    d3ddevice->CreateRenderTargetView(pBackTexture, NULL, &pBackRTV ));
  
  //------------//------------//------------//------------
  //Setup Depth Stencil Buffer
  //------------//------------//------------//------------
  E_RETURN("CreateTexture2D DepthStencilView",
    d3ddevice->CreateTexture2D(&descDSVTexture, NULL, &pDSVTexture ));
  
  E_RETURN("CreateDepthStencilView",
    d3ddevice->CreateDepthStencilView(pDSVTexture, &descDSV, &pDSV ));
  
  //------------//------------//------------//------------
  //Setup RenderTargetTexture Depth Buffer
  //------------//------------//------------//------------
  E_RETURN("CreateTexture2D",
    d3ddevice->CreateTexture2D(&descDepthTexture, NULL, &pDepthTexture ));
  
  E_RETURN("CreateRenderTargetView Depth Buffer",
    d3ddevice->CreateRenderTargetView(pDepthTexture, NULL, &pDepthRTV));
  
  //------------//------------//------------//------------
  //Create DepthBuffer Shader Texture View
  //------------//------------//------------//------------
  E_RETURN("CreateShaderResourceView Depth Buffer",
    d3ddevice->CreateShaderResourceView(pDepthTexture, &descDepthSRV, &pDepthSRV ));
  
  //------------//------------//------------//------------
  //Create SampleState
  //------------//------------//------------//------------
  E_RETURN("CreateSamplerState",
    d3ddevice->CreateSamplerState(&sampDesc, &pSamplerLinear ));
  
  //------------//------------//------------//------------
  //Create DirectX9
  //------------//------------//------------//------------
  E_NULL_RETURN("Direct3DCreate9",
    d3d9 = Direct3DCreate9(D3D_SDK_VERSION));
  
  E_RETURN("CreateDevice D3D9",
    d3d9->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3d9device));
  
  //Setup View Port
  d3dcontext->RSSetViewports(1, &vp );
  return S_OK;
}


//-----------//-----------//-----------//-----------//-----------//-----------
//  InitScene
//-----------//-----------//-----------//-----------//-----------//-----------
void InitScene() {
  HRESULT hRet = E_FAIL;
  TermScene();
  
  //Create Shader Resource
  ZeroMemory(constant, sizeof(constant));
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
  S_RETURN("CompileShaderFromFile VS",
    CompileShaderFromFile(fxfilename, "vs_main", "vs_4_0", &pBlobVS));
  S_RETURN("CompileShaderFromFile GS",
    CompileShaderFromFile(fxfilename, "gs_main", "gs_4_0", &pBlobGS));
  S_RETURN("CompileShaderFromFile PS",
    CompileShaderFromFile(fxfilename, "ps_main", "ps_4_0", &pBlobPS));
  S_RETURN("CreateVertexShader",
    d3ddevice->CreateVertexShader(pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &pVShader));
  S_RETURN("CreateGeometryShader",
    d3ddevice->CreateGeometryShader(pBlobGS->GetBufferPointer(), pBlobGS->GetBufferSize(), NULL, &pGShader));
  S_RETURN("CreatePixelShader",
    d3ddevice->CreatePixelShader(pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &pPShader));
  
  //Create Constant Buffer
  S_RETURN("D3DX11CreateBuffer:pConstant",
    D3DX11CreateBuffer(D3D11_BIND_CONSTANT_BUFFER, sizeof(constant), &pConstant, NULL));
  
  //Create Input Layout
  UINT numElements = ARRAYSIZE( layout );
  S_RETURN("CreateInputLayout",
    d3ddevice->CreateInputLayout(layout, numElements, pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), &pVLayout ));
  
  
  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData) );
  InitData.pSysMem = ScreenVertex;
  S_RETURN("D3DXCreateBuffer : pVertexScreen",
    D3DX11CreateBuffer(D3D11_BIND_VERTEX_BUFFER, sizeof(ScreenVertex), &pVertexScreen, &InitData));
  
  D3DX11CreateMesh(0, &mesh);
  printf("Loaded.\n");
}

//-----------//-----------//-----------//-----------//-----------//-----------
// SetupMeshData
//-----------//-----------//-----------//-----------//-----------//-----------
void SetupMeshData(D3DX11MeshData *data) {
  UINT stride = sizeof( VertexData );
  UINT offset = 0;
  d3dcontext->IASetVertexBuffers( 0, 1, &data->pVB, &stride, &offset );
  d3dcontext->IASetIndexBuffer( data->pIB, DXGI_FORMAT_R16_UINT, 0 );
  d3dcontext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ); //D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, D3D11_PRIMITIVE_TOPOLOGY_LINELIST
}

//-----------//-----------//-----------//-----------//-----------//-----------
// SetupContextResource
//-----------//-----------//-----------//-----------//-----------//-----------
void SetupContextResource() {
  SetupMeshData(&mesh);
  d3dcontext->VSSetShader(pVShader, NULL, 0);
  d3dcontext->GSSetShader(pGShader, NULL, 0);
  d3dcontext->PSSetShader(pPShader, NULL, 0);
  d3dcontext->VSSetConstantBuffers(0, 1, &pConstant);
  d3dcontext->GSSetConstantBuffers(0, 1, &pConstant);
  d3dcontext->PSSetConstantBuffers(0, 1, &pConstant);
  d3dcontext->PSSetSamplers(0, 1, &pSamplerLinear );
}

//-----------//-----------//-----------//-----------//-----------//-----------
// UpdateAndSetShaderResource
//-----------//-----------//-----------//-----------//-----------//-----------
void UpdateAndSetShaderResource() {
  d3dcontext->UpdateSubresource(pConstant, 0, NULL, constant, 0, 0);
  d3dcontext->PSSetShaderResources(0, 1, &pDepthSRV);
}

//-----------//-----------//-----------//-----------//-----------//-----------
// ClearView
//-----------//-----------//-----------//-----------//-----------//-----------
void ClearView(ID3D11RenderTargetView *pRTV, ID3D11DepthStencilView *pDSV, FLOAT ColorList[4]) {
  d3dcontext->ClearRenderTargetView(pRTV, ColorList);
  d3dcontext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

//-----------//-----------//-----------//-----------//-----------//-----------
// DrawMeshDepth
//-----------//-----------//-----------//-----------//-----------//-----------
void DrawMeshDepth() {
  float ClearColorDepth[4] = { 1.0f, 1.0f,   1.0f, 1.0f };
  ClearView(pDepthRTV, pDSV, ClearColorDepth);
  d3dcontext->OMSetRenderTargets( 1, &pDepthRTV, pDSV);
  SetupContextResource();
  ShaderConst *shader_const = &(constant[0]);
  shader_const->Indicate = 0;
  UpdateAndSetShaderResource();
  d3dcontext->DrawIndexed(mesh.NumIndex, 0, 0);
}

//-----------//-----------//-----------//-----------//-----------//-----------
// DrawMeshBackBuffer
//-----------//-----------//-----------//-----------//-----------//-----------
void DrawMeshBackBuffer() {
  float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
  ClearView(pBackRTV, pDSV, ClearColor);
  d3dcontext->OMSetRenderTargets( 1, &pBackRTV, pDSV);
  SetupContextResource();
  ShaderConst *shader_const = &(constant[0]);
  shader_const->Indicate = 1234;
  
  UpdateAndSetShaderResource();
  d3dcontext->DrawIndexed(mesh.NumIndex, 0, 0);
}

//-----------//-----------//-----------//-----------//-----------//-----------
// DrawPostProcess
//-----------//-----------//-----------//-----------//-----------//-----------
void DrawPostProcess() {
  float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
  ClearView(pBackRTV, pDSV, ClearColor);
  d3dcontext->OMSetRenderTargets( 1, &pBackRTV, NULL);
  
  //Draw rect angle.
  UINT stride = sizeof( ScreenVertex ) / 4;
  UINT offset = 0;
  d3dcontext->IASetIndexBuffer( NULL, DXGI_FORMAT_UNKNOWN, 0 );
  d3dcontext->IASetVertexBuffers( 0, 1, &pVertexScreen, &stride, &offset );
  d3dcontext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
  d3dcontext->IASetInputLayout(pVLayout);
  
  ShaderConst *shader_const = &(constant[0]);
  shader_const->Indicate = 2;
  UpdateAndSetShaderResource();
  d3dcontext->GSSetShader(NULL, NULL, 0);
  d3dcontext->Draw(4, 0);
}


//-----------//-----------//-----------//-----------//-----------//-----------
// SetupCamera
//-----------//-----------//-----------//-----------//-----------//-----------
void SetupCamera() {
  ShaderConst *shader_const = &(constant[0]);
  FLOAT k = constant[0].Const.x;
  
  static FLOAT ux = 0;
  static FLOAT uz = 0;
  static FLOAT dux = 0;
  static FLOAT duz = 0;
  
  dux *= 0.9;
  duz *= 0.9;
  //if(GetAsyncKeyState(VK_LEFT)  & 0x8000) dux -= 0.01;
  //if(GetAsyncKeyState(VK_RIGHT) & 0x8000) dux += 0.01;
  //if(GetAsyncKeyState(VK_UP)    & 0x8000) duz += 0.01;
  //if(GetAsyncKeyState(VK_DOWN)  & 0x8000) duz -= 0.01;
  ux += dux;  uz += duz;
  XMVECTOR Eye = XMVectorSet( sin(k * 0.3)*10 + ux, -sin(k * 0.4)*10 , cos(k * 0.5)*10 + uz, 0.0f );
  XMVECTOR At  = XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f );
  XMVECTOR Up  = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
  shader_const->mView  = (XMMatrixLookAtLH( Eye, At, Up ));
  shader_const->mProj  = (XMMatrixPerspectiveFovLH( XM_PIDIV4, 1.25f, 0.01f, 100.0f ));
  shader_const->mWorld = (XMMatrixRotationY( k * 0.3 ));
  shader_const->Indicate = 0;
}


//-----------//-----------//-----------//-----------//-----------//-----------
// UpdateScene
//-----------//-----------//-----------//-----------//-----------//-----------
void UpdateScene() {
  static unsigned long t = timeGetTime();
  unsigned long now = timeGetTime();
  if( (now - t) > 1000) {
    t = timeGetTime();
  }
  constant[0].Const.x += 0.016666666;
  SetupCamera();
}

//-----------//-----------//-----------//-----------//-----------//-----------
// RenderScene
//-----------//-----------//-----------//-----------//-----------//-----------
void RenderScene() {
  DrawMeshDepth();
  DrawPostProcess();
  d3dswapchain->Present(0, 0 );
}

//-----------//-----------//-----------//-----------//-----------//-----------
// DebugPrint
//-----------//-----------//-----------//-----------//-----------//-----------
void DebugPrint() {
  printf("pVShader   = %08X\n", pVShader);
  printf("pGShader   = %08X\n", pGShader);
  printf("pPShader   = %08X\n", pPShader);
  printf("pConstant  = %08X\n", pConstant);
  printf("pRSStage   = %08X\n", pRSStage );
}

//-----------//-----------//-----------//-----------//-----------//-----------
//
// DoScene
//
//-----------//-----------//-----------//-----------//-----------//-----------
void DoScene() {
  if(GetAsyncKeyState(VK_F5)) {
    InitScene();
  }
  if(pVShader && pGShader && pPShader && pConstant) {
    UpdateScene();
    RenderScene();
  } else {
    DebugPrint();
  }
}


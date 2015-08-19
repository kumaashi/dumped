//--------------------------------------------------------------------------------------------------------------
//
//  d3d.cpp
//
//--------------------------------------------------------------------------------------------------------------
#include "common.h"

//--------------------------------------------------------------------------------------------------------------
//  Resource
//--------------------------------------------------------------------------------------------------------------
ID3D11Device                     *d3ddevice            = NULL;
ID3D11DeviceContext              *d3dcontext           = NULL;
IDXGISwapChain                   *d3dswapchain         = NULL;
ID3D11Texture2D                  *pBackTexture         = NULL;
ID3D11RenderTargetView           *pBackRTV             = NULL;
ID3D11VertexShader               *pVShader             = NULL;
ID3D11PixelShader                *pPShader             = NULL;
ID3D11Buffer                     *pConstant            = NULL;
ID3D11InputLayout                *pVLayout             = NULL;
ID3D11Buffer                     *pVertexScreen        = NULL;

//--------------------------------------------------------------------------------------------------------------
//  tekitopu
//--------------------------------------------------------------------------------------------------------------
DXGI_SWAP_CHAIN_DESC    d3dsddesc = {
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
};

static const FLOAT ScreenVertex[] = {
  -1.0f,  1.0f, 1.0f, 
   1.0f,  1.0f, 1.0f, 
  -1.0f, -1.0f, 1.0f, 
   1.0f, -1.0f, 1.0f, 
};

static const  D3D11_VIEWPORT vp = {
  0.0f, 0.0f, (FLOAT)ScreenX, (FLOAT)ScreenY, 0.0f, 1.0f,
};

static const D3D11_SHADER_RESOURCE_VIEW_DESC descTexture = {
  DXGI_FORMAT_R32G32B32A32_FLOAT,
  D3D11_SRV_DIMENSION_TEXTURE2D, {0, 1}
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

//--------------------------------------------------------------------------------------------------------------
//  D3DGetSwapChainDesc
//--------------------------------------------------------------------------------------------------------------
DXGI_SWAP_CHAIN_DESC D3DGetSwapChainDesc(HWND hWnd, UINT Width, UINT Height) {
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


//--------------------------------------------------------------------------------------------------------------
//
//  D3DTermDevice
//
//--------------------------------------------------------------------------------------------------------------
HRESULT D3DTermDevice() {
  D3DTermPixelShader();
  RELEASE(pVShader);
  RELEASE(pConstant);
  RELEASE(pVLayout);
  RELEASE(pVertexScreen);
  RELEASE(pBackRTV);
  RELEASE(pBackTexture);
  RELEASE(d3dswapchain);
  RELEASE(d3dcontext);
  RELEASE(d3ddevice);
  return S_OK;
}

void D3DTermPixelShader() {
  RELEASE(pPShader);
}

HRESULT D3DInitPixelShader(LPCSTR fxfilename) {
  //--------------------------------------------------------------------------------------------------------------
  //Create PixelShader
  //--------------------------------------------------------------------------------------------------------------
  ID3DBlob  *pBlob = NULL;
  E_RETURN("CompileShaderFromFile PS", D3DCompileShaderFromFile(fxfilename, "ps_main", "ps_5_0", &pBlob));
  E_RETURN("CreatePixelShader",   d3ddevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pPShader));
  RELEASE(pBlob);
  return S_OK;
}

//--------------------------------------------------------------------------------------------------------------
//
//  D3DInitDevice
//
//--------------------------------------------------------------------------------------------------------------
HRESULT D3DInitDevice(HWND hWnd, UINT width ,UINT height, LPCSTR fxfilename) {
  HRESULT hRet = E_FAIL;
  
  //--------------------------------------------------------------------------------------------------------------
  //Create Direct3D
  //--------------------------------------------------------------------------------------------------------------
  d3dsddesc.OutputWindow = hWnd;
  E_RETURN("D3D11CreateDeviceAndSwapChain",
    D3D11CreateDeviceAndSwapChain(
      NULL, D3D_DRIVER_TYPE_HARDWARE,
      NULL, 0, NULL, 0,
      D3D11_SDK_VERSION,
      &d3dsddesc,
      &d3dswapchain,
      &d3ddevice,
      NULL,
      &d3dcontext));
  
  E_RETURN("d3dswapchain GetBuffer", d3dswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackTexture ));
  E_RETURN("CreateRenderTargetView Back Buffer", d3ddevice->CreateRenderTargetView(pBackTexture, NULL, &pBackRTV ));
  d3dcontext->RSSetViewports(1, &vp );
  
  //--------------------------------------------------------------------------------------------------------------
  //
  //Create Basic Shader Resource
  //
  //--------------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------------
  //Create VertexShader and Layout
  //--------------------------------------------------------------------------------------------------------------
  ID3DBlob  *pBlob = NULL;
  E_RETURN("CompileShaderFromFile VS", D3DCompileShaderFromFile(fxfilename, "vs_main", "vs_5_0", &pBlob));
  E_RETURN("CreateVertexShader",  d3ddevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &pVShader));
  UINT numElements = ARRAYSIZE( layout );
  E_RETURN("CreateInputLayout",   d3ddevice->CreateInputLayout(layout, numElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pVLayout ));
  RELEASE(pBlob);
  

  
  //--------------------------------------------------------------------------------------------------------------
  //Create Vertex Buffer
  //--------------------------------------------------------------------------------------------------------------
  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData) );
  InitData.pSysMem = ScreenVertex;
  E_RETURN("D3DXCreateBuffer : pVertexScreen", D3DCreateBuffer(&pVertexScreen, D3D11_BIND_VERTEX_BUFFER, sizeof(ScreenVertex), &InitData));


  return S_OK;
}


//--------------------------------------------------------------------------------------------------------------
//
//  D3DCompileShaderFromFile
//
//--------------------------------------------------------------------------------------------------------------
HRESULT D3DCompileShaderFromFile( LPCSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
	File file;
	file.Open(szFileName, "r");
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_PREFER_FLOW_CONTROL;
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompile2(
			file.Buf(),
			file.Size(),
			NULL, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			szEntryPoint, szShaderModel,
			dwShaderFlags, 0,
			0, NULL, 0,
			ppBlobOut, &pErrorBlob);
	if(pErrorBlob) {
		printf("INFO : %s\n", (char *)pErrorBlob->GetBufferPointer());
		RELEASE(pErrorBlob);
	}
	printf("%s : %s %s::%08X\n", __FUNCTION__, szEntryPoint, szShaderModel, hr);
	return hr;
}

//--------------------------------------------------------------------------------------------------------------
//
//  D3DCompileShaderFromFile
//
//--------------------------------------------------------------------------------------------------------------
void D3DPresent(int vsync) {
  //Draw rect angle.
  UINT VertexNum = 4;
  UINT Stride    = sizeof( ScreenVertex ) / VertexNum;
  UINT offset = 0;
  d3dcontext->IASetIndexBuffer( NULL, DXGI_FORMAT_UNKNOWN, 0 );
  d3dcontext->IASetVertexBuffers( 0, 1, &pVertexScreen, &Stride, &offset );
  d3dcontext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
  d3dcontext->IASetInputLayout(pVLayout);
  
  float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
  d3dcontext->ClearRenderTargetView(pBackRTV, ClearColor);
  
  d3dcontext->VSSetShader(pVShader, NULL, 0);
  d3dcontext->GSSetShader(NULL, NULL, 0);
  d3dcontext->PSSetShader(pPShader, NULL, 0);
  d3dcontext->VSSetConstantBuffers(0, 1, &pConstant);
  d3dcontext->GSSetConstantBuffers(0, 1, &pConstant);
  d3dcontext->PSSetConstantBuffers(0, 1, &pConstant);
  d3dcontext->Draw(VertexNum, 0);
  
  if(vsync) d3dswapchain->Present(1, 0);
  else      d3dswapchain->Present(0, 0);
}


//--------------------------------------------------------------------------------------------------------------
//
//  D3DCreateBuffer
//
//--------------------------------------------------------------------------------------------------------------
HRESULT D3DCreateBuffer(
  ID3D11Buffer **ppBuffer,
  UINT BindFlags,
  UINT ByteWidth,
  D3D11_SUBRESOURCE_DATA *data,
  D3D11_USAGE Usage,
  UINT AccessFlag,
  UINT MiscFlags,
  UINT StructureByteStride)
{
  
  HRESULT hRet = E_FAIL;
  printf("\n==CreateBuffer==\n");
  printf("  ByteWidth           = %d\n", ByteWidth          );
  printf("  D3D11_USAGE_DEFAULT = %d\n", Usage);
  printf("  BindFlags           = %d\n", BindFlags          );
  printf("  AccessFlag          = %d\n", AccessFlag  );
  printf("  MiscFlags           = %d\n", MiscFlags          );
  printf("  StructureByteStride = %d\n", StructureByteStride);
  
  D3D11_BUFFER_DESC bd = { ByteWidth, Usage, BindFlags, AccessFlag, MiscFlags, StructureByteStride};
  hRet = d3ddevice->CreateBuffer( &bd, data, ppBuffer );
  return hRet;
}


HRESULT D3DCreateBufferUAV(ID3D11Buffer **pBuffer, ID3D11UnorderedAccessView **ppUAV, UINT ByteWidth, UINT Stride)
{
  UINT CSBindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
  E_RETURN("D3DCreateBuffer",
    D3DCreateBuffer(
      pBuffer, CSBindFlags, ByteWidth, NULL, D3D11_USAGE_DEFAULT, 0, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, Stride ));
  D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
  ZeroMemory( &uavDesc, sizeof(uavDesc) );
  uavDesc.Format              = DXGI_FORMAT_UNKNOWN;
  uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
  uavDesc.Buffer.FirstElement = 0;
  uavDesc.Buffer.NumElements  = ByteWidth / Stride;
  E_RETURN("D3DCreateBufferUAV CS", d3ddevice->CreateUnorderedAccessView( *pBuffer, &uavDesc, ppUAV ));
  return S_OK;
}

//--------------------------------------------------------------------------------------------------------------
//
//  D3DCreateTexture
//
//--------------------------------------------------------------------------------------------------------------
HRESULT D3DCreateTexture(
  ID3D11Texture2D           **ppTex,
  ID3D11ShaderResourceView  **ppSRV,
  UINT  Width, UINT  Height, DXGI_FORMAT  Format, D3D11_USAGE  Usage, UINT  BindFlags, UINT  MipLevels)
{
  D3D11_TEXTURE2D_DESC   texdesc = {
    Width, Height, MipLevels, 1, Format,
    //{ 0, 0 },
    { 1, 0 },
    Usage, BindFlags, 0, 0,
  };
  E_RETURN("CreateTexture2D",
    d3ddevice->CreateTexture2D(&texdesc, 0, ppTex) );
  
  E_RETURN("CreateShaderResourceView",
    d3ddevice->CreateShaderResourceView(*ppTex, &descTexture, ppSRV ) );
  return S_OK;
}


//--------------------------------------------------------------------------------------------------------------
//
//  D3DCreateSamplerState
//
//--------------------------------------------------------------------------------------------------------------
HRESULT D3DCreateSamplerState(ID3D11SamplerState **ppSamplerLinear, const D3D11_SAMPLER_DESC *pdesc)
{
  if(!pdesc) pdesc = &sampDesc;
  E_RETURN("CreateSamplerState", d3ddevice->CreateSamplerState(pdesc, ppSamplerLinear ));
  return S_OK;
}




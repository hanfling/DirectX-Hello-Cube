#include "d3dApp.h"
#include "d3dx11effect.h"
#include "MathHelper.h"

using namespace DirectX;

struct Vertex {
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class BoxDemo : public D3DApp {
public:
	BoxDemo(HINSTANCE hInstance);
	~BoxDemo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();

private: 
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {

#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BoxDemo theApp(hInstance);

	if (!theApp.Init())
	{
		return 0;
	}

	theApp.Run();

}
	BoxDemo::BoxDemo(HINSTANCE hInstance)
		: D3DApp(hInstance),mBoxVB(0),mBoxIB(0),mFX(0),mTech(0),mfxWorldViewProj(0),mInputLayout(0),
			mTheta(1.5f * MathHelper::Pi),mPhi(0.25f*MathHelper::Pi),mRadius(5.0f)
	{
		mMainWndCaption = L"Box Demo";

		mLastMousePos.x = 0;
		mLastMousePos.y = 0;

		XMMATRIX I = XMMatrixIdentity();
		XMStoreFloat4x4(&mWorld, I);
		XMStoreFloat4x4(&mView, I);
		XMStoreFloat4x4(&mProj, I);
	}

	BoxDemo::~BoxDemo() {
		ReleaseCOM(mBoxVB);
		ReleaseCOM(mBoxIB);
		ReleaseCOM(mFX);
		ReleaseCOM(mInputLayout);
	}

	bool BoxDemo::Init(){
		if (!D3DApp::Init())
		{
			return false;
		}

		BuildGeometryBuffers();
		BuildFX();
		BuildVertexLayout();

		return true;
	}

	void BoxDemo::OnResize() {
		D3DApp::OnResize();

		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		XMStoreFloat4x4(&mProj, P);
	}

	void BoxDemo::UpdateScene(float dt) {
		float x = mRadius * sinf(mPhi) * cosf(mTheta);
		float y = mRadius * sinf(mPhi) * sinf(mTheta);
		float z = mRadius * cos(mPhi);

		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		XMVECTOR target = XMVectorZero();
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mView, V);
	}

	void BoxDemo::DrawScene() {
		md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
		md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		md3dImmediateContext->IASetInputLayout(mInputLayout);
		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
		
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mWorld);
		XMMATRIX view = XMLoadFloat4x4(&mView);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX worldViewProj = world * view * proj;

		mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

		D3DX11_TECHNIQUE_DESC techDesc;
		mTech->GetDesc(&techDesc);
		for (UINT p = 0;p<techDesc.Passes;++p)
		{
			mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

			md3dImmediateContext->DrawIndexed(36*2, 0, 0);
		}

		HR(mSwapChain->Present(0, 0));
	}

	void BoxDemo::OnMouseDown(WPARAM btnState, int x, int y) 
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(mhMainWnd);
	}

	void BoxDemo::OnMouseUp(WPARAM btnState, int x, int y)
	{
		ReleaseCapture();
	}

	void BoxDemo::OnMouseMove(WPARAM btnState, int x, int y)
	{
		if ((btnState & MK_LBUTTON) != 0) {
			float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
			float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

			mTheta += dx;
			mPhi += dy;

			mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
		}
		else if((btnState & MK_RBUTTON) != 0)
		{
			float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
			float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

			mRadius += dx - dy;

			mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
		}

		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}

	void BoxDemo::BuildGeometryBuffers()
	{
		XMVECTORF32 Clear = {0.0f, 0.0f, 0.0f, 0.0f};

		XMVECTORF32 MinusWhite     = {-1.0f, -1.0f, -1.0f, -1.0f};
		XMVECTORF32 MinusBlack     = {-0.0f, -0.0f, -0.0f, -1.0f};
		XMVECTORF32 MinusRed       = {-1.0f, -0.0f, -0.0f, -1.0f};
		XMVECTORF32 MinusGreen     = {-0.0f, -1.0f, -0.0f, -1.0f};
		XMVECTORF32 MinusBlue      = {-0.0f, -0.0f, -1.0f, -1.0f};
		XMVECTORF32 MinusYellow    = {-1.0f, -1.0f, -0.0f, -1.0f};
		XMVECTORF32 MinusCyan      = {-0.0f, -1.0f, -1.0f, -1.0f};
		XMVECTORF32 MinusMagenta   = {-1.0f, -0.0f, -1.0f, -1.0f};

		Vertex vertices[] =
		{
			// Actual vertices.
			{XMFLOAT3(-1.0f,-1.0f,-1.0f),XMFLOAT4((const float*) &Colors::White)},
			{XMFLOAT3(-1.0f, 1.0f,-1.0f),XMFLOAT4((const float*) &Colors::Black)},
			{XMFLOAT3(1.0f, 1.0f,-1.0f), XMFLOAT4((const float*) &Colors::Red)},
			{XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT4((const float*) &Colors::Green)},
			{XMFLOAT3(-1.0f,-1.0f, 1.0f),XMFLOAT4((const float*) &Colors::Blue)},
			{XMFLOAT3(-1.0f, 1.0f, 1.0f),XMFLOAT4((const float*) &Colors::Yellow)},
			{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4((const float*) &Colors::Cyan)},
			{XMFLOAT3(1.0f,-1.0f, 1.0f), XMFLOAT4((const float*) &Colors::Magenta)},

			// Actual vertices.
			{XMFLOAT3(-1.0f,-1.0f,-1.0f),XMFLOAT4((const float*) &MinusWhite)},
			{XMFLOAT3(-1.0f, 1.0f,-1.0f),XMFLOAT4((const float*) &MinusBlack)},
			{XMFLOAT3(1.0f, 1.0f,-1.0f), XMFLOAT4((const float*) &MinusRed)},
			{XMFLOAT3(1.0f,-1.0f,-1.0f), XMFLOAT4((const float*) &MinusGreen)},
			{XMFLOAT3(-1.0f,-1.0f, 1.0f),XMFLOAT4((const float*) &MinusBlue)},
			{XMFLOAT3(-1.0f, 1.0f, 1.0f),XMFLOAT4((const float*) &MinusYellow)},
			{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4((const float*) &MinusCyan)},
			{XMFLOAT3(1.0f,-1.0f, 1.0f), XMFLOAT4((const float*) &MinusMagenta)},

			// Dummy.
			{XMFLOAT3(0.0f, 0.0f,0.0f),XMFLOAT4((const float*) &Clear)},
		};

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * (8 * 2 +1 );
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = vertices;
		HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

		UINT indices[] = {
			//front face (barycentric/triangles)
			0,17,1,17,2,8+1,
			2,17,3,17,0,8+3,

			//back face (barycentric/triangles)
			6,17,5,17,4,8+5,
			4,17,7,17,6,8+7,

			//left face (linear/quad)
			4,17,5,17,1,0,
			1,17,0,17,4,5,

			//right Face (linear/quad)
			3,17,2,17,6,7,
			6,17,7,17,3,2,

			//top face (barycentric/triangles)
			1,17,5,17,6,8+5,
			6,17,2,17,1,8+2,

			//bottom face (linear/quad)
			4,17,0,17,3,7,
			3,17,7,17,4,0
		};

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * 36 * 2;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = indices;
		HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
	}

	void BoxDemo::BuildFX() {
		DWORD shaderFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags != D3D10_SHADER_DEBUG;
		shaderFlags != D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
		ID3DBlob* errorBlob;
		HRESULT hr;

		hr = D3DX11CompileEffectFromFile(L"color.fx",nullptr, nullptr ,shaderFlags,0,md3dDevice,&mFX,&errorBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr, (LPCWSTR)errorBlob->GetBufferPointer(), L"error", MB_OK);
			return;
		}
		ReleaseCOM(errorBlob);

		mTech = mFX->GetTechniqueByName("ColorTech");
		mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	}

	void BoxDemo::BuildVertexLayout()
	{
		D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}
		};

		D3DX11_PASS_DESC passDesc;
		mTech->GetPassByIndex(0)->GetDesc(&passDesc);
		HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
	}












































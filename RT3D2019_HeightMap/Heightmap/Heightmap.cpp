#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>

#include "CommonApp.h"

#include <stdio.h>

#include <DirectXMath.h>
using namespace DirectX;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class HeightMapApplication : public CommonApp
{
  public:
  protected:
	bool HandleStart();
	void HandleStop();
	void HandleUpdate();
	void HandleRender();
	bool LoadHeightMap(char* filename, float gridSize);

  private:
	ID3D11Buffer* m_pHeightMapBuffer;
	float m_rotationAngle;
	int m_HeightMapWidth;
	int m_HeightMapLength;
	int m_HeightMapVtxCount;
	XMFLOAT3* m_pHeightMap;
	Vertex_Pos3fColour4ubNormal3f* m_pMapVtxs;
	float m_cameraZ;
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool HeightMapApplication::HandleStart()
{
	this->SetWindowTitle("HeightMap");

	LoadHeightMap("HeightMap.bmp", 1.0f);

	m_cameraZ = 50.0f;

	m_pHeightMapBuffer = NULL;

	m_rotationAngle = 0.f;

	if(!this->CommonApp::HandleStart())
		return false;

	static const VertexColour MAP_COLOUR(200, 255, 255, 255);

	/*m_HeightMapVtxCount = m_HeightMapWidth * m_HeightMapLength * 6;
	m_pMapVtxs = new Vertex_Pos3fColour4ubNormal3f[m_HeightMapVtxCount];

	for (int y = 0; y < m_HeightMapLength - 1; ++y)
	{
		for (int x = 0; x < m_HeightMapWidth - 1; ++x)
		{
			int QuadIndex = y * m_HeightMapWidth + x;
			XMFLOAT3 v0 = m_pHeightMap[QuadIndex];
			XMFLOAT3 v1 = m_pHeightMap[QuadIndex + m_HeightMapWidth];
			XMFLOAT3 v2 = m_pHeightMap[QuadIndex + 1];
			XMFLOAT3 v3 = m_pHeightMap[QuadIndex + m_HeightMapWidth + 1];


			XMVECTOR U;
			XMVECTOR V;
			XMFLOAT3 Normal = XMFLOAT3(0, 0, 0);

			U = XMVECTOR(XMLoadFloat3(&v1) - XMLoadFloat3(&v0));
			V = XMVECTOR(XMLoadFloat3(&v2) - XMLoadFloat3(&v0));
			XMStoreFloat3(&Normal, DirectX::XMVector3Cross(U, V));
			m_pMapVtxs[QuadIndex * 6] = Vertex_Pos3fColour4ubNormal3f(v0, MAP_COLOUR, Normal);
			m_pMapVtxs[QuadIndex * 6 + 1] = Vertex_Pos3fColour4ubNormal3f(v1, MAP_COLOUR, Normal);
			m_pMapVtxs[QuadIndex * 6 + 2] = Vertex_Pos3fColour4ubNormal3f(v2, MAP_COLOUR, Normal);



			U = XMVECTOR(XMLoadFloat3(&v1) - XMLoadFloat3(&v2));
			V = XMVECTOR(XMLoadFloat3(&v3) - XMLoadFloat3(&v2));
			XMStoreFloat3(&Normal, DirectX::XMVector3Cross(U, V));
			m_pMapVtxs[QuadIndex * 6 + 3] = Vertex_Pos3fColour4ubNormal3f(v2, MAP_COLOUR, Normal);
			m_pMapVtxs[QuadIndex * 6 + 4] = Vertex_Pos3fColour4ubNormal3f(v1, MAP_COLOUR, Normal);
			m_pMapVtxs[QuadIndex * 6 + 5] = Vertex_Pos3fColour4ubNormal3f(v3, MAP_COLOUR, Normal);

		}
	}*/

	m_HeightMapVtxCount = (m_HeightMapWidth - 1) * m_HeightMapLength * 4;
	m_pMapVtxs = new Vertex_Pos3fColour4ubNormal3f[m_HeightMapVtxCount];

	XMFLOAT3 PrevVerts[2];
	int VertexCounter = 0;
	for (int y = 0; y < m_HeightMapLength - 1; ++y)
	{
		int StartPoint = y * m_HeightMapWidth;
		PrevVerts[0] = m_pHeightMap[StartPoint];
		PrevVerts[1] = m_pHeightMap[StartPoint + m_HeightMapWidth];

		XMVECTOR U;
		XMVECTOR V;
		XMFLOAT3 Normal = XMFLOAT3(0, 0, 0);

		U = XMVECTOR(XMLoadFloat3(&PrevVerts[1]) - XMLoadFloat3(&PrevVerts[0]));
		V = XMVECTOR(XMLoadFloat3(&m_pHeightMap[StartPoint + 1]) - XMLoadFloat3(&PrevVerts[0]));
		XMStoreFloat3(&Normal, DirectX::XMVector3Cross(U, V));

		m_pMapVtxs[VertexCounter++] = Vertex_Pos3fColour4ubNormal3f(PrevVerts[0], MAP_COLOUR, Normal);
		m_pMapVtxs[VertexCounter++] = Vertex_Pos3fColour4ubNormal3f(PrevVerts[1], MAP_COLOUR, Normal);

		for (int x = 0; x < (m_HeightMapWidth - 1); ++x)
		{
			int QuadIndex = y * m_HeightMapWidth + x;

			XMFLOAT3 Vert = m_pHeightMap[QuadIndex + 1];
			m_pMapVtxs[VertexCounter++] = Vertex_Pos3fColour4ubNormal3f(Vert, MAP_COLOUR, Normal);
			PrevVerts[0] = Vert;

			
			Vert = m_pHeightMap[QuadIndex + m_HeightMapWidth + 1];

			U = XMVECTOR(XMLoadFloat3(&PrevVerts[1]) - XMLoadFloat3(&PrevVerts[0]));
			V = XMVECTOR(XMLoadFloat3(&Vert) - XMLoadFloat3(&PrevVerts[0]));
			XMStoreFloat3(&Normal, DirectX::XMVector3Cross(U, V));
			m_pMapVtxs[VertexCounter++] = Vertex_Pos3fColour4ubNormal3f(Vert, MAP_COLOUR, Normal);
			PrevVerts[1] = Vert;
		}
		int EndPoint = (y + 1) * m_HeightMapWidth;
		m_pMapVtxs[VertexCounter++] = Vertex_Pos3fColour4ubNormal3f(PrevVerts[1], VertexColour(255, 0, 0, 0), Normal);
		XMFLOAT3 DEGEN = m_pHeightMap[EndPoint];
		m_pMapVtxs[VertexCounter++] = Vertex_Pos3fColour4ubNormal3f(DEGEN, VertexColour(255, 0, 0, 0), Normal);
		
	}


	m_pHeightMapBuffer = CreateImmutableVertexBuffer(m_pD3DDevice, sizeof Vertex_Pos3fColour4ubNormal3f * m_HeightMapVtxCount, m_pMapVtxs);

	delete m_pMapVtxs;

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMapApplication::HandleStop()
{
	Release(m_pHeightMapBuffer);

	this->CommonApp::HandleStop();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMapApplication::HandleUpdate()
{
	m_rotationAngle += .01f;

	if(this->IsKeyPressed('Q'))
	{
		if(m_cameraZ > 20.0f)
			m_cameraZ -= 2.0f;
	}

	if(this->IsKeyPressed('A'))
	{
		m_cameraZ += 2.0f;
	}
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void HeightMapApplication::HandleRender()
{
	XMFLOAT3 vCamera(sin(m_rotationAngle) * m_cameraZ, m_cameraZ / 2, cos(m_rotationAngle) * m_cameraZ);
	XMFLOAT3 vLookat(0.0f, 0.0f, 0.0f);
	XMFLOAT3 vUpVector(0.0f, 1.0f, 0.0f);

	XMMATRIX matView;
	matView = XMMatrixLookAtLH(XMLoadFloat3(&vCamera), XMLoadFloat3(&vLookat), XMLoadFloat3(&vUpVector));

	XMMATRIX matProj;
	matProj = XMMatrixPerspectiveFovLH(float(XM_PI / 4), 2, 1.5f, 5000.0f);

	this->SetViewMatrix(matView);
	this->SetProjectionMatrix(matProj);

	this->EnablePointLight(0, XMFLOAT3(100.0f, 100.f, -100.f), XMFLOAT3(1.f, 1.f, 1.f));

	this->Clear(XMFLOAT4(.2f, .2f, .6f, 1.f));

	this->DrawUntexturedLit(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pHeightMapBuffer, NULL, m_HeightMapVtxCount);
}

//////////////////////////////////////////////////////////////////////
// LoadHeightMap
// Original code sourced from rastertek.com
//////////////////////////////////////////////////////////////////////
bool HeightMapApplication::LoadHeightMap(char* filename, float gridSize)
{
	FILE* filePtr;
	int error;
	unsigned int count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageSize, i, j, k, index;
	unsigned char* bitmapImage;
	unsigned char height;

	// Open the height map file in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if(error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Read in the bitmap info header.
	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if(count != 1)
	{
		return false;
	}

	// Save the dimensions of the terrain.
	m_HeightMapWidth = bitmapInfoHeader.biWidth;
	m_HeightMapLength = bitmapInfoHeader.biHeight;

	// Calculate the size of the bitmap image data.
	imageSize = m_HeightMapWidth * m_HeightMapLength * 3;

	// Allocate memory for the bitmap image data.
	bitmapImage = new unsigned char[imageSize];
	if(!bitmapImage)
	{
		return false;
	}

	// Move to the beginning of the bitmap data.
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

	// Read in the bitmap image data.
	count = fread(bitmapImage, 1, imageSize, filePtr);
	if(count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if(error != 0)
	{
		return false;
	}

	// Create the structure to hold the height map data.
	m_pHeightMap = new XMFLOAT3[m_HeightMapWidth * m_HeightMapLength];
	if(!m_pHeightMap)
	{
		return false;
	}

	// Initialize the position in the image data buffer.
	k = 0;

	// Read the image data into the height map.
	for(j = 0; j < m_HeightMapLength; j++)
	{
		for(i = 0; i < m_HeightMapWidth; i++)
		{
			height = bitmapImage[k];

			index = (m_HeightMapLength * j) + i;

			m_pHeightMap[index].x = (float)(i - (m_HeightMapWidth / 2)) * gridSize;
			m_pHeightMap[index].y = (float)height / 16 * gridSize;
			m_pHeightMap[index].z = (float)(j - (m_HeightMapLength / 2)) * gridSize;

			k += 3;
		}
	}

	// Release the bitmap image data.
	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	HeightMapApplication application;

	Run(&application);

	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

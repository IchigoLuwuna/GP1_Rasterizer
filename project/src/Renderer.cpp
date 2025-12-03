// External includes
#include <cassert>
#include <cmath>
#include <execution>
#include <limits>

// Project includes
#include "Renderer.h"
#include "ColorRGB.h"
#include "DataTypes.h"
#include "Scene.h"
#include "Shading.h"

#define PARALLEL_PROJECT
#define PARALLEL_RASTERIZE

using namespace dae;

Renderer::Renderer( SDL_Window* pWindow )
	: m_pWindow( pWindow )
{
	// Initialize
	SDL_GetWindowSize( pWindow, &m_Width, &m_Height );

	// Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface( pWindow );
	m_pBackBuffer = SDL_CreateRGBSurface( 0, m_Width, m_Height, 32, 0, 0, 0, 0 );
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_DepthBufferPixels = std::vector<float>( m_Width * m_Height );
}

void Renderer::Update( Timer* pTimer )
{
}

void Renderer::Render( Scene* pScene )
{
	//@START
	// Lock BackBuffer
	SDL_LockSurface( m_pBackBuffer );

	// Flush buffers
	for ( int px{}; px < m_Width; ++px )
	{
		for ( int py{}; py < m_Height; ++py )
		{
			m_pBackBufferPixels[px + ( py * m_Width )] = SDL_MapRGB( m_pBackBuffer->format, 0, 0, 0 );
		}
	}
	for ( auto& depthPixel : m_DepthBufferPixels )
	{
		depthPixel = std::numeric_limits<float>::max();
	}

	// Get world to camera
	Matrix worldToCamera{ Matrix::Inverse( pScene->GetCamera().GetCameraToWorld() ) };

	// For every mesh
	const auto& meshes{ pScene->GetMeshes() };
	for ( const auto& mesh : meshes )
	{
		RasterizeMesh( mesh, pScene->GetCamera(), worldToCamera );
	}

	//@END
	// Update SDL Surface
	SDL_UnlockSurface( m_pBackBuffer );
	SDL_BlitSurface( m_pBackBuffer, 0, m_pFrontBuffer, 0 );
	SDL_UpdateWindowSurface( m_pWindow );
}

void Renderer::RasterizeMesh( const Mesh& mesh, const Camera& camera, const Matrix& worldToCamera ) noexcept
{
	// PROJECTION
	std::vector<VertexOut> verticesOut{};
	Project( mesh.vertices, verticesOut, camera, mesh.worldMatrix, worldToCamera );

	// For every triangle in mesh
	for ( size_t index{}; index < mesh.indices.size(); )
	{
		auto goToNextTriangleIndex{ [&]() {
			// Increment differently based on topology
			switch ( mesh.primitiveTopology )
			{
			case dae::PrimitiveTopology::TriangleList:
				index += 3;
				break;

			case dae::PrimitiveTopology::TriangleStrip:
				++index;
				break;
			}
		} };

		// Stop if at the end of strip
		if ( index + 2 >= mesh.indices.size() )
		{
			break;
		}

		// Construct triangle
		Triangle_Out projectedTriangle{};
		switch ( mesh.primitiveTopology )
		{
		case PrimitiveTopology::TriangleList:
			projectedTriangle = Triangle_Out{ verticesOut[mesh.indices[index + 0]],
											  verticesOut[mesh.indices[index + 1]],
											  verticesOut[mesh.indices[index + 2]] };
			break;
		case PrimitiveTopology::TriangleStrip:
			// Check if mesh is correct size to be a strip
			assert( mesh.indices.size() > 6 && "Mesh has too few indices to be a strip" );
			// Fix orientation for odd triangles
			if ( index & 1 )
			{
				projectedTriangle = Triangle_Out{ verticesOut[mesh.indices[index + 0]],
												  verticesOut[mesh.indices[index + 2]],
												  verticesOut[mesh.indices[index + 1]] };
				projectedTriangle.pTexture = &mesh.texture;
			}
			else
			{
				projectedTriangle = Triangle_Out{ verticesOut[mesh.indices[index + 0]],
												  verticesOut[mesh.indices[index + 1]],
												  verticesOut[mesh.indices[index + 2]] };
				projectedTriangle.pTexture = &mesh.texture;
			}
			break;
		}

		if ( IsCullable( projectedTriangle ) )
		{
			goToNextTriangleIndex();
			continue;
		}

		const Rectangle projectedTriangleBounds{ projectedTriangle.GetBounds() };

		auto processPixel{ [&]( int px, int py ) {
			Vector3 baryCentricPosition{};
			if ( !IsInPixel( projectedTriangle, px, py, baryCentricPosition ) )
			{
				return;
			}

			const float depthInterpolated{ 1.f /
										   ( ( 1.f / projectedTriangle.v0.position.z ) * baryCentricPosition.x +
											 ( 1.f / projectedTriangle.v1.position.z ) * baryCentricPosition.y +
											 ( 1.f / projectedTriangle.v2.position.z ) * baryCentricPosition.z ) };

			const int bufferIndex{ px + ( py * m_Width ) };

			// Check Depth Buffer
			if ( depthInterpolated > m_DepthBufferPixels[bufferIndex] )
			{
				return;
			}
			m_DepthBufferPixels[bufferIndex] = depthInterpolated;

			ColorRGB finalColor{ GetPixelColor( projectedTriangle, baryCentricPosition ) };

			finalColor.MaxToOne();

			m_pBackBufferPixels[bufferIndex] = SDL_MapRGB( m_pBackBuffer->format,
														   static_cast<uint8_t>( finalColor.r * 255 ),
														   static_cast<uint8_t>( finalColor.g * 255 ),
														   static_cast<uint8_t>( finalColor.b * 255 ) );
		} };

		const int pixelBoundsLeft{ static_cast<int>( projectedTriangleBounds.left ) };
		const int pixelBoundsRight{ static_cast<int>( projectedTriangleBounds.right ) };
		const int pixelBoundsTop{ static_cast<int>( projectedTriangleBounds.top ) };
		const int pixelBoundsBottom{ static_cast<int>( projectedTriangleBounds.bottom ) };

#ifdef PARALLEL_RASTERIZE
		std::vector<std::pair<int, int>> pixelIndices{};
		for ( int px{ pixelBoundsLeft }; px < pixelBoundsRight; ++px )
		{
			for ( int py{ pixelBoundsTop }; py < pixelBoundsBottom; ++py )
			{
				pixelIndices.push_back( { px, py } );
			}
		}

		std::for_each(
			std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&]( const std::pair<int, int> pxpy ) {
				int px{ pxpy.first };
				int py{ pxpy.second };
				processPixel( px, py );
			} );

#endif
#ifndef PARALLEL_RASTERIZE
		// RASTERIZATION
		for ( int px{ pixelBoundsLeft }; px < pixelBoundsRight; ++px )
		{
			for ( int py{ pixelBoundsTop }; py < pixelBoundsBottom; ++py )
			{
				processPixel( px, py );
			}
		}
#endif

		goToNextTriangleIndex();
	}
}

void Renderer::Project( const std::vector<Vertex>& verticesIn,
						std::vector<VertexOut>& verticesOut,
						const Camera& camera,
						const Matrix& modelToWorld,
						const Matrix& worldToCamera ) const noexcept
{
	verticesOut.clear();
	verticesOut.reserve( verticesIn.size() );

	std::vector<uint32_t> vertexIndices{};
	vertexIndices.reserve( verticesIn.size() );
	for ( size_t index{}; index < verticesIn.size(); ++index )
	{
		verticesOut.push_back( {} );
		vertexIndices.push_back( index );
	}

	auto projectVertex{ [&]( const uint32_t index ) {
		const Vertex& vertexIn{ verticesIn[index] };
		VertexOut vertexOut{};
		vertexOut.uv = vertexIn.uv;

		// World transform
		vertexOut.position = modelToWorld.TransformPoint( vertexIn.position ).ToPoint4();
		vertexOut.normal = modelToWorld.TransformVector( vertexIn.normal );

		// Camera transform
		vertexOut.position = worldToCamera.TransformPoint( vertexOut.position );
		vertexOut.normal = worldToCamera.TransformVector( vertexOut.normal );

		// Projection matrix
		const float aspectRatio{ static_cast<float>( m_Width ) / m_Height };
		const float a{ camera.GetFar() / ( camera.GetFar() - camera.GetNear() ) }; // Depends on coordinate system
		const float b{ -( camera.GetFar() * camera.GetNear() ) / ( camera.GetFar() - camera.GetNear() ) };
		Matrix projectionMatrix{
			{ 1.f / ( aspectRatio * camera.GetFov() ), 0.f, 0.f, 0.f },
			{ 0.f, 1.f / camera.GetFov(), 0.f, 0.f },
			{ 0.f, 0.f, a, 1.f },
			{ 0.f, 0.f, b, 0.f },
		};
		vertexOut.position = projectionMatrix.TransformPoint( vertexOut.position );

		// Perspective divide
		vertexOut.position.x /= vertexOut.position.w;
		vertexOut.position.y /= vertexOut.position.w;
		vertexOut.position.z /= vertexOut.position.w;

		// To screenspace
		vertexOut.position.x = ( 1.f + vertexOut.position.x ) * 0.5f * m_Width;
		vertexOut.position.y = ( 1.f - vertexOut.position.y ) * 0.5f * m_Height;

		verticesOut[index] = vertexOut;
	} };

#ifdef PARALLEL_PROJECT
	std::for_each( std::execution::par, vertexIndices.begin(), vertexIndices.end(), projectVertex );
#endif
#ifndef PARALLEL_PROJECT
	for ( const auto& vertexIndex : vertexIndices )
	{
		project( vertexIndex );
	}
#endif
}

ColorRGB Renderer::ProcessPixel( int px, int py, const Triangle_Out& triangle ) noexcept
{
	ColorRGB finalColor{ -1.f, -1.f, -1.f };

	Vector3 baryCentricPosition{};

	if ( IsInPixel( triangle, px, py, baryCentricPosition ) )
	{
		const int pixelIndex{ px * m_Height + py };
		const float depthInterpolated{ 1.f / ( ( 1.f / triangle.v0.position.z ) * baryCentricPosition.x +
											   ( 1.f / triangle.v1.position.z ) * baryCentricPosition.y +
											   ( 1.f / triangle.v2.position.z ) * baryCentricPosition.z ) };
		const float viewSpaceDepthInterpolated{ 1.f / ( ( 1.f / triangle.v0.position.w ) * baryCentricPosition.x +
														( 1.f / triangle.v1.position.w ) * baryCentricPosition.y +
														( 1.f / triangle.v2.position.w ) * baryCentricPosition.z ) };
		if ( depthInterpolated < m_DepthBufferPixels[pixelIndex] )
		{
			m_DepthBufferPixels[pixelIndex] = depthInterpolated;
			const Vector2 uv{ Vector2{ triangle.v0.uv / triangle.v0.position.w * baryCentricPosition.x +
									   triangle.v1.uv / triangle.v1.position.w * baryCentricPosition.y +
									   triangle.v2.uv / triangle.v2.position.w * baryCentricPosition.z } *
							  viewSpaceDepthInterpolated };

			finalColor = triangle.pTexture->Sample( uv );
		}
	}

	// Update Color in Buffer
	finalColor.MaxToOne();

	return finalColor;
}

bool Renderer::IsInPixel( const Triangle_Out& triangle, int px, int py, Vector3& baryCentricPosition ) noexcept
{
	const Vector2 screenSpace{ px + 0.5f, py + 0.5f };

	const std::array<Vector2, 3> triangle2d{ triangle.v0.position.GetXY(),
											 triangle.v1.position.GetXY(),
											 triangle.v2.position.GetXY() };

	std::array<float, 3> baryBuffer{};
	const float parallelogramArea{ Vector2::Cross( ( triangle2d[1] - triangle2d[0] ),
												   ( triangle2d[2] - triangle2d[0] ) ) };

	// Prevent floating point precision error-based crashes
	// This area being 0 or lower could pose issues later down the line
	if ( parallelogramArea <= 0.f )
	{
		return false;
	}

	for ( int offset{}; offset < 3; ++offset )
	{
		int nextOffset{ ( offset + 1 ) % 3 };
		const Vector2 vertex{ triangle2d[offset] };
		const Vector2 nextVertex{ triangle2d[nextOffset] };
		const Vector2 edgeVector{ nextVertex - vertex };
		const Vector2 vertexToPixel{ screenSpace - vertex };

		const float signedArea{ Vector2::Cross( edgeVector, vertexToPixel ) };

		if ( signedArea < 0.f )
		{
			return false;
		}
		else
		{
			const int baryIndex{ ( offset + 2 ) % 3 };
			baryBuffer[baryIndex] = signedArea / parallelogramArea;
			continue;
		}
	}

	baryCentricPosition = { baryBuffer[0], baryBuffer[1], baryBuffer[2] };

	return true;
}

bool Renderer::IsCullable( const Triangle_Out& triangle )
{
	// Backface Culling
	if ( triangle.normal.z > 0.f ) // positive Z is forward -> away from the screen
	{
		return true;
	}

	// Frustum Culling
	if ( triangle.v0.position.z > 1.f || triangle.v0.position.z < 0.f )
	{
		return true;
	}
	if ( triangle.v1.position.z > 1.f || triangle.v1.position.z < 0.f )
	{
		return true;
	}
	if ( triangle.v2.position.z > 1.f || triangle.v2.position.z < 0.f )
	{
		return true;
	}

	// ScreenSpace Culling
	if ( triangle.v0.position.x < 0.f || triangle.v0.position.x > m_Width )
	{
		return true;
	}
	if ( triangle.v1.position.x < 0.f || triangle.v1.position.x > m_Width )
	{
		return true;
	}
	if ( triangle.v2.position.x < 0.f || triangle.v2.position.x > m_Width )
	{
		return true;
	}
	if ( triangle.v0.position.y < 0.f || triangle.v0.position.y > m_Height )
	{
		return true;
	}
	if ( triangle.v1.position.y < 0.f || triangle.v1.position.y > m_Height )
	{
		return true;
	}
	if ( triangle.v2.position.y < 0.f || triangle.v2.position.y > m_Height )
	{
		return true;
	}

	return false;
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP( m_pBackBuffer, "Rasterizer_ColorBuffer.bmp" );
}

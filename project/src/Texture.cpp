#include "Texture.h"
#include <cstdint>
#include "ColorRGB.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <SDL_surface.h>

namespace dae
{
Texture::Texture( const Texture& rhs )
	: m_pSurface{ SDL_DuplicateSurface( rhs.m_pSurface ) }
	, m_pSurfacePixels{ reinterpret_cast<uint32_t*>( m_pSurface->pixels ) }
{
}

Texture::Texture( Texture&& rhs )
	: m_pSurface{ rhs.m_pSurface }
	, m_pSurfacePixels{ rhs.m_pSurfacePixels }
{
	rhs.m_pSurface = nullptr;
	rhs.m_pSurfacePixels = nullptr;
}

Texture::Texture( const std::string& path )
{
	LoadFromFile( path );
}

Texture::~Texture()
{
	if ( m_pSurface )
	{
		SDL_FreeSurface( m_pSurface );
		m_pSurface = nullptr;
	}
}

Texture& Texture::operator=( const Texture& rhs )
{
	m_pSurface = SDL_DuplicateSurface( rhs.m_pSurface );
	m_pSurfacePixels = reinterpret_cast<uint32_t*>( m_pSurface->pixels );

	return *this;
}

Texture& Texture::operator=( Texture&& rhs )
{
	m_pSurface = rhs.m_pSurface;
	m_pSurfacePixels = rhs.m_pSurfacePixels;

	rhs.m_pSurface = nullptr;
	rhs.m_pSurfacePixels = nullptr;

	return *this;
}

ColorRGB Texture::Sample( const Vector2& uv ) const
{
#define FAST_RGB

	// Convert UV coordinates to texture coordinates
	const int pixelWidth{ m_pSurface->w };
	const int pixelHeight{ m_pSurface->h };
	const int uvpx{ static_cast<int>( std::round( uv.x * pixelWidth ) ) };
	const int uvpy{ static_cast<int>( std::round( uv.y * pixelHeight ) ) };

// Get RGB
#ifndef FAST_RGB
	const int pixelIndex{ uvpx + uvpy * pixelWidth };
	uint8_t pixelR{};
	uint8_t pixelG{};
	uint8_t pixelB{};
	uint32_t pixel{ m_pSurfacePixels[pixelIndex] };
	SDL_GetRGB( pixel, m_pSurface->format, &pixelR, &pixelG, &pixelB );
#endif
#ifdef FAST_RGB
	const int pixelIndex{ uvpx + uvpy * pixelWidth };
	uint32_t pixel{ m_pSurfacePixels[pixelIndex] };
	const uint8_t pixelR{ *reinterpret_cast<const uint8_t*>( &pixel ) }; // Capture first 8 bits -> Red
	pixel = pixel >> 8;													 // Shift right to capture next
	const uint8_t pixelG{ *reinterpret_cast<const uint8_t*>( &pixel ) }; // Capture first 8 bits -> Green
	pixel = pixel >> 8;													 // Shift right to capture next
	const uint8_t pixelB{ *reinterpret_cast<const uint8_t*>( &pixel ) }; // Capture first 8 bits -> Blue
#endif

	// Convert unsigned int RGB to float RGB
	const ColorRGB color{ pixelR / 255.f, pixelG / 255.f, pixelB / 255.f };
	return color;
}

void Texture::LoadFromFile( const std::string& path )
{
	m_pSurface = IMG_Load( path.c_str() );
	m_pSurfacePixels = reinterpret_cast<uint32_t*>( m_pSurface->pixels );
}
} // namespace dae

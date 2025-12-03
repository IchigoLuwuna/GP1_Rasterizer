#ifndef TEXTURE_H
#define TEXTURE_H

#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
struct Vector2;
class Texture final
{
public:
	Texture() = default;
	Texture( const Texture& rhs );
	Texture( Texture&& rhs );
	Texture( const std::string& path );
	~Texture();

	Texture& operator=( const Texture& );
	Texture& operator=( Texture&& rhs );

	ColorRGB Sample( const Vector2& uv ) const;

private:
	SDL_Surface* m_pSurface{};
	uint32_t* m_pSurfacePixels{};

	void LoadFromFile( const std::string& path );
};
} // namespace dae
#endif

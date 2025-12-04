#ifndef RENDERER_H
#define RENDERER_H

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
class Texture;
struct Mesh;
struct Vertex;
class Timer;
class Scene;

class Renderer final
{
public:
	Renderer( SDL_Window* pWindow );
	~Renderer();

	Renderer( const Renderer& ) = delete;
	Renderer( Renderer&& ) noexcept = delete;
	Renderer& operator=( const Renderer& ) = delete;
	Renderer& operator=( Renderer&& ) noexcept = delete;

	void Update( Timer* pTimer );
	void Render( const Scene* pScene );

	bool SaveBufferToImage() const;

private:
	SDL_Window* m_pWindow{};

	SDL_Surface* m_pFrontBuffer{ nullptr };
	SDL_Surface* m_pBackBuffer{ nullptr };
	uint32_t* m_pBackBufferPixels{};

	std::vector<float> m_DepthBufferPixels{};
	std::vector<std::pair<bool, VertexOut>> m_PixelAttributeBuffer{};

	int m_Width{};
	int m_Height{};

	void Project( const std::vector<Vertex>& verticesIn,
				  std::vector<VertexOut>& verticesOut,
				  const Camera& camera,
				  const Matrix& modelToWorld,
				  const Matrix& worldToCamera ) const noexcept;
	void RasterizeMesh( const Mesh& mesh, const Scene* pScene, const Matrix& worldToCamera ) noexcept;
	void ShadePixel( int px, int py, const VertexOut& attributes );

	bool IsInPixel( const TriangleOut& triangle, int px, int py, Vector3& baryCentricPosition ) noexcept;
	bool IsCullable( const TriangleOut& triangle ) noexcept;
};
} // namespace dae

#endif

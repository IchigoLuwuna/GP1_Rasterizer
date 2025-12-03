#include "Shading.h"
#include <cassert>
#include "ColorRGB.h"

namespace dae
{
ColorRGB GetPixelColor( const Triangle_Out& triangle, const Vector3 baryCentricPosition )
{
	const float viewSpaceDepthInterpolated{ 1.f / ( ( 1.f / triangle.v0.position.w ) * baryCentricPosition.x +
													( 1.f / triangle.v1.position.w ) * baryCentricPosition.y +
													( 1.f / triangle.v2.position.w ) * baryCentricPosition.z ) };

	const Vector2 uvPosition{ ( triangle.v0.uv / triangle.v0.position.w * baryCentricPosition.x +
								triangle.v1.uv / triangle.v1.position.w * baryCentricPosition.y +
								triangle.v2.uv / triangle.v2.position.w * baryCentricPosition.z ) *
							  viewSpaceDepthInterpolated };

	const ColorRGB textureSample{ triangle.pTexture->Sample( uvPosition ) };

	return textureSample;
}
} // namespace dae

#include "Shading.h"
#include <cassert>
#include "ColorRGB.h"
#include "DataTypes.h"
#include "Vector3.h"

namespace dae
{
ColorRGB GetPixelColor( const Mesh& mesh, const VertexOut& pixelVertex, const std::vector<Light>& lights )
{
	const ColorRGB diffuseColor{ mesh.texture.Sample( pixelVertex.uv ) };
	const Vector3 pixelPos{ pixelVertex.position.x, pixelVertex.position.y, pixelVertex.position.w };

	if ( lights.empty() )
	{
		return diffuseColor;
	}

	const Vector3 binormal{ Vector3::Cross( pixelVertex.normal, pixelVertex.tangent ).Normalized() };
	const Matrix tangentAxisSpace{ pixelVertex.tangent, binormal, pixelVertex.normal, {} };

	const ColorRGB sampledNormalColor{ ( mesh.normalMap.Sample( pixelVertex.uv ) / 255.f * 2.f ) };
	Vector3 sampledNormal{ sampledNormalColor.r, sampledNormalColor.g, sampledNormalColor.b };
	sampledNormal = tangentAxisSpace.TransformVector( sampledNormal );
	sampledNormal.Normalize();

	ColorRGB finalColor{};
	for ( auto& light : lights )
	{
		constexpr float diffuseReflectance{ 7.f }; // Hardcoded to make up for lack of lights

		const float observedArea{ lightUtils::GetObservedArea( light, pixelPos, sampledNormal ) };
		const ColorRGB radiance{ lightUtils::GetRadiance( light, pixelPos ) };
		const ColorRGB lambertDiffuse{ ( diffuseColor * diffuseReflectance ) / PI };

		finalColor += observedArea * radiance * lambertDiffuse;
	}

	finalColor.MaxToOne();

	return finalColor;
}

namespace lightUtils
{
float GetObservedArea( const Light& light, const Vector3& position, const Vector3& normal )
{
	Vector3 dirToLight{};
	switch ( light.type )
	{
	case LightType::point:
		dirToLight = light.vector - position;
		break;

	case LightType::directional:
		dirToLight = -light.vector;
		break;
	}

	return std::max( Vector3::Dot( normal, dirToLight ), 0.f );
}

ColorRGB GetRadiance( const Light& light, const Vector3& target )
{
	switch ( light.type )
	{
	case LightType::point:
		return { light.color * ( light.intensity / ( light.vector - target ).SqrMagnitude() ) };
		break;

	case LightType::directional:
		return light.color * light.intensity;
		break;
	default:
		return light.color * light.intensity;
	}
}
} // namespace lightUtils
} // namespace dae

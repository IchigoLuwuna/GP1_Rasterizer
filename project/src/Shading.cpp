#include "Shading.h"
#include "Renderer.h"

namespace dae
{
ColorRGB GetPixelColor( const Mesh& mesh,
						const VertexOut& pixelVertex,
						const Camera& camera,
						const std::vector<Light>& lights,
						const LightingMode& lightingMode,
						bool useNormalMap )
{
	const ColorRGB diffuseColor{ mesh.texture.Sample( pixelVertex.uv ) };

	if ( lights.empty() )
	{
		return diffuseColor;
	}

	const Vector3 pixelPos{ pixelVertex.position.x, pixelVertex.position.y, pixelVertex.position.w };

	Vector3 sampledNormal{};
	if ( useNormalMap )
	{
		const Vector3 binormal{ Vector3::Cross( pixelVertex.normal, pixelVertex.tangent ).Normalized() };
		const Matrix tangentAxisSpace{ pixelVertex.tangent, binormal, pixelVertex.normal, {} };
		ColorRGB sampledNormalColor{ ( mesh.normalMap.Sample( pixelVertex.uv ) ) };
		sampledNormal = { sampledNormalColor.r, sampledNormalColor.g, sampledNormalColor.b };
		sampledNormal = ( sampledNormal * 2.f ) - Vector3{ 1.f, 1.f, 1.f };
		sampledNormal = tangentAxisSpace.TransformVector( sampledNormal );
		sampledNormal.Normalize();
	}
	else
	{
		sampledNormal = pixelVertex.normal;
	}

	const ColorRGB sampledSpecularity{ mesh.specularMap.Sample( pixelVertex.uv ) };
	const float sampledGloss{ mesh.glossMap.Sample( pixelVertex.uv ).r }; // Assuming map is greyscale

	const Vector3 toCameraDir{ Vector3( pixelPos, camera.GetPosition() ).Normalized() };

	ColorRGB finalColor{};
	for ( auto& light : lights )
	{
		Vector3 lightToPoint{};
		switch ( light.type )
		{
		case LightType::point:
			lightToPoint = Vector3{ light.vector, pixelPos }.Normalized();
			break;

		case LightType::directional:
			lightToPoint = light.vector;
			break;
		}

		// List of hardcoded values because
		constexpr float diffuseReflectance{ 7.f }; // Hardcoded to make up for lack of lights
		constexpr float shininess{ 25.f };
		constexpr ColorRGB ambientLight{ 0.03f, 0.03f, 0.03f };

		const float observedArea{ lightUtils::GetObservedArea( light, pixelPos, sampledNormal ) };
		const ColorRGB radiance{ lightUtils::GetRadiance( light, pixelPos ) };
		const ColorRGB lambertDiffuse{ ( diffuseColor * diffuseReflectance ) / PI };
		const ColorRGB phongSpecular{ lightUtils::GetPhong(
			sampledSpecularity, sampledGloss * shininess, lightToPoint, toCameraDir, sampledNormal ) };
		const ColorRGB brdf{ lambertDiffuse + phongSpecular + ambientLight };

		switch ( lightingMode )
		{
		case LightingMode::observedArea:
			finalColor += ColorRGB{ observedArea, observedArea, observedArea };
			break;

		case LightingMode::diffuse:
			finalColor += lambertDiffuse;
			break;

		case LightingMode::specular:
			finalColor += phongSpecular;
			break;

		case LightingMode::combined:
			finalColor += observedArea * radiance * brdf;
			break;

		default:
			break;
		}
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

ColorRGB GetPhong( ColorRGB specularReflectance,
				   float phongExponent,
				   const Vector3& lightIncomingDir,
				   const Vector3& toCameraDir,
				   const Vector3& normal )
{
	const Vector3 reflect{ Vector3::Reflect( lightIncomingDir, normal ) };
	const float dot{ std::max( Vector3::Dot( reflect, toCameraDir ), 0.f ) };
	const ColorRGB phongReflection{ specularReflectance * std::pow( dot, phongExponent ) };

	return phongReflection;
}
} // namespace lightUtils
} // namespace dae

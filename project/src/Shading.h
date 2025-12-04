#ifndef SHADING_H
#define SHADING_H
#include "Camera.h"
#include "ColorRGB.h"
#include "DataTypes.h"

// Everything related to shading

enum class LightingMode;

namespace dae
{
enum class LightType
{
	point,
	directional
};

struct Light final
{
	Vector3 vector{}; // represents either a direction or an origin
	ColorRGB color{};
	float intensity{};

	LightType type{};
};

ColorRGB GetPixelColor( const Mesh& mesh,
						const VertexOut& pixelVertex,
						const Camera& camera,
						const std::vector<Light>& lights,
						const LightingMode& lightingMode,
						bool useNormalMap = true );

namespace lightUtils
{
float GetObservedArea( const Light& light, const Vector3& position, const Vector3& normal );
ColorRGB GetRadiance( const Light& light, const Vector3& position );
ColorRGB GetPhong( ColorRGB specularReflectance,
				   float phongExponent,
				   const Vector3& lightIncomingDir,
				   const Vector3& toCameraDir,
				   const Vector3& normal );
} // namespace lightUtils
} // namespace dae

#endif

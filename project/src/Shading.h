#ifndef SHADING_H
#define SHADING_H
#include "ColorRGB.h"
#include "DataTypes.h"

// Everything related to shading

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

ColorRGB GetPixelColor( const Mesh& mesh, const VertexOut& pixelVertex, const std::vector<Light>& lights );

namespace lightUtils
{
float GetObservedArea( const Light& light, const Vector3& position, const Vector3& normal );
ColorRGB GetRadiance( const Light& light, const Vector3& position );
} // namespace lightUtils
} // namespace dae

#endif

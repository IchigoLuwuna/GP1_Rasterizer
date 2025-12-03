#ifndef DATATYPES_H
#define DATATYPES_H

#include <vector>
#include <array>
#include "Vector2.h"
#include "Matrix.h"
#include "Texture.h"

namespace dae
{
struct Vertex
{
	Vector3 position{};
	ColorRGB color{ colors::White };
	Vector2 uv{};
	Vector3 normal{};
	Vector3 tangent{};
	// Vector3 viewDirection{};
};

struct VertexOut
{
	Vector4 position{};
	ColorRGB color{ colors::White };
	Vector2 uv{};
	Vector3 normal{};
	Vector3 tangent{};
	// Vector3 viewDirection{};
};

struct Rectangle
{
	float left{};
	float right{};
	float top{};
	float bottom{};
};

struct Triangle_Out
{
	Triangle_Out() = default;
	Triangle_Out( const VertexOut& v0, const VertexOut& v1, const VertexOut& v2 )
		: v0{ v0 }
		, v1{ v1 }
		, v2{ v2 }
		, normal{
			Vector3::Cross( Vector3( v0.position, v2.position ), Vector3( v0.position, v1.position ) ).Normalized()
		}
	{
	}

	VertexOut v0{};
	VertexOut v1{};
	VertexOut v2{};

	Vector3 normal{};

	const Texture* pTexture{};

	Rectangle GetBounds()
	{
		Rectangle rectangle{};
		std::array<float, 3> axisContainer{};

		axisContainer[0] = v0.position.x;
		axisContainer[1] = v1.position.x;
		axisContainer[2] = v2.position.x;

		rectangle.left = *std::min_element( axisContainer.begin(), axisContainer.end() );
		rectangle.right = *std::max_element( axisContainer.begin(), axisContainer.end() );

		axisContainer[0] = v0.position.y;
		axisContainer[1] = v1.position.y;
		axisContainer[2] = v2.position.y;

		rectangle.top = *std::min_element( axisContainer.begin(), axisContainer.end() );
		rectangle.bottom = *std::max_element( axisContainer.begin(), axisContainer.end() );

		return rectangle;
	}
};

enum class PrimitiveTopology
{
	TriangleList,
	TriangleStrip
};

struct Mesh
{
	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
	std::vector<Vertex> transformedVertices{};
	PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

	std::vector<VertexOut> vertices_out{};
	Matrix worldMatrix{};

	Texture texture{};
};
} // namespace dae
#endif

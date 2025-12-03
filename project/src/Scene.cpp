#include "Scene.h"
#include "DataTypes.h"
#include "Utils.h"
using namespace dae;

// Scene base
Scene::Scene( const Camera& camera, const std::vector<Mesh>& meshes )
	: m_Camera{ camera }
	, m_Meshes{ meshes }
{
}

Scene::Scene( const Camera& camera, std::vector<Mesh>&& meshes )
	: m_Camera{ camera }
	, m_Meshes{ std::move( meshes ) }
{
}

const Camera& Scene::GetCamera() const
{
	return m_Camera;
}

const std::vector<Mesh>& Scene::GetMeshes() const
{
	return m_Meshes;
}

void Scene::Update( Timer* pTimer )
{
	m_Camera.Update( pTimer );
}

// Week 1 Scene
void SceneW1::Initialize()
{
	const Vector3 cameraOrigin{ 0.f, 0.0f, -10.f };
	constexpr float fovAngle{ 60.f };

	m_Camera.SetPos( cameraOrigin );
	m_Camera.SetFovAngleDegrees( fovAngle );
	m_Camera.Rotate( 0.f, 0.f );

	std::vector<Mesh> meshes{};

	Vertex v0{ Vector3{ 0.f, 2.f, 0.f }, ColorRGB{ 1.f, 0.f, 0.f } };
	Vertex v1{ Vector3{ 2.f, 0.f, 0.f }, ColorRGB{ 0.f, 1.f, 0.f } };
	Vertex v2{ Vector3{ -1.f, 0.f, 0.f }, ColorRGB{ 0.f, 0.f, 1.f } };

	Mesh triangle{};
	triangle.vertices = { v0, v1, v2 };
	triangle.indices = { 0, 1, 2 };
	triangle.primitiveTopology = PrimitiveTopology::TriangleList;

	Vertex v3{ Vector3{ 1.f, 2.f, 0.5f }, ColorRGB{ 1.f, 0.f, 0.f } };
	Vertex v4{ Vector3{ 2.f, 0.f, 1.f }, ColorRGB{ 0.f, 1.f, 0.f } };
	Vertex v5{ Vector3{ 0.f, 0.f, -1.f }, ColorRGB{ 0.f, 0.f, 1.f } };

	Mesh triangle2{};
	triangle2.vertices = { v3, v4, v5 };
	triangle2.indices = { 0, 1, 2 };
	triangle2.primitiveTopology = PrimitiveTopology::TriangleList;

	meshes.push_back( std::move( triangle ) );
	meshes.push_back( std::move( triangle2 ) );

	m_Meshes = std::move( meshes );
}

// Week 2 Scene
void SceneW2::Initialize()
{
	const Vector3 cameraOrigin{ 0.f, 0.0f, -10.f };
	constexpr float fovAngle{ 60.f };

	m_Camera.SetPos( cameraOrigin );
	m_Camera.SetFovAngleDegrees( fovAngle );
	m_Camera.Rotate( 0.f, 0.f );

	std::vector<Mesh> meshes{};

	Mesh mesh{ std::vector<Vertex>{ { { -3.f, 3.f, -2.f }, colors::Red, { 0.f, 0.f } },
									{ { 0.f, 3.f, -2.f }, colors::Green, { 0.5f, 0.f } },
									{ { 3.f, 3.f, -2.f }, colors::Blue, { 1.f, 0.f } },
									{ { -3.f, 0.f, -2.f }, colors::Red, { 0.f, 0.5f } },
									{ { 0.f, 0.f, -2.f }, colors::Green, { 0.5f, 0.5f } },
									{ { 3.f, 0.f, -2.f }, colors::Blue, { 1.f, 0.5f } },
									{ { -3.f, -3.f, -2.f }, colors::Red, { 0.f, 1.f } },
									{ { 0.f, -3.f, -2.f }, colors::Green, { 0.5f, 1.f } },
									{ { 3.f, -3.f, -2.f }, colors::Blue, { 1.f, 1.f } } },
			   std::vector<uint32_t>{ 3, 0, 4, 1, 5, 2, 2, 6, 6, 3, 7, 4, 8, 5 } };
	mesh.primitiveTopology = PrimitiveTopology::TriangleStrip;

	mesh.texture = Texture{ "./resources/uv_grid_2.png" };

	meshes.push_back( std::move( mesh ) );

	m_Meshes = std::move( meshes );
}

// Week 4 Scene
void SceneW4::Initialize()
{
	const Vector3 cameraOrigin{ 0.f, 0.0f, -20.f };
	constexpr float fovAngle{ 60.f };

	m_Camera.SetPos( cameraOrigin );
	m_Camera.SetFovAngleDegrees( fovAngle );
	m_Camera.Rotate( 0.f, 0.f );

	std::vector<Mesh> meshes{};

	Mesh mesh{};
	Utils::ParseOBJ( "./resources/tuktuk.obj", mesh.vertices, mesh.indices );
	mesh.primitiveTopology = PrimitiveTopology::TriangleList;

	mesh.texture = Texture{ "./resources/tuktuk.png" };

	meshes.push_back( std::move( mesh ) );

	m_Meshes = std::move( meshes );
}

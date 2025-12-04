#include "Scene.h"
#include <SDL_keyboard.h>
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

const std::vector<Light>& Scene::GetLights() const
{
	return m_Lights;
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

// Week 5 Scene
void SceneW5::Update( Timer* pTimer )
{
	Scene::Update( pTimer );

	const uint8_t* pKeyboardState{ SDL_GetKeyboardState( nullptr ) };

	if ( !m_F5Held && pKeyboardState[SDL_SCANCODE_F5] )
	{
		m_IsRotating = !m_IsRotating;
		m_F5Held = true;
	}
	else if ( m_F5Held && !pKeyboardState[SDL_SCANCODE_F5] )
	{
		m_F5Held = false;
	}

	if ( !m_IsRotating )
	{
		return;
	}

	m_Yaw += pTimer->GetElapsed();
	m_Meshes[0].worldMatrix = Matrix::CreateRotationY( m_Yaw );

	for ( auto& mesh : m_Meshes )
	{
		mesh.UpdateMesh();
	}
}

void SceneW5::Initialize()
{
	const Vector3 cameraOrigin{ 0.f, 5.f, -64.f };
	constexpr float fovAngle{ 45.f };

	m_Camera.SetPos( cameraOrigin );
	m_Camera.SetFovAngleDegrees( fovAngle );
	m_Camera.Rotate( 0.f, 0.f );

	std::vector<Mesh> meshes{};

	Mesh mesh{};
	Utils::ParseOBJ( "./resources/vehicle.obj", mesh.vertices, mesh.indices );
	mesh.primitiveTopology = PrimitiveTopology::TriangleList;

	mesh.transformedVertices = std::vector<Vertex>( mesh.vertices.size() );

	mesh.UpdateMesh();

	mesh.texture = Texture{ "./resources/vehicle_diffuse.png" };
	mesh.normalMap = Texture{ "./resources/vehicle_normal.png" };
	mesh.specularMap = Texture{ "./resources/vehicle_specular.png" };
	mesh.glossMap = Texture{ "./resources/vehicle_gloss.png" };

	meshes.push_back( std::move( mesh ) );

	const Light light{ Vector3{ 0.577f, -0.577f, 0.577f }, { 1.f, 1.f, 1.f }, 1.f, LightType::directional };
	m_Lights.push_back( light );

	m_Meshes = std::move( meshes );
}

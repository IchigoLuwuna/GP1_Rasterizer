#ifndef SCENE_H
#define SCENE_H

// Local includes
#include "Camera.h"
#include "DataTypes.h"
#include "Shading.h"

namespace dae
{
class Scene
{
public:
	Scene() = default;
	Scene( const Camera& camera, const std::vector<Mesh>& meshes );
	Scene( const Camera& camera, std::vector<Mesh>&& meshes );
	Scene( const Scene& ) = delete;
	Scene( Scene&& ) = delete;
	virtual ~Scene() = default;

	virtual Scene& operator=( const Scene& ) = delete;
	virtual Scene& operator=( Scene&& ) = delete;

	const Camera& GetCamera() const;
	const std::vector<Mesh>& GetMeshes() const;
	const std::vector<Light>& GetLights() const;

	virtual void Update( Timer* pTimer );
	virtual void Initialize() = 0;

protected:
	Camera m_Camera{ {}, 0.f };
	std::vector<Mesh> m_Meshes{};
	std::vector<Light> m_Lights{};
};

class SceneW1 final : public Scene
{
public:
	virtual void Initialize() override;
};

class SceneW2 final : public Scene
{
public:
	virtual void Initialize() override;
};

class SceneW4 final : public Scene
{
public:
	virtual void Initialize() override;
};

class SceneW5 final : public Scene
{
public:
	virtual void Initialize() override;
};
} // namespace dae

#endif

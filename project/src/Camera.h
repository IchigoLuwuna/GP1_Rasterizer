#ifndef CAMERA_H
#define CAMERA_H

#include <SDL_mouse.h>
#include "Maths.h"
#include "Timer.h"

namespace dae
{
class Camera final
{
public:
	Camera() = default;
	explicit Camera( const Vector3& origin, float fovAngle, float near = 0.1f, float far = 100.f );

	// Getters
	const Matrix& GetCameraToWorld() const;
	const Vector3& GetPosition() const;
	float GetFov() const;
	float GetNear() const;
	float GetFar() const;

	// Setters
	void SetPos( const Vector3& newPos );
	void SetFovAngleDegrees( float newFovAngle );

	// Methods
	void Update( Timer* pTimer );
	void Move( const Vector3& change );
	void Rotate( float yaw, float pitch );

private:
	// Members
	Vector3 m_Origin{};
	Vector3 m_Forward{ Vector3::UnitZ };
	Vector3 m_Up{ Vector3::UnitY };
	Vector3 m_Right{ Vector3::UnitX };

	float m_FovAngle{ 90.f / 180.f * PI };
	float m_Fov{ tanf( m_FovAngle * 0.5f ) };
	float m_TotalPitch{ 0.f };
	float m_TotalYaw{ 0.f };

	float m_Near{};
	float m_Far{};

	Matrix m_CameraToWorld{};
	bool m_UpdateONB{ true };

	// Methods
	Matrix CalculateCameraToWorld();
};
} // namespace dae
#endif

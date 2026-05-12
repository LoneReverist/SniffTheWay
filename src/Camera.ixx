// Camera.ixx

module;

#include <numbers>

#include <glm/vec3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

export module Camera;

import Input;

export struct ViewProjUniform
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

export struct CameraPosUniform
{
	alignas(16) glm::vec3 pos;
};

export class Camera
{
public:
	explicit Camera(bool flip_proj_y = false) : m_flip_proj_y(flip_proj_y) {}

	void Init(glm::vec3 const & pos, glm::vec3 const & dir);
	void OnViewportResized(int width, int height);

	void Update(float dt, Input const & input);

	CameraPosUniform const & GetPosUniform() const { return m_pos_uniform; }
	glm::vec3 const & GetDir() const { return m_dir; }
	ViewProjUniform const & GetViewProjUniform() const { return m_view_proj_uniform; }

private:
	CameraPosUniform m_pos_uniform{ { 0.0f, 0.0f, 0.0f } };
	glm::vec3 m_dir{ 0.0f, 0.0f, -1.0f };
	ViewProjUniform m_view_proj_uniform;

	bool m_flip_proj_y = false; // whether to flip the y-axis in the projection matrix

	static constexpr glm::vec3 UpDir{ 0.0f, 0.0f, 1.0f }; // a little atypical, but i prefer Z to be up
	static constexpr float FOV = glm::radians(45.0f);
	static constexpr float NearPlane = 0.1f;
	static constexpr float FarPlane = 100.0f;
};

void Camera::Init(glm::vec3 const & pos, glm::vec3 const & dir)
{
	m_pos_uniform.pos = pos;
	m_dir = dir;
	m_view_proj_uniform.view = glm::lookAt(m_pos_uniform.pos, m_pos_uniform.pos + m_dir, UpDir);
}

void Camera::OnViewportResized(int width, int height)
{
	if (height == 0)
		return;

	const float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
	m_view_proj_uniform.proj = glm::perspective(FOV, aspect_ratio, NearPlane, FarPlane);

	if (m_flip_proj_y)
		m_view_proj_uniform.proj[1][1] *= -1; // account for vulkan having flipped y-axis compared to opengl
}

void Camera::Update(float dt, Input const & input)
{
}

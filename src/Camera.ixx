// Camera.ixx

module;

#include <cmath>
#include <numbers>
#include <optional>

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>

export module Camera;

import Input;

export struct ViewProjUniform
{
	alignas(16) glm::mat4 view{ 1.0f };
	alignas(16) glm::mat4 proj{ 1.0f };
};

export struct ProjUniform
{
	alignas(16) glm::mat4 proj{ 1.0f };
};

export struct ViewportUniform
{
	alignas(16) glm::vec2 size{ 0.0f };
};

export struct CameraPosUniform
{
	alignas(16) glm::vec3 pos{ 0.0f };
};

export class Camera3d
{
public:
	explicit Camera3d(bool flip_proj_y = false) : m_flip_proj_y(flip_proj_y) {}

	void Init(glm::vec3 const & pos, glm::vec3 const & dir);
	void Init(glm::vec3 const & pos, glm::vec3 const & dir, float fov_degrees);
	void SetViewportSize(int width, int height);
	void SetPosition(glm::vec3 const & pos);
	void SetDirection(glm::vec3 const & dir);
	void SetFovDegrees(float fov_degrees);
	std::optional<glm::vec2> ScreenPointToGround(glm::vec2 framebuffer_pos, glm::ivec4 viewport) const;

	void Update(float dt, Input const & input) {}

	CameraPosUniform const & GetPosUniform() const { return m_pos_uniform; }
	glm::vec3 const & GetPosition() const { return m_pos_uniform.pos; }
	glm::vec3 const & GetDir() const { return m_dir; }
	float GetFovDegrees() const { return m_fov_degrees; }
	ViewProjUniform const & GetViewProjUniform() const { return m_view_proj_uniform; }
	ViewportUniform const & GetViewportUniform() const { return m_viewport_uniform; }

private:
	void update_view();
	void update_projection();

	ViewProjUniform m_view_proj_uniform;
	ViewportUniform m_viewport_uniform;
	CameraPosUniform m_pos_uniform{ { 0.0f, 0.0f, 0.0f } };
	glm::vec3 m_dir{ 0.0f, 0.0f, -1.0f };
	float m_fov_degrees = 45.0f;
	int m_viewport_width = 0;
	int m_viewport_height = 0;

	bool m_flip_proj_y = false; // whether to flip the y-axis in the projection matrix

	static constexpr glm::vec3 UpDir{ 0.0f, 0.0f, 1.0f }; // a little atypical, but i prefer Z to be up
	static constexpr float NearPlane = 0.1f;
	static constexpr float FarPlane = 100.0f;
};

export class Camera2d
{
public:
	explicit Camera2d(bool flip_proj_y = false) : m_flip_proj_y(flip_proj_y) {}

	void Init(float left, float right, float top, float bottom);
	void SetViewportSize(int width, int height);

	void Update(float dt, Input const & input) {}

	ProjUniform const & GetProjUniform() const { return m_proj_uniform; }
	ViewportUniform const & GetViewportUniform() const { return m_viewport_uniform; }

private:
	ProjUniform m_proj_uniform;
	ViewportUniform m_viewport_uniform;

	bool m_flip_proj_y = false; // whether to flip the y-axis in the projection matrix
};

void Camera3d::Init(glm::vec3 const & pos, glm::vec3 const & dir)
{
	Init(pos, dir, m_fov_degrees);
}

void Camera3d::Init(glm::vec3 const & pos, glm::vec3 const & dir, float fov_degrees)
{
	m_pos_uniform.pos = pos;
	SetDirection(dir);
	SetFovDegrees(fov_degrees);
}

void Camera3d::SetViewportSize(int width, int height)
{
	if (height == 0)
		return;

	m_viewport_width = width;
	m_viewport_height = height;
	m_viewport_uniform.size = glm::vec2{ static_cast<float>(width), static_cast<float>(height) };
	update_projection();
}

void Camera3d::SetPosition(glm::vec3 const & pos)
{
	m_pos_uniform.pos = pos;
	update_view();
}

void Camera3d::SetDirection(glm::vec3 const & dir)
{
	if (glm::length(dir) < 1e-6f)
		return;

	m_dir = glm::normalize(dir);
	update_view();
}

void Camera3d::SetFovDegrees(float fov_degrees)
{
	if (fov_degrees <= 0.0f)
		return;

	m_fov_degrees = fov_degrees;
	update_projection();
}

void Camera3d::update_view()
{
	m_view_proj_uniform.view = glm::lookAt(m_pos_uniform.pos, m_pos_uniform.pos + m_dir, UpDir);
}

void Camera3d::update_projection()
{
	if (m_viewport_height == 0)
		return;

	const float aspect_ratio = static_cast<float>(m_viewport_width) / static_cast<float>(m_viewport_height);
	m_view_proj_uniform.proj = glm::perspective(glm::radians(m_fov_degrees), aspect_ratio, NearPlane, FarPlane);

	if (m_flip_proj_y)
		m_view_proj_uniform.proj[1][1] *= -1; // account for vulkan having flipped y-axis compared to opengl
}

std::optional<glm::vec2> Camera3d::ScreenPointToGround(glm::vec2 framebuffer_pos, glm::ivec4 viewport) const
{
	if (viewport.z <= 0 || viewport.w <= 0)
		return std::nullopt;

	if (framebuffer_pos.x < static_cast<float>(viewport.x) ||
		framebuffer_pos.y < static_cast<float>(viewport.y) ||
		framebuffer_pos.x > static_cast<float>(viewport.x + viewport.z) ||
		framebuffer_pos.y > static_cast<float>(viewport.y + viewport.w))
	{
		return std::nullopt;
	}

	glm::vec2 const viewport_pos{
		framebuffer_pos.x - static_cast<float>(viewport.x),
		framebuffer_pos.y - static_cast<float>(viewport.y)
	};

	const float ndc_x = viewport_pos.x / static_cast<float>(viewport.z) * 2.0f - 1.0f;
	const float ndc_y = m_flip_proj_y
		? viewport_pos.y / static_cast<float>(viewport.w) * 2.0f - 1.0f
		: 1.0f - viewport_pos.y / static_cast<float>(viewport.w) * 2.0f;

	glm::mat4 const inv_view_proj = glm::inverse(m_view_proj_uniform.proj * m_view_proj_uniform.view);
	glm::vec4 near_world = inv_view_proj * glm::vec4{ ndc_x, ndc_y, -1.0f, 1.0f };
	glm::vec4 far_world = inv_view_proj * glm::vec4{ ndc_x, ndc_y, 1.0f, 1.0f };

	if (std::abs(near_world.w) < 1e-6f || std::abs(far_world.w) < 1e-6f)
		return std::nullopt;

	near_world /= near_world.w;
	far_world /= far_world.w;

	glm::vec3 const ray_start{ near_world };
	glm::vec3 const ray_end{ far_world };
	glm::vec3 const ray_dir = glm::normalize(ray_end - ray_start);

	if (std::abs(ray_dir.z) < 1e-6f)
		return std::nullopt;

	const float t = -ray_start.z / ray_dir.z;
	if (t < 0.0f)
		return std::nullopt;

	glm::vec3 const hit = ray_start + ray_dir * t;
	return glm::vec2{ hit.x, hit.y };
}

void Camera2d::Init(float left, float right, float top, float bottom)
{
	if (m_flip_proj_y)
		m_proj_uniform.proj = glm::ortho(left, right, top, bottom, -1.0f, 1.0f);
	else
		m_proj_uniform.proj = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
}

void Camera2d::SetViewportSize(int width, int height)
{
	if (width == 0 || height == 0)
		return;

	m_viewport_uniform.size = glm::vec2{ static_cast<float>(width), static_cast<float>(height) };
}

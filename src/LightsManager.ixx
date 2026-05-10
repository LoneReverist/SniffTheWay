// LightsManager.ixx

module;

#include <glm/vec3.hpp>

export module LightsManager;

export struct AmbientLight
{
	alignas(16) glm::vec3 color{ 1.0, 1.0, 1.0 };
};

export struct LightsUniform
{
	alignas(16) AmbientLight ambient_light;
};

export class LightsManager
{
public:
	void SetAmbientLight(AmbientLight const & light) { m_lights.ambient_light = light; }

	LightsUniform const & GetLightsUniform() const { return m_lights; }

private:
	LightsUniform m_lights;
};

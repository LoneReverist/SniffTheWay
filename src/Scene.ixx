// Scene.ixx

module;

#include <string>
#include <filesystem>

export module Scene;

import Dreamhearth;

import Input;

using namespace Dreamhearth;

export class Scene
{
public:
	explicit Scene(RenderContext const & render_context, std::string const & title, float dpi_scale_factor);

	void OnViewportResized(int width, int height);
	void OnDPIScalingFactorChanged(float dpi_scale_factor);

	bool Update(float dt, Input const & input);
	void Render() const;

private:
	RenderContext const & m_render_context;
	std::filesystem::path const m_resources_path;
	std::string const m_title;

	Renderer m_renderer;
};

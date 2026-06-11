// FPSLabel.ixx

module;

#include <string>

#include <glm/vec2.hpp>

export module FPSLabel;

import AssetManager;
import Camera;
import FontAtlas;
import SceneRenderer;
import SniffTheWayConstants;
import UILabel;
import UIShadowedLabel;

export class FPSLabel
{
public:
	void Init(AssetManager & asset_manager, SceneRenderer & renderer, Camera2d const & camera2d, FontAtlas const & font_atlas);
	void Update(float dt);
	void RenderOffscreenTexture() const;

private:
	UIShadowedLabel m_label;

	float m_frame_timer = 0.0f;
	int m_frame_count = 0;
};

void FPSLabel::Init(AssetManager & asset_manager, SceneRenderer & renderer, Camera2d const & camera2d, FontAtlas const & font_atlas)
{
	m_label.Init(
		asset_manager,
		renderer,
		camera2d,
		"fps",
		"FPS: ",
		font_atlas,
		SniffTheWay::LabelFontSize,
		glm::vec2{ 96, 1026 } /*origin*/,
		UILabel::Align::Left,
		SniffTheWay::StoryTextColor);
}

void FPSLabel::Update(float dt)
{
	m_frame_timer += dt;
	m_frame_count++;
	if (m_frame_timer >= 1.0)
	{
		float fps = static_cast<float>(m_frame_count) / m_frame_timer;
		m_label.SetText("FPS: " + std::to_string(static_cast<int>(fps)));
		m_frame_timer = 0.0;
		m_frame_count = 0;
	}
}

void FPSLabel::RenderOffscreenTexture() const
{
	m_label.RenderOffscreenTexture();
}

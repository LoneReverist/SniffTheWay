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
import UIShadowedText;

export class FPSLabel
{
public:
	void Init(AssetManager & asset_manager, SceneRenderer & renderer, Camera2d const & camera2d, FontAtlas const & font_atlas);
	void Update(float dt);
	void RenderOffscreenTexture() const;

private:
	UIShadowedText m_text;

	float m_frame_timer = 0.0f;
	int m_frame_count = 0;
};

void FPSLabel::Init(AssetManager & asset_manager, SceneRenderer & renderer, Camera2d const & camera2d, FontAtlas const & font_atlas)
{
	m_text.Init(
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
		m_text.SetText("FPS: " + std::to_string(static_cast<int>(fps)));
		m_frame_timer = 0.0;
		m_frame_count = 0;
	}
}

void FPSLabel::RenderOffscreenTexture() const
{
	m_text.RenderOffscreenTexture();
}

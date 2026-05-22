// FPSLabel.ixx

module;

#include <memory>
#include <string>

#include <glm/vec2.hpp>

export module FPSLabel;

import AssetManager;
import FontAtlas;
import SniffTheWayConstants;
import UILabel;

export class FPSLabel
{
public:
	void Init(AssetManager & asset_manager, FontAtlas const & font_atlas);
	void Update(float dt);

	void OnViewportResized(int width, int height);
	void OnDPIScaleFactorChanged(float dpi_scale_factor);

	UILabel const * GetUILabel() const { return m_ui_label.get(); }

private:
	std::unique_ptr<UILabel> m_ui_label;

	float m_frame_timer = 0.0f;
	int m_frame_count = 0;
};

void FPSLabel::Init(AssetManager & asset_manager, FontAtlas const & font_atlas)
{
	m_ui_label = std::make_unique<UILabel>(asset_manager, "FPS: ", font_atlas,
		SniffTheWay::LabelFontSize, glm::vec2{ -0.9, -0.9 } /*origin*/, SniffTheWay::StoryTextColor);
}

void FPSLabel::Update(float dt)
{
	m_frame_timer += dt;
	m_frame_count++;
	if (m_frame_timer >= 1.0)
	{
		float fps = static_cast<float>(m_frame_count) / m_frame_timer;
		m_ui_label->SetText("FPS: " + std::to_string(static_cast<int>(fps)));
		m_frame_timer = 0.0;
		m_frame_count = 0;
	}
}

void FPSLabel::OnViewportResized(int width, int height)
{
	if (m_ui_label)
		m_ui_label->OnViewportResized(width, height);
}

void FPSLabel::OnDPIScaleFactorChanged(float dpi_scale_factor)
{
	if (m_ui_label)
		m_ui_label->SetFontSize(SniffTheWay::LabelFontSize * dpi_scale_factor);
}

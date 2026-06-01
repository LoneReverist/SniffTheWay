// SceneCreek.ixx

module;

#include <array>
#include <string_view>

export module SceneCreek;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class SceneCreek : public StoryScene
{
public:
	explicit SceneCreek(dh::RenderContext const & render_context);

private:
	// Purpose: Show growing teamwork and deepen emotional bond.
	// Emotional arc: Obstacle → fear → trust → teamwork
	static constexpr std::array<std::string_view, 4> StoryImages{
		"approaching_creek.png",
		"baby_reluctant.png",
		"crossing_creek.png",
		"beyond_the_creek.png",
	};
	static constexpr std::array<std::string_view, 4> StoryTexts{
		// water blocks path
		"The forest grew thicker.<pause>And soon,\nthe path disappeared.<pause>",
		// words appear slower, slight wobble on "Too scary."
		"The water looked cold.<pause>Too far.<pause>Too scary.<pause>",
		// dog guiding path
		"But the puppy searched carefully.<pause>One safe step...<pause>then another.<pause>",
		// they make it across, music becomes hopeful, camera slowly pushes in
		"Together,<pause>they kept going.<pause>",
	};
};

SceneCreek::SceneCreek(dh::RenderContext const & render_context)
	: StoryScene(render_context, StoryImages, StoryTexts)
{
	m_next_scene_id = SceneId::DarkForest;
}

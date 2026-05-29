// SceneHome.ixx

module;

#include <array>
#include <string_view>

export module SceneHome;

import Dreamhearth;
namespace dh = Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

using namespace SniffTheWay;

export class SceneHome : public StoryScene
{
public:
	explicit SceneHome(dh::RenderContext const & render_context);

private:
	// Purpose: Emotional payoff.
	// Emotional arc: Recognition → relief → reunion → peace
	static constexpr std::array<std::string_view, 3> StoryImages{
		"home.png",
		//"parents_notice.png",
		"reunion.png",
		"safe_again.png",
		//"dog_by_fireplace.png",
	};
	static constexpr std::array<std::string_view, 5> StoryTexts{
		// parents far away, hold this longer than usual.
		"Then,<pause>something familiar.<pause>Home.<pause>",
		// parents notice
		"Voices called out.<pause>Feet hurried down the path.<pause>",
		// mom hugs baby, dad hugs dog, Keep it simple. Let visuals carry emotion.
		"Safe again.<pause>Held tight.<pause>Loved deeply.<pause>",
		// calm ending, family together by the fire
		"<pause>",
		// dog and baby cuddled together
		"A loyal nose.<pause>A brave heart.<pause>Home at last.",
		// fade to credits
	};
};

SceneHome::SceneHome(dh::RenderContext const & render_context)
	: StoryScene(render_context, StoryImages, StoryTexts)
{
	m_next_scene_id = SceneId::Exit;
}

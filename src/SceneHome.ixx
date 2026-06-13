// SceneHome.ixx

module;

#include <array>
#include <string_view>

export module SceneHome;

import Dreamhearth;

import SniffTheWayConstants;
import StoryScene;

namespace dh = Dreamhearth;
using namespace SniffTheWay;

export class SceneHome : public StoryScene
{
public:
	explicit SceneHome(dh::RenderContext const & render_context);

private:
	// Purpose: Emotional payoff.
	// Emotional arc: Recognition -> relief -> reunion -> peace
	// parents far away, hold this longer than usual.
	static constexpr std::array<StoryText, 1> HomeTexts{
		StoryText{ .text = "Then,<pause>out from the trees,<pause>there it was.<pause>Home.<pause>", .font_size = TitleFontSize },
	};
	// parents notice
	static constexpr std::array<StoryText, 1> ParentsNoticeTexts{
		StoryText{ .text = "Voices called out.<pause>Feet hurried down the path.<pause>", .font_size = TitleFontSize },
	};
	// mom hugs baby, dad hugs dog, Keep it simple. Let visuals carry emotion.
	static constexpr std::array<StoryText, 1> ReunionTexts{
		StoryText{ .text = "Arms wrapped tight.<pause>Happy tears fell.<pause>The little one was safe.<pause>The dog had brought him home.<pause>", .font_size = TitleFontSize },
	};
	// calm ending, family together by the fire
	static constexpr std::array<StoryText, 1> SafeAgainTexts{
		StoryText{ .text = "That evening,<pause>the house glowed warm.<pause>The long day softened.<pause>The woods felt far away.<pause>", .font_size = TitleFontSize },
	};
	// dog and baby cuddled together
	static constexpr std::array<StoryText, 1> DogByFireplaceTexts{
		StoryText{ .text = "A loyal nose.<pause>A brave heart.<pause>Home at last.", .font_size = TitleFontSize },
	};
	// fade to credits
	static constexpr std::array<StoryPage, 5> StoryPages{
		StoryPage{ .bg_image_filename = "home.png", .story_texts = HomeTexts },
		StoryPage{ .bg_image_filename = "parents_notice.png", .story_texts = ParentsNoticeTexts },
		StoryPage{ .bg_image_filename = "reunion.png", .story_texts = ReunionTexts },
		StoryPage{ .bg_image_filename = "safe_again.png", .story_texts = SafeAgainTexts },
		StoryPage{ .bg_image_filename = "dog_by_fireplace.png", .story_texts = DogByFireplaceTexts },
	};
	static constexpr SceneId NextSceneId = SceneId::Exit;
};

SceneHome::SceneHome(dh::RenderContext const & render_context)
	: StoryScene(render_context, StoryPages, NextSceneId)
{
}

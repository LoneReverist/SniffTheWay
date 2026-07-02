// GameViewport.ixx

module;

#include <algorithm>
#include <cstdint>
#include <optional>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

export module GameViewport;

import SniffTheWayConstants;

export namespace SniffTheWay
{
	struct GameViewport
	{
		glm::ivec4 pixels{ 0, 0, 0, 0 };

		constexpr bool IsValid() const;
		bool Contains(glm::vec2 framebuffer_position) const;
		std::optional<glm::vec2> FramebufferToUI(glm::vec2 framebuffer_position) const;
	};

	constexpr bool GameViewport::IsValid() const
	{
		return pixels.z > 0 && pixels.w > 0;
	}

	bool GameViewport::Contains(glm::vec2 framebuffer_position) const
	{
		return IsValid()
			&& framebuffer_position.x >= static_cast<float>(pixels.x)
			&& framebuffer_position.x < static_cast<float>(pixels.x + pixels.z)
			&& framebuffer_position.y >= static_cast<float>(pixels.y)
			&& framebuffer_position.y < static_cast<float>(pixels.y + pixels.w);
	}

	std::optional<glm::vec2> GameViewport::FramebufferToUI(glm::vec2 framebuffer_position) const
	{
		if (!Contains(framebuffer_position))
			return std::nullopt;

		return glm::vec2{
			(framebuffer_position.x - static_cast<float>(pixels.x)) * UIWidth / static_cast<float>(pixels.z),
			(framebuffer_position.y - static_cast<float>(pixels.y)) * UIHeight / static_cast<float>(pixels.w),
		};
	}

	constexpr GameViewport CalculateGameViewport(int framebuffer_width, int framebuffer_height)
	{
		if (framebuffer_width <= 0 || framebuffer_height <= 0)
			return {};

		int viewport_width = 0;
		int viewport_height = 0;

		// Compare cross-products so the aspect-ratio decision is exact and cannot
		// wobble by a pixel as the window is resized.
		if (std::int64_t{ framebuffer_width } * DesignHeight
			<= std::int64_t{ framebuffer_height } * DesignWidth)
		{
			viewport_width = framebuffer_width;
			viewport_height = static_cast<int>(
				(std::int64_t{ viewport_width } * DesignHeight + DesignWidth / 2) / DesignWidth);
		}
		else
		{
			viewport_height = framebuffer_height;
			viewport_width = static_cast<int>(
				(std::int64_t{ viewport_height } * DesignWidth + DesignHeight / 2) / DesignHeight);
		}

		viewport_width = std::clamp(viewport_width, 1, framebuffer_width);
		viewport_height = std::clamp(viewport_height, 1, framebuffer_height);

		return GameViewport{ .pixels = {
			(framebuffer_width - viewport_width) / 2,
			(framebuffer_height - viewport_height) / 2,
			viewport_width,
			viewport_height,
		} };
	}
}

namespace SniffTheWay
{
	static_assert(CalculateGameViewport(1920, 1080).pixels == glm::ivec4{ 0, 0, 1920, 1080 });
	static_assert(CalculateGameViewport(3440, 1440).pixels == glm::ivec4{ 440, 0, 2560, 1440 });
	static_assert(CalculateGameViewport(1600, 1200).pixels == glm::ivec4{ 0, 150, 1600, 900 });
	static_assert(CalculateGameViewport(1080, 1920).pixels == glm::ivec4{ 0, 656, 1080, 608 });
	static_assert(!CalculateGameViewport(0, 0).IsValid());
}

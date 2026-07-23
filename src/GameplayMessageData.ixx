// GameplayMessageData.ixx

module;

#include <string>

export module GameplayMessageData;

export struct GameplayMessage
{
	std::string text;
	float hold_duration = 4.0f;
	float fade_in_duration = 0.25f;
	float fade_out_duration = 0.75f;
};

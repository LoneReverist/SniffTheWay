module;

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <glog/logging.h>
#include <nlohmann/json.hpp>

export module SceneAudioJson;

import SniffTheWayConstants;
import SceneAudioData;

namespace
{
	template<class Cue>
	std::optional<Cue> ParseCue(nlohmann::ordered_json const & audio, char const * field,
		std::filesystem::path const & filepath, std::optional<Cue> (*from_string)(std::string_view))
	{
		if (!audio.contains(field) || audio[field].is_null())
			return std::nullopt;
		auto const & value = audio[field];
		if (value.is_string())
		{
			if (auto cue = from_string(value.get_ref<std::string const &>()))
				return cue;
		}
		LOG(WARNING) << "SceneAudioJson: Invalid audio." << field << " in " << filepath
			<< ": " << value.dump() << ". Using silence.";
		return std::nullopt;
	}

	template<class Cue>
	nlohmann::ordered_json SerializeCue(std::optional<Cue> cue)
	{
		if (cue)
		{
			const auto name = SniffTheWay::ToString(*cue);
			if (!name.empty())
				return std::string{ name };
		}
		return nullptr;
	}
}

export SceneAudioData ParseSceneAudio(nlohmann::ordered_json const & root,
	std::filesystem::path const & filepath)
{
	if (!root.contains("audio"))
		return {};
	auto const & audio = root["audio"];
	if (!audio.is_object())
	{
		LOG(WARNING) << "SceneAudioJson: Invalid audio object in " << filepath << ". Using silence.";
		return {};
	}
	return {
		.music = ParseCue(audio, "music", filepath, SniffTheWay::MusicCueFromString),
		.ambience = ParseCue(audio, "ambience", filepath, SniffTheWay::AmbienceCueFromString),
	};
}

export nlohmann::ordered_json SerializeSceneAudio(SceneAudioData const & data)
{
	return { { "music", SerializeCue(data.music) },
		{ "ambience", SerializeCue(data.ambience) } };
}

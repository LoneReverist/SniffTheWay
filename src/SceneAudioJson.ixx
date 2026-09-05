module;

#include <filesystem>
#include <optional>
#include <string>
#include <utility>

#include <glog/logging.h>
#include <nlohmann/json.hpp>

export module SceneAudioJson;

import AudioSystem;
import SceneAudioData;

namespace
{
	constexpr std::pair<char const *, MusicCue> MusicNames[] = {
		{ "title", MusicCue::Title }, { "picnic", MusicCue::Picnic },
		{ "early_forest", MusicCue::EarlyForest }, { "creek", MusicCue::Creek },
		{ "middle_forest", MusicCue::MiddleForest }, { "night", MusicCue::Night },
		{ "late_forest", MusicCue::LateForest }, { "home", MusicCue::Home },
	};
	constexpr std::pair<char const *, AmbienceCue> AmbienceNames[] = {
		{ "early_forest", AmbienceCue::EarlyForest }, { "creek", AmbienceCue::Creek },
		{ "middle_forest", AmbienceCue::MiddleForest }, { "night", AmbienceCue::Night },
		{ "late_forest", AmbienceCue::LateForest },
	};

	template<class Cue, std::size_t N>
	std::optional<Cue> ParseCue(nlohmann::ordered_json const & audio, char const * field,
		std::filesystem::path const & filepath, std::pair<char const *, Cue> const (&names)[N])
	{
		if (!audio.contains(field) || audio[field].is_null())
			return std::nullopt;
		auto const & value = audio[field];
		if (value.is_string())
		{
			for (auto const & [name, cue] : names)
				if (value.get_ref<std::string const &>() == name)
					return cue;
		}
		LOG(WARNING) << "SceneAudioJson: Invalid audio." << field << " in " << filepath
			<< ": " << value.dump() << ". Using silence.";
		return std::nullopt;
	}

	template<class Cue, std::size_t N>
	nlohmann::ordered_json SerializeCue(std::optional<Cue> cue,
		std::pair<char const *, Cue> const (&names)[N])
	{
		if (cue)
			for (auto const & [name, candidate] : names)
				if (*cue == candidate)
					return name;
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
		.music = ParseCue(audio, "music", filepath, MusicNames),
		.ambience = ParseCue(audio, "ambience", filepath, AmbienceNames),
	};
}

export nlohmann::ordered_json SerializeSceneAudio(SceneAudioData const & data)
{
	return { { "music", SerializeCue(data.music, MusicNames) },
		{ "ambience", SerializeCue(data.ambience, AmbienceNames) } };
}

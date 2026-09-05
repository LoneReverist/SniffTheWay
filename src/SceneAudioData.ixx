module;

#include <optional>
#include <filesystem>

export module SceneAudioData;

import AudioSystem;
import GameAudio;
import SniffTheWayConstants;

export struct SceneAudioData
{
	std::optional<SniffTheWay::MusicCue> music;
	std::optional<SniffTheWay::AmbienceCue> ambience;
};

export void ApplySceneAudio(AudioSystem & audio, SceneAudioData const & data,
	std::filesystem::path const & resources_path)
{
	if (data.music)
		audio.PlayMusic(SniffTheWay::MusicTrack(*data.music, resources_path));
	else
		audio.StopMusic();
	if (data.ambience)
		audio.PlayAmbience(SniffTheWay::AmbienceTracks(*data.ambience, resources_path));
	else
		audio.StopAmbience();
}

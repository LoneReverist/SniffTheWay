module;

#include <optional>

export module SceneAudioData;

import AudioSystem;

export struct SceneAudioData
{
	std::optional<MusicCue> music;
	std::optional<AmbienceCue> ambience;
};

export void ApplySceneAudio(AudioSystem & audio, SceneAudioData const & data)
{
	if (data.music)
		audio.PlayMusic(*data.music);
	else
		audio.StopMusic();
	if (data.ambience)
		audio.PlayAmbience(*data.ambience);
	else
		audio.StopAmbience();
}

module;

#include <algorithm>
#include <array>
#include <filesystem>
#include <memory>
#include <vector>

#include <glog/logging.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

// The Windows multimedia headers define PlaySound as a PlaySoundA/PlaySoundW macro.
// Keep the AudioSystem API platform-neutral and strongly typed instead.
#ifdef PlaySound
#undef PlaySound
#endif

module AudioSystem;

import PlatformUtils;

namespace
{
	constexpr std::size_t AmbienceLayerCount = 2;

	struct TrackConfig
	{
		std::filesystem::path path;
		float volume = 1.0f;
	};

	std::filesystem::path GetResourcesPath()
	{
#ifdef SNIFF_THE_WAY_DEV_RESOURCES_PATH
		return SNIFF_THE_WAY_DEV_RESOURCES_PATH;
#else
		return PlatformUtils::GetExecutableDir() / "resources";
#endif
	}

	TrackConfig MusicTrack(MusicCue cue)
	{
		switch (cue)
		{
		case MusicCue::Title:
			return { GetResourcesPath() / "music" / "Sunlight_on_the_Forest_Floor.mp3", 0.45f };
		case MusicCue::Picnic:
			return { GetResourcesPath() / "music" / "The_Afternoon_Meadow.mp3", 0.45f };
		case MusicCue::EarlyForest:
			return { GetResourcesPath() / "music" / "Through_the_Sun_Dappled_Thicket.mp3", 0.45f };
		case MusicCue::Creek:
			return { GetResourcesPath() / "music" / "The_Crossing_at_Dawn.mp3", 0.45f };
		case MusicCue::MiddleForest:
			return { GetResourcesPath() / "music" / "Stepping_Stones_at_Dawn.mp3", 0.45f };
		case MusicCue::Night:
			return { GetResourcesPath() / "music" / "The_Quiet_Between_Pines.mp3", 0.45f };
		case MusicCue::LateForest:
			return { GetResourcesPath() / "music" / "Noon_in_the_Hidden_Clearing.mp3", 0.45f };
		case MusicCue::Home:
			return { GetResourcesPath() / "music" / "The_Hearth_s_Last_Glow.mp3", 0.45f };
		}

		return {};
	}

	TrackConfig SoundTrack(SoundCue cue)
	{
		switch (cue)
		{
		case SoundCue::ShortChime:
			return { GetResourcesPath() / "sfx" / "short_chime.wav", 1.0f };
		}

		return {};
	}

	std::vector<TrackConfig> AmbienceTracks(AmbienceCue cue)
	{
		switch (cue)
		{
		case AmbienceCue::EarlyForest:
			return {
				TrackConfig{ GetResourcesPath() / "music" / "Firefly_audio_clip_birds_chirping_softly_#1.wav", 0.22f },
				TrackConfig{ GetResourcesPath() / "music" / "Firefly_audio_clip_leaves_rustling_in_the_wind_softly_#4.wav", 0.45f },
			};
		case AmbienceCue::Creek:
			return {
				TrackConfig{ GetResourcesPath() / "music" / "Firefly_audio_water_trickling,_small_creek_variation1.wav", 0.35f },
			};
		case AmbienceCue::MiddleForest:
			return {
				TrackConfig{ GetResourcesPath() / "music" / "Firefly_audio_clip_leaves_rustling_in_the_wind_softly_#4.wav", 0.45f },
			};
		case AmbienceCue::LateForest:
			return {
				TrackConfig{ GetResourcesPath() / "music" / "Firefly_audio_clip_birds_chirping_softly_#1.wav", 0.22f },
				TrackConfig{ GetResourcesPath() / "music" / "Firefly_audio_clip_leaves_rustling_in_the_wind_softly_#4.wav", 0.45f },
			};
		}

		return {};
	}
}

class AudioSystem::Impl
{
public:
	Impl();
	~Impl();

	bool IsAvailable() const { return m_engine_initialized; }

	bool PlayMusic(MusicCue cue);
	void StopMusic();
	bool PlayAmbience(AmbienceCue cue);
	void StopAmbience();
	bool PlaySound(SoundCue cue);

	void SetTransitionVolume(float volume_factor);

	void SetMasterVolume(float volume);
	void SetMusicVolume(float volume);
	void SetSoundEffectsVolume(float volume);

	float GetMasterVolume() const { return m_master_volume; }
	float GetMusicVolume() const { return m_music_category_volume; }
	float GetSoundEffectsVolume() const { return m_sound_effects_volume; }

private:
	void apply_volumes();
	ma_engine m_engine{};
	ma_sound m_music{};
	ma_sound m_sound_effect{};
	bool m_engine_initialized = false;
	bool m_music_initialized = false;
	bool m_sound_effect_initialized = false;
	bool m_has_current_sound_cue = false;
	SoundCue m_current_sound_cue = SoundCue::ShortChime;
	float m_sound_effect_volume = 1.0f;
	bool m_has_current_cue = false;
	MusicCue m_current_cue = MusicCue::Title;
	float m_music_volume = 1.0f;
	std::array<ma_sound, AmbienceLayerCount> m_ambience{};
	std::array<float, AmbienceLayerCount> m_ambience_volumes{};
	std::size_t m_ambience_initialized_count = 0;
	bool m_has_current_ambience_cue = false;
	AmbienceCue m_current_ambience_cue = AmbienceCue::EarlyForest;
	float m_transition_volume = 1.0f;
	float m_master_volume = 1.0f;
	float m_music_category_volume = 1.0f;
	float m_sound_effects_volume = 1.0f;
};

AudioSystem::Impl::Impl()
{
	const ma_result result = ma_engine_init(nullptr, &m_engine);
	if (result != MA_SUCCESS)
	{
		LOG(WARNING) << "AudioSystem: Failed to initialize audio: " << ma_result_description(result);
		return;
	}

	m_engine_initialized = true;
}

AudioSystem::Impl::~Impl()
{
	if (m_sound_effect_initialized)
	{
		ma_sound_stop(&m_sound_effect);
		ma_sound_uninit(&m_sound_effect);
	}
	StopAmbience();
	StopMusic();
	if (m_engine_initialized)
		ma_engine_uninit(&m_engine);
}

bool AudioSystem::Impl::PlayMusic(MusicCue cue)
{
	if (!m_engine_initialized)
		return false;

	if (m_music_initialized && m_has_current_cue && m_current_cue == cue)
	{
		if (!ma_sound_is_playing(&m_music))
			ma_sound_start(&m_music);
		return true;
	}

	StopMusic();

	const TrackConfig music_track = MusicTrack(cue);
	ma_result result;
#if defined(_WIN32)
	result = ma_sound_init_from_file_w(
		&m_engine,
		music_track.path.c_str(),
		MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
		nullptr,
		nullptr,
		&m_music);
#else
	result = ma_sound_init_from_file(
		&m_engine,
		music_track.path.string().c_str(),
		MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
		nullptr,
		nullptr,
		&m_music);
#endif
	if (result != MA_SUCCESS)
	{
		LOG(WARNING) << "AudioSystem: Failed to load music '" << music_track.path.string()
			<< "': " << ma_result_description(result);
		return false;
	}

	m_music_initialized = true;
	m_has_current_cue = true;
	m_current_cue = cue;
	ma_sound_set_looping(&m_music, MA_TRUE);
	m_music_volume = music_track.volume;
	apply_volumes();

	result = ma_sound_start(&m_music);
	if (result != MA_SUCCESS)
	{
		LOG(WARNING) << "AudioSystem: Failed to start music '" << music_track.path.string()
			<< "': " << ma_result_description(result);
		StopMusic();
		return false;
	}

	LOG(INFO) << "AudioSystem: Playing music '" << music_track.path.string() << "'.";
	return true;
}

void AudioSystem::Impl::StopMusic()
{
	if (!m_music_initialized)
		return;

	ma_sound_stop(&m_music);
	ma_sound_uninit(&m_music);
	m_music = {};
	m_music_initialized = false;
	m_has_current_cue = false;
}

bool AudioSystem::Impl::PlayAmbience(AmbienceCue cue)
{
	if (!m_engine_initialized)
		return false;

	if (m_has_current_ambience_cue && m_current_ambience_cue == cue)
	{
		for (std::size_t i = 0; i < m_ambience_initialized_count; ++i)
		{
			if (!ma_sound_is_playing(&m_ambience[i]))
				ma_sound_start(&m_ambience[i]);
		}
		return true;
	}

	StopAmbience();

	const auto ambience_tracks = AmbienceTracks(cue);
	for (std::size_t i = 0; i < ambience_tracks.size(); ++i)
	{
		ma_result result;
#if defined(_WIN32)
		result = ma_sound_init_from_file_w(
			&m_engine,
			ambience_tracks[i].path.c_str(),
			MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
			nullptr,
			nullptr,
			&m_ambience[i]);
#else
		result = ma_sound_init_from_file(
			&m_engine,
			ambience_tracks[i].path.string().c_str(),
			MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
			nullptr,
			nullptr,
			&m_ambience[i]);
#endif
		if (result != MA_SUCCESS)
		{
			LOG(WARNING) << "AudioSystem: Failed to load ambience '" << ambience_tracks[i].path.string()
				<< "': " << ma_result_description(result);
			StopAmbience();
			return false;
		}

		++m_ambience_initialized_count;
		ma_sound_set_looping(&m_ambience[i], MA_TRUE);
		m_ambience_volumes[i] = ambience_tracks[i].volume;
		apply_volumes();
	}

	const ma_uint64 start_time = ma_engine_get_time_in_pcm_frames(&m_engine)
		+ ma_engine_get_sample_rate(&m_engine) / 20;
	for (std::size_t i = 0; i < m_ambience_initialized_count; ++i)
	{
		ma_sound_set_start_time_in_pcm_frames(&m_ambience[i], start_time);
		const ma_result result = ma_sound_start(&m_ambience[i]);
		if (result != MA_SUCCESS)
		{
			LOG(WARNING) << "AudioSystem: Failed to start ambience '" << ambience_tracks[i].path.string()
				<< "': " << ma_result_description(result);
			StopAmbience();
			return false;
		}
	}

	m_has_current_ambience_cue = true;
	m_current_ambience_cue = cue;
	LOG(INFO) << "AudioSystem: Playing " << m_ambience_initialized_count << " ambience layers.";
	return true;
}

void AudioSystem::Impl::StopAmbience()
{
	for (std::size_t i = 0; i < m_ambience_initialized_count; ++i)
	{
		ma_sound_stop(&m_ambience[i]);
		ma_sound_uninit(&m_ambience[i]);
		m_ambience[i] = {};
	}

	m_ambience_initialized_count = 0;
	m_has_current_ambience_cue = false;
}

void AudioSystem::Impl::SetTransitionVolume(float volume_factor)
{
	m_transition_volume = std::clamp(volume_factor, 0.0f, 1.0f);
	apply_volumes();
}

void AudioSystem::Impl::SetMasterVolume(float volume)
{
	m_master_volume = std::clamp(volume, 0.0f, 1.0f);
	apply_volumes();
}

void AudioSystem::Impl::SetMusicVolume(float volume)
{
	m_music_category_volume = std::clamp(volume, 0.0f, 1.0f);
	apply_volumes();
}

void AudioSystem::Impl::SetSoundEffectsVolume(float volume)
{
	m_sound_effects_volume = std::clamp(volume, 0.0f, 1.0f);
	apply_volumes();
}

bool AudioSystem::Impl::PlaySound(SoundCue cue)
{
	if (!m_engine_initialized)
		return false;

	if (m_sound_effect_initialized && (!m_has_current_sound_cue || m_current_sound_cue != cue))
	{
		ma_sound_stop(&m_sound_effect);
		ma_sound_uninit(&m_sound_effect);
		m_sound_effect = {};
		m_sound_effect_initialized = false;
		m_has_current_sound_cue = false;
	}

	if (!m_sound_effect_initialized)
	{
		const TrackConfig sound_track = SoundTrack(cue);
		ma_result init_result;
#if defined(_WIN32)
		init_result = ma_sound_init_from_file_w(
			&m_engine,
			sound_track.path.c_str(),
			MA_SOUND_FLAG_NO_SPATIALIZATION,
			nullptr,
			nullptr,
			&m_sound_effect);
#else
		init_result = ma_sound_init_from_file(
			&m_engine,
			sound_track.path.string().c_str(),
			MA_SOUND_FLAG_NO_SPATIALIZATION,
			nullptr,
			nullptr,
			&m_sound_effect);
#endif
		if (init_result != MA_SUCCESS)
		{
			LOG(WARNING) << "AudioSystem: Failed to load sound effect '" << sound_track.path.string()
				<< "': " << ma_result_description(init_result);
			return false;
		}
		m_sound_effect_initialized = true;
		m_has_current_sound_cue = true;
		m_current_sound_cue = cue;
		m_sound_effect_volume = sound_track.volume;
	}

	ma_sound_stop(&m_sound_effect);
	ma_sound_seek_to_pcm_frame(&m_sound_effect, 0);
	apply_volumes();
	const ma_result result = ma_sound_start(&m_sound_effect);
	if (result != MA_SUCCESS)
	{
		LOG(WARNING) << "AudioSystem: Failed to play sound effect: "
			<< ma_result_description(result);
		return false;
	}
	return true;
}

void AudioSystem::Impl::apply_volumes()
{
	if (m_music_initialized)
		ma_sound_set_volume(&m_music,
			m_music_volume * m_music_category_volume * m_master_volume * m_transition_volume);

	for (std::size_t i = 0; i < m_ambience_initialized_count; ++i)
		ma_sound_set_volume(&m_ambience[i],
			m_ambience_volumes[i] * m_music_category_volume * m_master_volume * m_transition_volume);

	if (m_sound_effect_initialized)
		ma_sound_set_volume(&m_sound_effect,
			m_sound_effect_volume * m_sound_effects_volume * m_master_volume);
}

AudioSystem::AudioSystem()
	: m_impl{ std::make_unique<Impl>() }
{}

AudioSystem::~AudioSystem() = default;

bool AudioSystem::IsAvailable() const
{
	return m_impl->IsAvailable();
}

bool AudioSystem::PlayMusic(MusicCue cue)
{
	return m_impl->PlayMusic(cue);
}

void AudioSystem::StopMusic()
{
	m_impl->StopMusic();
}

bool AudioSystem::PlayAmbience(AmbienceCue cue)
{
	return m_impl->PlayAmbience(cue);
}

void AudioSystem::StopAmbience()
{
	m_impl->StopAmbience();
}

bool AudioSystem::PlaySound(SoundCue cue)
{
	return m_impl->PlaySound(cue);
}

void AudioSystem::SetTransitionVolume(float volume_factor)
{
	m_impl->SetTransitionVolume(volume_factor);
}

void AudioSystem::SetMasterVolume(float volume)
{
	m_impl->SetMasterVolume(volume);
}

void AudioSystem::SetMusicVolume(float volume)
{
	m_impl->SetMusicVolume(volume);
}

void AudioSystem::SetSoundEffectsVolume(float volume)
{
	m_impl->SetSoundEffectsVolume(volume);
}

float AudioSystem::GetMasterVolume() const
{
	return m_impl->GetMasterVolume();
}

float AudioSystem::GetMusicVolume() const
{
	return m_impl->GetMusicVolume();
}

float AudioSystem::GetSoundEffectsVolume() const
{
	return m_impl->GetSoundEffectsVolume();
}

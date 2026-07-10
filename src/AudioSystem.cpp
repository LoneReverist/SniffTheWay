module;

#include <array>
#include <filesystem>
#include <memory>

#include <glog/logging.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

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
		case MusicCue::EarlyForest:
			return { GetResourcesPath() / "music" / "Through_the_Sun_Dappled_Thicket.mp3", 0.45f };
		}

		return {};
	}

	std::array<TrackConfig, AmbienceLayerCount> AmbienceTracks(AmbienceCue cue)
	{
		switch (cue)
		{
		case AmbienceCue::EarlyForest:
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

private:
	ma_engine m_engine{};
	ma_sound m_music{};
	bool m_engine_initialized = false;
	bool m_music_initialized = false;
	bool m_has_current_cue = false;
	MusicCue m_current_cue = MusicCue::Title;
	std::array<ma_sound, AmbienceLayerCount> m_ambience{};
	std::size_t m_ambience_initialized_count = 0;
	bool m_has_current_ambience_cue = false;
	AmbienceCue m_current_ambience_cue = AmbienceCue::EarlyForest;
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
	ma_sound_set_volume(&m_music, music_track.volume);

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
		ma_sound_set_volume(&m_ambience[i], ambience_tracks[i].volume);
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

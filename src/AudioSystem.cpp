module;

#include <algorithm>
#include <span>
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

class AudioSystem::Impl
{
public:
	Impl();
	~Impl();

	bool IsAvailable() const { return m_engine_initialized; }

	bool PlayMusic(AudioTrack const & music_track);
	void StopMusic();
	bool PlayAmbience(std::span<AudioTrack const> ambience_tracks);
	void StopAmbience();
	bool PlaySound(AudioTrack const & sound_track);

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
	std::filesystem::path m_current_sound_path;
	float m_sound_effect_volume = 1.0f;
	std::filesystem::path m_current_music_path;
	float m_music_volume = 1.0f;
	// miniaudio sounds must keep stable addresses while the layer list grows.
	std::vector<std::unique_ptr<ma_sound>> m_ambience;
	std::vector<float> m_ambience_volumes;
	std::size_t m_ambience_initialized_count = 0;
	std::vector<AudioTrack> m_current_ambience_tracks;
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

bool AudioSystem::Impl::PlayMusic(AudioTrack const & music_track)
{
	if (!m_engine_initialized)
		return false;

	if (m_music_initialized && m_current_music_path == music_track.path)
	{
		m_music_volume = music_track.volume;
		apply_volumes();
		if (!ma_sound_is_playing(&m_music))
			ma_sound_start(&m_music);
		return true;
	}

	StopMusic();

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
	m_current_music_path = music_track.path;
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
	m_current_music_path.clear();
}

bool AudioSystem::Impl::PlayAmbience(std::span<AudioTrack const> ambience_tracks)
{
	if (!m_engine_initialized)
		return false;

	if (std::ranges::equal(m_current_ambience_tracks, ambience_tracks,
		[](AudioTrack const & a, AudioTrack const & b) { return a.path == b.path; }))
	{
		for (std::size_t i = 0; i < m_ambience_initialized_count; ++i)
		{
			m_ambience_volumes[i] = ambience_tracks[i].volume;
			if (!ma_sound_is_playing(m_ambience[i].get()))
				ma_sound_start(m_ambience[i].get());
		}
		apply_volumes();
		return true;
	}

	StopAmbience();

	m_ambience_volumes.resize(ambience_tracks.size());
	for (std::size_t i = 0; i < ambience_tracks.size(); ++i)
	{
		m_ambience.push_back(std::make_unique<ma_sound>());
		ma_result result;
#if defined(_WIN32)
		result = ma_sound_init_from_file_w(
			&m_engine,
			ambience_tracks[i].path.c_str(),
			MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
			nullptr,
			nullptr,
			m_ambience[i].get());
#else
		result = ma_sound_init_from_file(
			&m_engine,
			ambience_tracks[i].path.string().c_str(),
			MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
			nullptr,
			nullptr,
			m_ambience[i].get());
#endif
		if (result != MA_SUCCESS)
		{
			LOG(WARNING) << "AudioSystem: Failed to load ambience '" << ambience_tracks[i].path.string()
				<< "': " << ma_result_description(result);
			StopAmbience();
			return false;
		}

		++m_ambience_initialized_count;
		ma_sound_set_looping(m_ambience[i].get(), MA_TRUE);
		m_ambience_volumes[i] = ambience_tracks[i].volume;
		apply_volumes();
	}

	const ma_uint64 start_time = ma_engine_get_time_in_pcm_frames(&m_engine)
		+ ma_engine_get_sample_rate(&m_engine) / 20;
	for (std::size_t i = 0; i < m_ambience_initialized_count; ++i)
	{
		ma_sound_set_start_time_in_pcm_frames(m_ambience[i].get(), start_time);
		const ma_result result = ma_sound_start(m_ambience[i].get());
		if (result != MA_SUCCESS)
		{
			LOG(WARNING) << "AudioSystem: Failed to start ambience '" << ambience_tracks[i].path.string()
				<< "': " << ma_result_description(result);
			StopAmbience();
			return false;
		}
	}

	m_current_ambience_tracks.assign(ambience_tracks.begin(), ambience_tracks.end());
	LOG(INFO) << "AudioSystem: Playing " << m_ambience_initialized_count << " ambience layers.";
	return true;
}

void AudioSystem::Impl::StopAmbience()
{
	for (std::size_t i = 0; i < m_ambience_initialized_count; ++i)
	{
		ma_sound_stop(m_ambience[i].get());
		ma_sound_uninit(m_ambience[i].get());
		m_ambience[i] = {};
	}

	m_ambience_initialized_count = 0;
	m_ambience.clear();
	m_ambience_volumes.clear();
	m_current_ambience_tracks.clear();
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

bool AudioSystem::Impl::PlaySound(AudioTrack const & sound_track)
{
	if (!m_engine_initialized)
		return false;

	if (m_sound_effect_initialized && m_current_sound_path != sound_track.path)
	{
		ma_sound_stop(&m_sound_effect);
		ma_sound_uninit(&m_sound_effect);
		m_sound_effect = {};
		m_sound_effect_initialized = false;
		m_current_sound_path.clear();
	}

	if (!m_sound_effect_initialized)
	{
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
		m_current_sound_path = sound_track.path;
	}

	m_sound_effect_volume = sound_track.volume;
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
		ma_sound_set_volume(m_ambience[i].get(),
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

bool AudioSystem::PlayMusic(AudioTrack const & music_track)
{
	return m_impl->PlayMusic(music_track);
}

void AudioSystem::StopMusic()
{
	m_impl->StopMusic();
}

bool AudioSystem::PlayAmbience(std::span<AudioTrack const> ambience_tracks)
{
	return m_impl->PlayAmbience(ambience_tracks);
}

void AudioSystem::StopAmbience()
{
	m_impl->StopAmbience();
}

bool AudioSystem::PlaySound(AudioTrack const & sound_track)
{
	return m_impl->PlaySound(sound_track);
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

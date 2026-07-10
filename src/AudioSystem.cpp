module;

#include <filesystem>
#include <memory>

#include <glog/logging.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

module AudioSystem;

import PlatformUtils;

namespace
{
	std::filesystem::path GetResourcesPath()
	{
#ifdef SNIFF_THE_WAY_DEV_RESOURCES_PATH
		return SNIFF_THE_WAY_DEV_RESOURCES_PATH;
#else
		return PlatformUtils::GetExecutableDir() / "resources";
#endif
	}

	std::filesystem::path MusicPath(MusicCue cue)
	{
		switch (cue)
		{
		case MusicCue::Title:
			return GetResourcesPath() / "music" / "Sunlight_on_the_Forest_Floor.mp3";
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

private:
	ma_engine m_engine{};
	ma_sound m_music{};
	bool m_engine_initialized = false;
	bool m_music_initialized = false;
	bool m_has_current_cue = false;
	MusicCue m_current_cue = MusicCue::Title;
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

	const std::filesystem::path music_path = MusicPath(cue);
	ma_result result;
#if defined(_WIN32)
	result = ma_sound_init_from_file_w(
		&m_engine,
		music_path.c_str(),
		MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
		nullptr,
		nullptr,
		&m_music);
#else
	result = ma_sound_init_from_file(
		&m_engine,
		music_path.string().c_str(),
		MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION,
		nullptr,
		nullptr,
		&m_music);
#endif
	if (result != MA_SUCCESS)
	{
		LOG(WARNING) << "AudioSystem: Failed to load music '" << music_path.string()
			<< "': " << ma_result_description(result);
		return false;
	}

	m_music_initialized = true;
	m_has_current_cue = true;
	m_current_cue = cue;
	ma_sound_set_looping(&m_music, MA_TRUE);
	ma_sound_set_volume(&m_music, 0.45f);

	result = ma_sound_start(&m_music);
	if (result != MA_SUCCESS)
	{
		LOG(WARNING) << "AudioSystem: Failed to start music '" << music_path.string()
			<< "': " << ma_result_description(result);
		StopMusic();
		return false;
	}

	LOG(INFO) << "AudioSystem: Playing music '" << music_path.string() << "'.";
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

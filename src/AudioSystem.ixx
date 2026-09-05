module;

#include <memory>
#include <filesystem>
#include <span>

export module AudioSystem;

// Callers supply full paths; AudioSystem does not resolve resource directories.
export struct AudioTrack
{
	std::filesystem::path path;
	float volume = 1.0f;
};

export class AudioSystem
{
public:
	AudioSystem();
	~AudioSystem();

	AudioSystem(AudioSystem const &) = delete;
	AudioSystem & operator=(AudioSystem const &) = delete;
	AudioSystem(AudioSystem &&) = delete;
	AudioSystem & operator=(AudioSystem &&) = delete;

	bool IsAvailable() const;
	
	// Music and ambience loop. Reusing the same paths updates volume without restarting.
	bool PlayMusic(AudioTrack const & music_track);
	void StopMusic();
	// An empty list stops ambience. Tracks are copied; the input need not outlive this call.
	bool PlayAmbience(std::span<AudioTrack const> ambience_tracks);
	void StopAmbience();
	// Sound effects play once, restarting from the beginning on each request.
	bool PlaySound(AudioTrack const & sound_track);

	void SetTransitionVolume(float volume_factor);

	void SetMasterVolume(float volume);
	void SetMusicVolume(float volume);
	void SetSoundEffectsVolume(float volume);

	float GetMasterVolume() const;
	float GetMusicVolume() const;
	float GetSoundEffectsVolume() const;

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};

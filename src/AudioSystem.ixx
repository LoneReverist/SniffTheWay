module;

#include <memory>

export module AudioSystem;

export enum class MusicCue
{
	Title,
	Picnic,
	EarlyForest,
	Creek,
	MiddleForest,
	Night,
	LateForest,
	Home,
};

export enum class AmbienceCue
{
	EarlyForest,
	Creek,
	MiddleForest,
	LateForest,
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
	bool PlayMusic(MusicCue cue);
	void StopMusic();
	bool PlayAmbience(AmbienceCue cue);
	void StopAmbience();
	void SetTransitionVolume(float volume_factor);
	void SetMasterVolume(float volume);
	void SetMusicVolume(float volume);
	void SetSoundEffectsVolume(float volume);
	bool PlayVolumeChangeChime();
	float GetMasterVolume() const;
	float GetMusicVolume() const;
	float GetSoundEffectsVolume() const;

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};

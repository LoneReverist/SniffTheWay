module;

#include <memory>

export module AudioSystem;

export enum class MusicCue
{
	Title,
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

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};

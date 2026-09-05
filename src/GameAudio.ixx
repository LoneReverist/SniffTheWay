module;

#include <filesystem>
#include <vector>

export module GameAudio;

import AudioSystem;
import SniffTheWayConstants;

export namespace SniffTheWay
{
	AudioTrack MusicTrack(MusicCue cue, std::filesystem::path const & resources_path)
	{
		switch (cue)
		{
		case MusicCue::Title:
			return { resources_path / "music" / "Sunlight_on_the_Forest_Floor.mp3", 0.45f };
		case MusicCue::Picnic:
			return { resources_path / "music" / "The_Afternoon_Meadow.mp3", 0.45f };
		case MusicCue::EarlyForest:
			return { resources_path / "music" / "Through_the_Sun_Dappled_Thicket.mp3", 0.45f };
		case MusicCue::Creek:
			return { resources_path / "music" / "The_Crossing_at_Dawn.mp3", 0.45f };
		case MusicCue::MiddleForest:
			return { resources_path / "music" / "Stepping_Stones_at_Dawn.mp3", 0.45f };
		case MusicCue::Night:
			return { resources_path / "music" / "The_Quiet_Between_Pines.mp3", 0.45f };
		case MusicCue::LateForest:
			return { resources_path / "music" / "Noon_in_the_Hidden_Clearing.mp3", 0.45f };
		case MusicCue::Home:
			return { resources_path / "music" / "The_Hearth_s_Last_Glow.mp3", 0.45f };
		}

		return {};
	}

	AudioTrack SoundTrack(SoundCue cue, std::filesystem::path const & resources_path)
	{
		switch (cue)
		{
		case SoundCue::ShortChime:
			return { resources_path / "sfx" / "short_chime.wav", 1.0f };
		case SoundCue::GustOfWind:
			return { resources_path / "sfx" / "gust_of_wind.wav", 1.0f };
		}

		return {};
	}

	std::vector<AudioTrack> AmbienceTracks(AmbienceCue cue, std::filesystem::path const & resources_path)
	{
		switch (cue)
		{
		case AmbienceCue::EarlyForest:
			return {
				AudioTrack{ resources_path / "music" / "Firefly_audio_clip_birds_chirping_softly_#1.wav", 0.22f },
				AudioTrack{ resources_path / "music" / "Firefly_audio_clip_leaves_rustling_in_the_wind_softly_#4.wav", 0.45f },
			};
		case AmbienceCue::DistantCreek:
			return {
				AudioTrack{ resources_path / "music" / "Firefly_audio_clip_birds_chirping_softly_#1.wav", 0.22f },
				AudioTrack{ resources_path / "music" / "Firefly_audio_clip_leaves_rustling_in_the_wind_softly_#4.wav", 0.45f },
				AudioTrack{ resources_path / "music" / "Firefly_audio_water_trickling,_small_creek_variation1.wav", 0.2f },
			};
		case AmbienceCue::Creek:
			return {
				AudioTrack{ resources_path / "music" / "Firefly_audio_water_trickling,_small_creek_variation1.wav", 0.35f },
			};
		case AmbienceCue::MiddleForest:
			return {
				AudioTrack{ resources_path / "music" / "Firefly_audio_clip_leaves_rustling_in_the_wind_softly_#4.wav", 0.45f },
			};
		case AmbienceCue::Night:
			return {
				AudioTrack{ resources_path / "music" / "crickets_intermittent.wav", 0.225f },
			};
		case AmbienceCue::LateForest:
			return {
				AudioTrack{ resources_path / "music" / "Firefly_audio_clip_birds_chirping_softly_#1.wav", 0.22f },
				AudioTrack{ resources_path / "music" / "Firefly_audio_clip_leaves_rustling_in_the_wind_softly_#4.wav", 0.45f },
			};
		}

		return {};
	}
}

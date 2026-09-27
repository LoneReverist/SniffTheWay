// Playthrough.ixx

module;

#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <stdexcept>
#include <glog/logging.h>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <utility>

export module Playthrough;

import PlaythroughState;
import SniffTheWayConstants;

export struct SquirrelProgress
{
	std::size_t found = 0;
	std::size_t total = 0;
	bool AllFound() const { return total > 0 && found == total; }
};

export class Playthrough
{
public:
	Playthrough() = default;
	explicit Playthrough(PlaythroughState state)
		: m_state{ std::move(state) } {}

	void LoadSquirrelCatalog(std::filesystem::path const & resources, bool refresh = false);
	SquirrelProgress GetSquirrelProgress() const
	{
		SquirrelProgress progress{ 0, m_squirrel_catalog.size() };
		for (auto const & id : m_squirrel_catalog)
			if (HasTrigger(id)) ++progress.found;
		return progress;
	}
	static std::string SquirrelKey(SniffTheWay::SceneId scene, std::string_view id)
	{ return std::string{ SniffTheWay::ToString(scene) } + "/squirrel/" + std::string{ id }; }
	bool TryFindSquirrel(SniffTheWay::SceneId scene, std::string_view id)
	{ return !id.empty() && SetTrigger(SquirrelKey(scene, id)); }
	bool HasFoundSquirrel(SniffTheWay::SceneId scene, std::string_view id) const
	{ return HasTrigger(SquirrelKey(scene, id)); }
	bool TryTrigger(SniffTheWay::SceneId scene_id, std::string_view local_id);
	bool SetTrigger(std::string_view id) { return !id.empty() && m_state.triggered_ids.insert(std::string{ id }).second; }
	bool HasTrigger(std::string_view id) const { return m_state.triggered_ids.contains(std::string{ id }); }
	bool MeetsTriggerConditions(std::string_view required, std::string_view forbidden) const
	{
		return (required.empty() || HasTrigger(required)) && (forbidden.empty() || !HasTrigger(forbidden));
	}

	PlaythroughState const & GetState() const { return m_state; }
	void Reset() { m_state.triggered_ids.clear(); }

private:
	PlaythroughState m_state;
	std::unordered_set<std::string> m_squirrel_catalog;
	bool m_catalog_loaded = false;
};

bool Playthrough::TryTrigger(SniffTheWay::SceneId scene_id, std::string_view local_id)
{
	std::string id{ SniffTheWay::ToString(scene_id) };
	id.push_back('/');
	id.append(local_id);
	return m_state.triggered_ids.insert(std::move(id)).second;
}

void Playthrough::LoadSquirrelCatalog(std::filesystem::path const & resources, bool refresh)
{
	if (m_catalog_loaded && !refresh) return;
	std::unordered_set<std::string> catalog;
	try
	{
		// Use the registered scene enum and filename mapping, without loading textures.
		for (int value = 0; value <= static_cast<int>(SniffTheWay::SceneId::Home); ++value)
		{
			auto scene = static_cast<SniffTheWay::SceneId>(value);
			if (!SniffTheWay::IsGameplayScene(scene)) continue;
			auto path = resources / "gameplay" / (std::string{ SniffTheWay::ToString(scene) } + ".json");
			std::ifstream file{ path };
			if (!file) throw std::runtime_error("Cannot read " + path.string());
			auto data = nlohmann::json::parse(file);
			for (auto const & squirrel : data.value("squirrels", nlohmann::json::array()))
			{
				auto id = squirrel.at("id").get<std::string>();
				if (id.empty() || !catalog.insert(SquirrelKey(scene, id)).second)
					throw std::runtime_error("Invalid or duplicate squirrel ID in " + path.string());
			}
		}
		m_squirrel_catalog = std::move(catalog);
		m_catalog_loaded = true;
	}
	catch (std::exception const & error)
	{
		LOG(ERROR) << "Squirrel catalog: " << error.what();
	}
}

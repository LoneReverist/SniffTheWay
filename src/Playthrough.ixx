// Playthrough.ixx

module;

#include <string>
#include <string_view>
#include <utility>

export module Playthrough;

import PlaythroughState;
import SniffTheWayConstants;

export class Playthrough
{
public:
	Playthrough() = default;
	explicit Playthrough(PlaythroughState state)
		: m_state{ std::move(state) } {}

	bool TryTrigger(SniffTheWay::SceneId scene_id, std::string_view local_id);
	bool SetTrigger(std::string_view id) { return !id.empty() && m_state.triggered_ids.insert(std::string{ id }).second; }
	bool HasTrigger(std::string_view id) const { return m_state.triggered_ids.contains(std::string{ id }); }
	bool MeetsTriggerConditions(std::string_view required, std::string_view forbidden) const
	{
		return (required.empty() || HasTrigger(required)) && (forbidden.empty() || !HasTrigger(forbidden));
	}

	PlaythroughState const & GetState() const { return m_state; }

private:
	PlaythroughState m_state;
};

bool Playthrough::TryTrigger(SniffTheWay::SceneId scene_id, std::string_view local_id)
{
	std::string id{ SniffTheWay::ToString(scene_id) };
	id.push_back('/');
	id.append(local_id);
	return m_state.triggered_ids.insert(std::move(id)).second;
}

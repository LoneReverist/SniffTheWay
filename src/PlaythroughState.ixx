// PlaythroughState.ixx

module;

#include <string>
#include <unordered_set>

export module PlaythroughState;

export struct PlaythroughState
{
	std::unordered_set<std::string> triggered_ids;
};

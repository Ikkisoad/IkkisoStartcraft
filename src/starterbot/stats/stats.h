#pragma once

#include <BWAPI.h>

namespace Stats
{
	void LogGameStats(const std::string opponent, const std::string& map, const std::string& strategy, bool win);
	void GetStatsLine();
	std::string readStrategy(const std::string& filename, const std::string& opponent, const std::string& mapHash);
	void updateWinRateFile(const std::string& filename, const std::string& opponent, const std::string& mapHash, const std::string& strategy, bool won);
}
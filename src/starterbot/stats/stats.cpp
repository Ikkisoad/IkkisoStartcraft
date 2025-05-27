#include "Stats.h"
#include <BWAPI.h>
#include <fstream>

struct StrategyStats {
    std::string strategy;
    int games;
    int wins;
};

std::string statsFileName = "strats-v1";

std::string Stats::readStrategy(const std::string& opponent, const std::string& opponentRace, const std::string& mapHash) {
    auto fileName = (std::ostringstream() << "../src/starterbot/stats/data/" << statsFileName << ".csv").str();
    std::ifstream file(fileName);
    std::string line;
    StrategyStats bestStrategy = { "Default", 0, 0 };
    double bestWinRate = -1.0;
    StrategyStats bestStrategyAgainstRace = { "Default", 0, 0 };
    double bestWinRateAgainstRace = -1.0;
    bool opponentFound = false;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string opp, map, strat, oppRace;
        int games, wins;

        std::getline(ss, opp, ',');
        std::getline(ss, oppRace, ',');
        std::getline(ss, map, ',');
        std::getline(ss, strat, ',');
        ss >> games;
        ss.ignore();
        ss >> wins;

        if (map == mapHash && games > 0) {
            if (opp == opponent) {
                opponentFound = true;
                double winRate = (double)wins / games;
                if (winRate > bestWinRate) {
                    bestWinRate = winRate;
                    bestStrategy = { strat, games, wins };
                }
            }
            else if (oppRace == opponentRace) {
                double winRate = (double)wins / games;
                if (winRate > bestWinRateAgainstRace) {
                    bestWinRateAgainstRace = winRate;
                    bestStrategyAgainstRace = { strat, games, wins };
                }
            }
        }
    }
    if (opponentFound) {
        return bestStrategy.strategy;
	}
    return bestStrategyAgainstRace.strategy;
}

void Stats::updateWinRateFile(const std::string& opponent, const std::string& opponentRace, const std::string& mapHash, const std::string& strategy, bool won) {
    auto fileName = (std::ostringstream() << "../src/starterbot/stats/data/" << statsFileName << ".csv").str();
    std::ifstream inFile(fileName);
    std::vector<std::string> lines;
    std::string line;
    bool updated = false;

    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string opp, oppRace, map, strat;
        int games, wins;

        std::getline(ss, opp, ',');
        std::getline(ss, oppRace, ',');
        std::getline(ss, map, ',');
        std::getline(ss, strat, ',');
        ss >> games;
        ss.ignore();
        ss >> wins;

        if (opp == opponent && oppRace == opponentRace && map == mapHash && strat == strategy) {
            games += 1;
            if (won) wins += 1;

            std::ostringstream updatedLine;
            updatedLine << opp << "," << oppRace << "," << map << "," << strat << "," << games << "," << wins;
            lines.push_back(updatedLine.str());
            updated = true;
        }
        else {
            lines.push_back(line);
        }
    }
    inFile.close();

    if (!updated) {
        std::ostringstream newLine;
        newLine << opponent << "," << opponentRace << "," << mapHash << "," << strategy << ",1," << (won ? 1 : 0);
        lines.push_back(newLine.str());
    }

    std::ofstream outFile(fileName, std::ios::trunc);
    for (const auto& l : lines) {
        outFile << l << "\n";
    }
}
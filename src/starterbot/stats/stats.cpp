#include "Stats.h"
#include <BWAPI.h>
#include <fstream>

struct StrategyStats {
    std::string strategy;
    int games;
    int wins;
};

//TODO Log stats to file
void Stats::LogGameStats(const std::string opponent, const std::string& map, const std::string& strategy, bool win) {
    return;
    auto fileName = (std::ostringstream() << "../src/starterbot/stats/data/" << opponent << "-" << map << ".csv").str();
    std::ofstream write(fileName, std::ios::app);
    std::ifstream file(fileName, std::ios::app);
    if (file.is_open()) {
        //file << strategy << "," << (win ? "1" : "0") << ",\n";
        std::string line;
        int games = 0;
        std::string bestStrategy;
        int bestWinrate = 0;

        do {
            std::getline(file, line, ',');

            if (line == "\n") break;
            if (line.find('\n')) {
                size_t start_pos = line.find("\n");
                if (start_pos != std::string::npos) {
                    line.replace(start_pos, 1, "");
                }
            }
            if (strategy == line) {
                std::getline(file, line, ',');
                const int games = std::stoi(line);
				line = std::to_string(games + 1);
                std::getline(file, line, ',');
                const int wins = std::stoi(line);
                if (win) line = std::to_string(wins + 1);
            }
        } while (!line.empty());

        file.close();
    }
}

//void Stats::ChooseStrategy(const std::string opponent, const std::string& map) {
//
//}
//void Stats::ReadGameStats(const std::string opponent, const std::string& map) {
//    auto fileName = (std::ostringstream() << "../src/starterbot/stats/data/" << opponent << ".csv").str();
//    //std::ofstream file(fileName, std::ios::app);
//    std::ifstream file;
//    file.open(fileName);
//    if (file.is_open()) {
//        std::string line;
//        std::getline(file, line);
//        file.close();
//    }
//}
// 
// 
//std::ifstream old_file("plik1.txt~");
//std::ofstream new_file("plik1.txt");
//
//for (std::string contents_of_file; std::getline(old_file, contents_of_file); ) {
//    std::string::size_type position = contents_of_file.find("Zuzia");
//    if (position != std::string::npos)
//        contents_of_file = contents_of_file.replace(position, 5, "car");
//    new_file << contents_of_file << '\n';
//}
void Stats::GetStatsLine() {
    return;
    std::string oponentName = "";
    for (auto player : BWAPI::Broodwar->getPlayers()) {
        if (player != BWAPI::Broodwar->self() && player != BWAPI::Broodwar->neutral()) {
            oponentName = player->getName();
        }
    }//BWAPI::Broodwar->mapHash()
    std::string map = BWAPI::Broodwar->mapHash();
    auto fileName = (std::ostringstream() << "../src/starterbot/stats/data/" << oponentName << "-" << map << ".csv").str();
    //std::ofstream file(fileName, std::ios::app);
    std::ifstream file;
    file.open(fileName);
    if (file.is_open()) {
        std::string line;
        std::string strategy;
        int winrate = 0;
        std::string bestStrategy;
        int bestWinrate = 0;
       
        do {
            std::getline(file, line, ',');

            if (line == "\n") break;
            if (line.find('\n')) {
                size_t start_pos = line.find("\n");
                if (start_pos != std::string::npos) {
                    line.replace(start_pos, 1, "");
                }
            }
            if (map == line) {
                std::getline(file, line, ',');
				strategy = line;
                std::getline(file, line, ',');
				const int games = std::stoi(line);
                std::getline(file, line, ',');
                const int wins = std::stoi(line);
                winrate = wins / games;

                if (bestStrategy.empty()) {
                    bestStrategy = strategy;
                    bestWinrate = winrate;
                }
                else if (winrate > bestWinrate) {
                    bestStrategy = strategy;
                    bestWinrate = winrate;
				}

			}



        } while (!line.empty());

        file.close();
    }
}

std::string Stats::readStrategy(const std::string& filename, const std::string& opponent, const std::string& mapHash) {
    auto fileName = (std::ostringstream() << "../src/starterbot/stats/data/" << filename << ".csv").str();
    std::ifstream file(fileName);
    std::string line;
    StrategyStats bestStrategy = { "Default", 0, 0 };
    double bestWinRate = -1.0;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string opp, map, strat;
        int games, wins;

        std::getline(ss, opp, ',');
        std::getline(ss, map, ',');
        std::getline(ss, strat, ',');
        ss >> games;
        ss.ignore();
        ss >> wins;

        if (opp == opponent && map == mapHash && games > 0) {
            double winRate = (double)wins / games;
            if (winRate > bestWinRate) {
                bestWinRate = winRate;
                bestStrategy = { strat, games, wins };
            }
        }
    }

    return bestStrategy.strategy;
}

void Stats::updateWinRateFile(const std::string& filename, const std::string& opponent, const std::string& mapHash,
    const std::string& strategy, bool won) {
    auto fileName = (std::ostringstream() << "../src/starterbot/stats/data/" << filename << ".csv").str();
    std::ifstream inFile(fileName);
    std::vector<std::string> lines;
    std::string line;
    bool updated = false;

    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string opp, map, strat;
        int games, wins;

        std::getline(ss, opp, ',');
        std::getline(ss, map, ',');
        std::getline(ss, strat, ',');
        ss >> games;
        ss.ignore();
        ss >> wins;

        if (opp == opponent && map == mapHash && strat == strategy) {
            games += 1;
            if (won) wins += 1;

            std::ostringstream updatedLine;
            updatedLine << opp << "," << map << "," << strat << "," << games << "," << wins;
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
        newLine << opponent << "," << mapHash << "," << strategy << ",1," << (won ? 1 : 0);
        lines.push_back(newLine.str());
    }

    std::ofstream outFile(fileName, std::ios::trunc);
    for (const auto& l : lines) {
        outFile << l << "\n";
    }
}
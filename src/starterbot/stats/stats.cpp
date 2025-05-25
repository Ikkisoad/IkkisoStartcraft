#include "Stats.h"
#include <BWAPI.h>
#include <fstream>

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
#pragma once
#include "GameTypes.h"

void addBuildingPlatforms(const Building& b, std::vector<Platform>& platforms);
void buildTrainingStage(std::vector<Platform>& platforms, std::vector<sf::Vector2f>& spawns);
void buildKohonaStage(std::vector<Platform>& platforms, std::vector<sf::Vector2f>& spawns);
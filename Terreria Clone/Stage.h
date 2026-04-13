#pragma once
#include "GameTypes.h"


//---------------------------------------------------
// Stage Visuals
//---------------------------------------------------
struct StageVisuals {
	std::vector<sf::RectangleShape> backgroundLayers;
	std::vector<sf::RectangleShape> midgroundLayers;
	std::vector<sf::RectangleShape> foregroundLayers;
};

//---------------------------------------------------
// Stage Building
//---------------------------------------------------
void addBuildingPlatforms(const Building& b, std::vector<Platform>& platforms);

//Build stage gameplay + visuals
void buildDojoStage(
	std::vector<Platform>& platforms, 
	std::vector<sf::Vector2f>& spawns,
	StageVisuals& visuals
);

void buildHiddenEmberStage(
	std::vector<Platform>& platforms, 
	std::vector<sf::Vector2f>& spawns,
	StageVisuals& visuals
);

//---------------------------------------------------
// Stage Rendering
//---------------------------------------------------
void drawStageBackground(sf::RenderWindow& window, const StageVisuals& visuals);
void drawStageMidground(sf::RenderWindow& window, const StageVisuals& visuals);
void drawStagePlatforms(sf::RenderWindow& window, const std::vector<Platform>& platforms);
void drawStageForeground(sf::RenderWindow& window, const StageVisuals& visuals);
#include "Stage.h"


// ============================================================
//  STAGE 
// ============================================================

void addBuildingPlatforms(const Building& b, std::vector<Platform>& platforms) {
	// Left Wall
	if (!b.openFront) {
		Platform leftWall;
		leftWall.shape.setSize({ b.wallThickness, b.size.y });
		leftWall.shape.setPosition(b.position);
		leftWall.shape.setFillColor(sf::Color::White);
		leftWall.oneWay = false;
		platforms.push_back(leftWall);
	}

	// Right Wall
	Platform rightWall;
	rightWall.shape.setSize({ b.wallThickness, b.size.y });
	rightWall.shape.setPosition({ b.position.x + b.size.x - b.wallThickness, b.position.y });
	rightWall.shape.setFillColor(sf::Color::White);
	rightWall.oneWay = false;
	platforms.push_back(rightWall);

	// Roof (one-Way)
	Platform roof;
	roof.shape.setSize({ b.size.x, b.roofThickness });
	roof.shape.setPosition({ b.position.x, b.position.y });
	roof.shape.setFillColor(sf::Color::White);
	roof.oneWay = true;
	platforms.push_back(roof);

	// Interior Ledges
	for (const auto& ledge : b.ledges) {
		Platform p;
		p.shape.setSize({ ledge.width, 16.f });
		p.shape.setPosition({ b.position.x + ledge.xOffset, b.position.y + ledge.yOffset });
		p.shape.setFillColor(sf::Color::Red);
		p.oneWay = true;
		platforms.push_back(p);
	}
}

void buildTrainingStage(std::vector<Platform>& platforms, std::vector<sf::Vector2f>& spawns) {
	platforms.clear();
	spawns.clear();

	const float groundY = 300.f;

	Platform ground;
	ground.shape.setSize({ 1400.f, 50.f });
	ground.shape.setPosition({ -700.f, groundY });
	ground.shape.setFillColor(sf::Color::White);
	ground.shape.setOutlineThickness(2.f);
	ground.shape.setOutlineColor(sf::Color::Green);
	ground.oneWay = false;
	platforms.push_back(ground);

	Platform p1;
	p1.shape.setSize({ 200.f, 16.f });
	p1.shape.setPosition({ -200.f, 220.f });
	p1.shape.setFillColor(sf::Color::White);
	p1.shape.setOutlineThickness(2.f);
	p1.shape.setOutlineColor(sf::Color::Green);
	p1.oneWay = true;
	platforms.push_back(p1);

	Platform p2;
	p2.shape.setSize({ 200.f, 16.f });
	p2.shape.setPosition({ 200.f, 180.f });
	p2.shape.setFillColor(sf::Color::White);
	p2.shape.setOutlineThickness(2.f);
	p2.shape.setOutlineColor(sf::Color::Green);
	p2.oneWay = true;
	platforms.push_back(p2);

	spawns.push_back({ 0.f, groundY - 60.f });     // player 1
	spawns.push_back({ -150.f, groundY - 60.f });  // player 2
	spawns.push_back({ 150.f, groundY - 60.f });   // player 3
	spawns.push_back({ 300.f, groundY - 60.f });   // player 4
}

void buildKohonaStage(std::vector<Platform>& platforms, std::vector<sf::Vector2f>& spawns) {
	platforms.clear();
	spawns.clear();

	const float groundY = 300.f;

	// Ground
	Platform ground;
	ground.shape.setSize({ 1600.f, 50.f });
	ground.shape.setFillColor(sf::Color::White);
	ground.shape.setPosition({ -800.f, groundY });
	ground.oneWay = false;
	platforms.push_back(ground);

	// Left Building
	Building leftBuilding;
	leftBuilding.size = { 260.f, 220.f };
	leftBuilding.position = { -500.f, groundY - leftBuilding.size.y - 30.f };
	leftBuilding.ledges = {
		{ 140.f, 180.f, 40.f }, // walk way style ledge
	};
	leftBuilding.ledges.push_back({ 60.f, 60.f, 80.f });
	addBuildingPlatforms(leftBuilding, platforms);

	// Middle Building (open front)
	Building middleBuilding;
	middleBuilding.size = { 340.f, 360.f };
	middleBuilding.position = { -170.f, groundY - middleBuilding.size.y - 50.f };
	middleBuilding.openFront = true;
	middleBuilding.ledges = {
		{ 200.f, 120.f, 40.f },
		{ 140.f, 100.f, 80.f },
		{ 80.f, 80.f, 120.f }
	};
	addBuildingPlatforms(middleBuilding, platforms);

	// Right Building
	Building rightBuilding;
	rightBuilding.size = { 260.f, 220.f };
	rightBuilding.position = { 420.f, groundY - rightBuilding.size.y - 30.f };
	rightBuilding.ledges = {
		{ 110.f, 120.f, 60.f }
	};
	addBuildingPlatforms(rightBuilding, platforms);

	spawns.push_back({ -250.f, groundY - 60.f });  // player 1
	spawns.push_back({ -350.f, groundY - 60.f });  // player 2
	spawns.push_back({ -150.f, groundY - 60.f });  // player 3
	spawns.push_back({ -50.f, groundY - 60.f });   // player 4
}
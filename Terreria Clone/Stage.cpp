#include "Stage.h"

// ============================================================
//  HELPERS
// ============================================================

static sf::RectangleShape makeRect(
	const sf::Vector2f& size,
	const sf::Vector2f& position,
	const sf::Color& color
) {
	sf::RectangleShape r;
	r.setSize(size);
	r.setPosition(position);
	r.setFillColor(color);
	return r;
}

// ============================================================
//  STAGE PLATFORM BUILDING
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

	// Roof (one-way)
	Platform roof;
	roof.shape.setSize({ b.size.x, b.roofThickness });
	roof.shape.setPosition({ b.position.x, b.position.y });
	roof.shape.setFillColor(sf::Color::White);
	roof.oneWay = false;
	platforms.push_back(roof);

	// Interior ledges
	for (const auto& ledge : b.ledges) {
		Platform p;
		p.shape.setSize({ ledge.width, 16.f });
		p.shape.setPosition({ b.position.x + ledge.xOffset, b.position.y + ledge.yOffset });
		p.shape.setFillColor(sf::Color::Red);
		p.oneWay = true;
		platforms.push_back(p);
	}
}

// ============================================================
//  DOJO STAGE
// ============================================================

void buildDojoStage(
	std::vector<Platform>& platforms,
	std::vector<sf::Vector2f>& spawns,
	StageVisuals& visuals) {

	platforms.clear();
	spawns.clear();
	visuals.backgroundLayers.clear();
	visuals.midgroundLayers.clear();
	visuals.foregroundLayers.clear();

	const float groundY = 300.f;

	// ----------------------------
	// Gameplay platforms
	// ----------------------------
	Platform ground;
	ground.shape.setSize({ 1400.f, 50.f });
	ground.shape.setPosition({ -700.f, groundY });
	ground.shape.setFillColor(sf::Color(190, 190, 190));
	ground.shape.setOutlineThickness(2.f);
	ground.shape.setOutlineColor(sf::Color::Green);
	ground.oneWay = false;
	platforms.push_back(ground);

	Platform p1;
	p1.shape.setSize({ 200.f, 16.f });
	p1.shape.setPosition({ -200.f, 220.f });
	p1.shape.setFillColor(sf::Color(220, 220, 220));
	p1.shape.setOutlineThickness(2.f);
	p1.shape.setOutlineColor(sf::Color::Green);
	p1.oneWay = true;
	platforms.push_back(p1);

	Platform p2;
	p2.shape.setSize({ 200.f, 16.f });
	p2.shape.setPosition({ 200.f, 180.f });
	p2.shape.setFillColor(sf::Color(220, 220, 220));
	p2.shape.setOutlineThickness(2.f);
	p2.shape.setOutlineColor(sf::Color::Green);
	p2.oneWay = true;
	platforms.push_back(p2);

	// ----------------------------
	// Spawns
	// ----------------------------
	spawns.push_back({ 0.f, groundY - 60.f });
	spawns.push_back({ -150.f, groundY - 60.f });
	spawns.push_back({ 150.f, groundY - 60.f });
	spawns.push_back({ 300.f, groundY - 60.f });

	// ----------------------------
	// Visual layers
	// ----------------------------

	// Sky
	visuals.backgroundLayers.push_back(
		makeRect({ 2600.f, 1400.f }, { -1300.f, -900.f }, sf::Color(80, 150, 220))
	);

	// Back wall / room silhouette
	visuals.backgroundLayers.push_back(
		makeRect({ 1800.f, 500.f }, { -900.f, -200.f }, sf::Color(60, 70, 90))
	);

	// Dojo room floor backing
	visuals.midgroundLayers.push_back(
		makeRect({ 1600.f, 120.f }, { -800.f, 300.f }, sf::Color(90, 90, 100))
	);

	// Decorative panel left
	visuals.midgroundLayers.push_back(
		makeRect({ 180.f, 220.f }, { -550.f, 40.f }, sf::Color(110, 120, 145))
	);

	// Decorative panel right
	visuals.midgroundLayers.push_back(
		makeRect({ 180.f, 220.f }, { 370.f, 40.f }, sf::Color(110, 120, 145))
	);

	// Foreground trim line
	visuals.foregroundLayers.push_back(
		makeRect({ 1600.f, 8.f }, { -800.f, 350.f }, sf::Color(40, 40, 40))
	);
}

// ============================================================
//  HIDDEN EMBER DISTRICT STAGE
// ============================================================

void buildHiddenEmberStage(
	std::vector<Platform>& platforms,
	std::vector<sf::Vector2f>& spawns,
	StageVisuals& visuals){

	platforms.clear();
	spawns.clear();
	visuals.backgroundLayers.clear();
	visuals.midgroundLayers.clear();
	visuals.foregroundLayers.clear();

	const float groundY = 300.f;
	const float groundLeft = -800.f;
	const float groundRight = 800.f;
	const float mainRoofY = 165.f;

	// ----------------------------
	// Gameplay platforms
	// ----------------------------

	// Ground
	Platform ground;
	ground.shape.setSize({ 1600.f, 50.f });
	ground.shape.setPosition({ -800.f, groundY });
	ground.oneWay = false;
	platforms.push_back(ground);

	// Left Rooftop
	Platform leftRooftop;
	leftRooftop.shape.setSize({ 340.f, 20.f });
	leftRooftop.shape.setPosition({ -760.f, 165.f });
	leftRooftop.oneWay = true;
	platforms.push_back(leftRooftop);

	// Left Hanging Ledge
	Platform leftLedge;
	leftLedge.shape.setSize({ 150.f,16.f });
	leftLedge.shape.setPosition({ -900.f, groundY });
	leftLedge.oneWay = true;
	platforms.push_back(leftLedge);

	// Center Rooftop
	Platform centerRooftop;
	centerRooftop.shape.setSize({ 360.f,16.f });
	centerRooftop.shape.setPosition({ -180.f,165.f });
	centerRooftop.oneWay = true;
	platforms.push_back(centerRooftop);

	// Right Rooftop
	Building rightMainBuilding;
	rightMainBuilding.position = { 440.f, 165.f };
	rightMainBuilding.size = { 360.f, 185.f };
	rightMainBuilding.openFront = false;
	rightMainBuilding.wallThickness = 30.f;
	rightMainBuilding.roofThickness = 16.f;
	addBuildingPlatforms(rightMainBuilding, platforms);

	// Right Mini Rooftop
	Platform rightTop;
	rightTop.shape.setSize({ 120.f,16.f });
	rightTop.shape.setPosition({ 600.f, 70.f });
	rightTop.oneWay = true;
	platforms.push_back(rightTop);

	// ----------------------------
	// Spawns
	// ----------------------------
	spawns.push_back({ -250.f, groundY - 60.f });
	spawns.push_back({ -350.f, groundY - 60.f });
	spawns.push_back({ -150.f, groundY - 60.f });
	spawns.push_back({ -50.f, groundY - 60.f });

	// ----------------------------
	// Visual-only building bodies
	// ----------------------------

	Building leftMainBuilding;
	leftMainBuilding.position = { -760.f, 185.f };
	leftMainBuilding.size = { 340.f, 165.f };

	Building centerLookoutBuilding;
	centerLookoutBuilding.position = { -180.f, 181.f };
	centerLookoutBuilding.size = { 360.f, 120.f };

	Building rightBaseBuilding;
	rightBaseBuilding.position = rightMainBuilding.position;
	rightBaseBuilding.size = rightMainBuilding.size;

	Building rightTopBuilding;
	rightTopBuilding.position = { 600.f, 86.f };
	rightTopBuilding.size = { 120.f, 95.f };

	// ----------------------------
	// Visual layers
	// ----------------------------

	// Sky
	visuals.backgroundLayers.push_back(
		makeRect({ 3000.f, 1400.f }, { -1500.f, -900.f }, sf::Color(115, 175, 235))
	);

	// Warm horizon glow
	visuals.backgroundLayers.push_back(
		makeRect({ 2600.f, 180.f }, { -1300.f, 210.f }, sf::Color(210, 135, 80))
	);

	// Volcano silhouette
	visuals.backgroundLayers.push_back(
		makeRect({ 520.f, 260.f }, { 320.f, 20.f }, sf::Color(85, 70, 70))
	);

	// Volcano peak
	visuals.backgroundLayers.push_back(
		makeRect({ 180.f, 70.f }, { 500.f, -20.f }, sf::Color(95, 75, 75))
	);

	// Distant village / mountain band
	visuals.backgroundLayers.push_back(
		makeRect({ 2400.f, 120.f }, { -1200, 150.f }, sf::Color(105, 120, 135))
	);

	// Village back strip
	visuals.midgroundLayers.push_back(
		makeRect({ 1900.f, 170.f }, { -950.f, 175.f }, sf::Color(145, 108, 88))
	);

	// Darker lower village band
	visuals.midgroundLayers.push_back(
		makeRect({ 1900.f, 70.f }, { -950.f, 275.f }, sf::Color(120, 88, 72))
	);

	// Decorative building faces matching Hidden Ember District layout
	visuals.midgroundLayers.push_back(
		makeRect(leftMainBuilding.size, leftMainBuilding.position, sf::Color(175, 125, 95))
	);

	visuals.midgroundLayers.push_back(
		makeRect(centerLookoutBuilding.size, centerLookoutBuilding.position, sf::Color(175, 132, 98))
	);

	visuals.midgroundLayers.push_back(
		makeRect(rightBaseBuilding.size, rightBaseBuilding.position, sf::Color(168, 126, 96))
	);

	visuals.midgroundLayers.push_back(
		makeRect(rightTopBuilding.size, rightTopBuilding.position, sf::Color(150, 110, 85))
	);

	// Roof caps for style
	visuals.midgroundLayers.push_back(
		makeRect(
			{ leftMainBuilding.size.x + 28.f, 20.f },
			{ leftMainBuilding.position.x - 14.f, leftMainBuilding.position.y - 14.f },
			sf::Color(105, 52, 40)
		)
	);

	visuals.midgroundLayers.push_back(
		makeRect(
			{ centerLookoutBuilding.size.x + 28.f, 22.f },
			{ centerLookoutBuilding.position.x - 14.f, centerLookoutBuilding.position.y - 16.f },
			sf::Color(118, 60, 44)
		)
	);

	visuals.midgroundLayers.push_back(
		makeRect(
			{ rightBaseBuilding.size.x + 28.f, 20.f },
			{ rightBaseBuilding.position.x - 14.f, rightBaseBuilding.position.y - 14.f },
			sf::Color(105, 52, 40)
		)
	);

	visuals.midgroundLayers.push_back(
		makeRect(
			{ rightTopBuilding.size.x + 20.f, 16.f },
			{ rightTopBuilding.position.x - 10.f, rightTopBuilding.position.y - 12.f },
			sf::Color(105, 52, 40)
		)
	);

	// Lava glow accent near horizon
	visuals.midgroundLayers.push_back(
		makeRect({ 1900.f, 18.f }, { -950.f, 330.f }, sf::Color(190, 95, 45))
	);

	// Foreground ground trim
	visuals.foregroundLayers.push_back(
		makeRect({ 1900.f, 12.f }, { -950.f, 350.f }, sf::Color(88, 56, 40))
	);

	// Dark volcanic base strip
	visuals.foregroundLayers.push_back(
		makeRect({ 1900.f, 20.f }, { -950.f,362.f }, sf::Color(52, 42, 42))
	);
}

// ============================================================
//  STAGE DRAWING
// ============================================================

void drawStageBackground(sf::RenderWindow& window, const StageVisuals& visuals) {
	for (const auto& shape : visuals.backgroundLayers) {
		window.draw(shape);
	}
}

void drawStageMidground(sf::RenderWindow& window, const StageVisuals& visuals) {
	for (const auto& shape : visuals.midgroundLayers) {
		window.draw(shape);
	}
}

void drawStagePlatforms(sf::RenderWindow& window, const std::vector<Platform>& platforms) {
	for (const auto& platform : platforms) {
		window.draw(platform.shape);
	}
}

void drawStageForeground(sf::RenderWindow& window, const StageVisuals& visuals) {
	for (const auto& shape : visuals.foregroundLayers) {
		window.draw(shape);
	}
}
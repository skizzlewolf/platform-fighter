#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdint>

// ============================================================
//  GAME FLOW
// ============================================================

enum class StageID {
	Dojo,
	Kohona
};

enum class GameState {
	StageSelect,
	Playing
};

enum class GameMode {
	Versus,
	Training
};


// ============================================================
//  STAGE + PLATFORMS
// ============================================================

// One Way Platforms
struct Platform {
	sf::RectangleShape shape;
	bool oneWay = false;
};

struct Ledge {
	float yOffset; // distance from top of building
	float width;
	float xOffset; // distance from left wall
};

struct Building {
	sf::Vector2f position;
	sf::Vector2f size;
	bool openFront = false;

	float wallThickness = 30.f;
	float roofThickness = 20.f;

	std::vector<Ledge> ledges;
};

// ============================================================
//  INPUT + COMBAT DATA
// ============================================================

struct Controls {
	sf::Keyboard::Key left = sf::Keyboard::Key::A;
	sf::Keyboard::Key right = sf::Keyboard::Key::D;
	sf::Keyboard::Key down = sf::Keyboard::Key::S;
	sf::Keyboard::Key jump = sf::Keyboard::Key::Space;

	sf::Keyboard::Key light = sf::Keyboard::Key::J;
	sf::Keyboard::Key heavy = sf::Keyboard::Key::K;
	sf::Keyboard::Key defense = sf::Keyboard::Key::L; //block if standing still, dodge if moving side-to-side
};

enum class AttackID {
	None,
	Light1,
	Light2,
	Light3,
	Heavy,
	RunLight
};

enum class AttackVariant {
	Neutral,
	Side
};

constexpr float HEAVY_CANCEL_RECOVERY_FRACTION = 0.35f;

struct MoveData {
	float startup = 0.f;  // seconds before hitbox appears
	float active = 0.f;   // seconds hitbox exists
	float recovery = 0.f; // seconds after active before you can act

	sf::Vector2f hitboxSize{ 0.f, 0.f };
	sf::Vector2f hitboxOffset{ 0.f, 0.f }; // relative to player's top left
	sf::Vector2f knockback{ 0.f, 0.f };
	float knockbackScaling = 0.f;
	float damage = 0.f;
	float hitstun = 0.f;

	bool causesKnockdown = false;
	float knockdownTime = 0.f;

	float cancelStart = 0.f; // seconds after startup when cancel becomes allowed
	float cancelEnd = 0.f;   // seconds after startup when cancel stops
	bool cancelOnHitOnly = true;

	float lungeSpeed = 0.f;     // horizontal speed to inject
	float lungeDuration = 0.f;  // seconds after move start where lunge applies

	float slideSpeed = 0.f; //RunLight slide speed
	float slideDuration = 0.f; //seconds the slide velocity applies
};

struct AttackState {
	AttackID current = AttackID::None;
	AttackVariant variant = AttackVariant::Neutral;

	float timer = 0.f;   // counts down total move time (startup+active+recovery)
	bool hasHit = false; // prevent multi-hit per move
};
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <cstdint>

#include "GameTypes.h"
#include "Stage.h"
#include "Player.h"

int main() {
	constexpr float GROUND_Y = 300.f;

	sf::RenderWindow window(sf::VideoMode({ 1080, 720 }), "Anime Mash-Up *BUILDING*");  // Creates window

	GameState gameState = GameState::StageSelect;
	StageID selectedStage = StageID::Training;  // what menu highlights
	StageID currentStage = StageID::Training;   // what is loaded when playing

	sf::Font font;
	if (!font.openFromFile("COLONNA.ttf")) {
		return -1;
	}

	sf::Text title(font);
	title.setFillColor(sf::Color::White);
	title.setString("Select Stage");
	title.setCharacterSize(48);
	title.setPosition({ 200.f, 100.f });

	sf::View camera;
	float cameraZoom = 1.15f;

	sf::FloatRect blastZone(sf::Vector2f{ -1200.f, -800.f }, sf::Vector2f{ 2400.f, 2200.f });  // left, top. width, height

	std::vector<Platform> platforms;  // creates a list of platforms
	std::vector<sf::Vector2f> spawns;
	sf::Clock clock;

	Player player1;
	Player player2;

	bool debugShowHitboxes = true; // toggle hitbox visuals

	// override player2 controls (for now)
	player2.controls.left = sf::Keyboard::Key::Left;
	player2.controls.right = sf::Keyboard::Key::Right;
	player2.controls.down = sf::Keyboard::Key::Down;
	player2.controls.jump = sf::Keyboard::Key::RShift;

	player2.controls.light = sf::Keyboard::Key::Num1;
	player2.controls.heavy = sf::Keyboard::Key::Num2;
	player2.controls.defense = sf::Keyboard::Key::Num3;

	while (window.isOpen()) {

		// -------------------------------
		// 1) EVENTS (one-time inputs)
		// -------------------------------
		while (const std::optional<sf::Event> event = window.pollEvent()) {

			if (event->is<sf::Event::Closed>()) {
				window.close();
			}

			if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {

				// MENU CONTROLS
				if (gameState == GameState::StageSelect) {

					if (key->code == sf::Keyboard::Key::H) {
						debugShowHitboxes = !debugShowHitboxes;
					}

					if (key->code == sf::Keyboard::Key::Num1) selectedStage = StageID::Training;
					if (key->code == sf::Keyboard::Key::Num2) selectedStage = StageID::Kohona;

					if (key->code == sf::Keyboard::Key::Enter) {
						currentStage = selectedStage;

						if (currentStage == StageID::Training) {
							buildTrainingStage(platforms, spawns);
						}
						else {
							buildKohonaStage(platforms, spawns);
						}

						if (!spawns.empty()) {
							player1.resetForStageStart(spawns[0]);  // spawns player 1
							if (spawns.size() > 1) {
								player2.resetForStageStart(spawns[1]);
							}
						}

						gameState = GameState::Playing;
						clock.restart();

						camera = window.getDefaultView();
						camera.zoom(cameraZoom);
						camera.setCenter(player1.center());
					}
				}

				// PLAYING CONTROLS
				else if (gameState == GameState::Playing) {
					if (key->code == sf::Keyboard::Key::Escape) {
						gameState = GameState::StageSelect;
					}
				}
			}
		}

		// -------------------------------
		// 2) STAGE SELECT (draw + skip gameplay)
		// -------------------------------
		if (gameState == GameState::StageSelect) {

			std::string hitboxLine = std::string("\n\nH: Hitboxes ")
				+ (debugShowHitboxes ? "[ON]" : "[OFF]")
				+ "\nEnter to Start";

			if (selectedStage == StageID::Training) {
				title.setString(std::string("Select Stage\n> 1 Training\n  2 Kohona") + hitboxLine);
			}
			else {
				title.setString(std::string("Select Stage\n  1 Training\n> 2 Kohona") + hitboxLine);
			}

			window.clear();
			window.setView(window.getDefaultView());
			window.draw(title);
			window.display();
			continue; // <- gameplay does not run in menu
		}

		// -------------------------------
		// 3) GAMEPLAY UPDATE (dt, input, physics, collision)
		// -------------------------------
		float dt = clock.restart().asSeconds();
		if (dt > 0.05f) dt = 0.05f;

		player1.update(dt, platforms);
		auto c1 = player1.center();
		if (!blastZone.contains(c1)) {
			player1.kill();
		}

		player2.update(dt, platforms);
		auto c2 = player2.center();
		if (!blastZone.contains(c2)) {
			player2.kill();
		}

		// Player combat
		auto resolveHit = [&](Player& attacker, Player& victim) {
			sf::FloatRect hb = attacker.currentHitbox();
			if (hb.size.x <= 0.f) { return; }
			if (attacker.attack.hasHit) { return; }
			if (!victim.canBeHit()) { return; }

			if (hb.findIntersection(victim.hurtbox())) {
				const MoveData* m = attacker.currentMoveData();
				if (!m) { return; }

				sf::Vector2f kb = m->knockback;
				if (attacker.attack.variant == AttackVariant::Side) {
					kb.x *= 1.15f;
					kb.y *= 0.90f;
				}

				float kx = attacker.facingRight ? kb.x : -kb.x;
				bool isRunSlide = (attacker.attack.current == AttackID::RunLight);
				if (isRunSlide) {
					victim.takeHit(sf::Vector2f{ kx, kb.y }, m->hitstun, true, 0.55f);
				}
				else {
					victim.takeHit(sf::Vector2f{ kx, kb.y }, m->hitstun);
				}

				attacker.startHitstop(0.05f);
				victim.startHitstop(0.05f);

				attacker.attack.hasHit = true;
				attacker.lastAttackHit = true;
			}
			};

		resolveHit(player1, player2);
		resolveHit(player2, player1);

		// -------------------------------
		// 4) RENDER
		// -------------------------------
		window.clear();

		// map
		camera.setCenter(player1.center());
		window.setView(camera);

		for (auto& platform : platforms) window.draw(platform.shape);
		player1.draw(window);
		player2.draw(window);

		auto baseColorFor = [&](AttackID id) {
			switch (id) {
			case AttackID::Light1: return sf::Color(255, 255, 0); // yellow
			case AttackID::Light2: return sf::Color(0, 255, 0);   // green
			case AttackID::Light3: return sf::Color(0, 200, 255); // cyan
			case AttackID::Heavy:  return sf::Color(255, 0, 0);   // red
			default:               return sf::Color(255, 255, 255);
			}
			};

		auto withAlpha = [&](sf::Color c, std::uint8_t a) {
			c.a = a;
			return c;
			};

		auto drawDefenseDebug = [&](const Player& p) {
			if (!debugShowHitboxes) { return; }
			if (!p.alive) { return; }

			//Block
			if (p.isBlocking) {
				sf::FloatRect hb = p.hurtbox();

				sf::RectangleShape r;
				r.setPosition(hb.position);
				r.setSize(hb.size);
				r.setFillColor(sf::Color(0, 120, 255, 60));
				r.setOutlineThickness(3.f);
				r.setOutlineColor(sf::Color(0, 120, 255, 200));
				window.draw(r);
			}

			//Dodge
			if (p.dodgeInvulnTimer > 0.f) {
				sf::FloatRect hb = p.hurtbox();

				sf::RectangleShape r;
				r.setPosition(hb.position);
				r.setSize(hb.size);
				r.setFillColor(sf::Color(0, 0, 0, 0));
				r.setOutlineThickness(3.f);
				r.setOutlineColor(sf::Color(180, 0, 255, 220));
				window.draw(r);
			}

			//Knockdown (orange outline)
			if (p.knockdownLockTimer > 0.f || p.knockdownLockTimer > 0.f) {
				sf::FloatRect hb = p.hurtbox();
				sf::RectangleShape r;
				r.setPosition(hb.position);
				r.setSize(hb.size);
				r.setFillColor(sf::Color(0, 0, 0, 0));
				r.setOutlineThickness(3.f);
				r.setOutlineColor(sf::Color(255, 140, 0, 220));
				window.draw(r);
			}
		};

		auto drawHitboxDebug = [&](const Player& p) {
			if (!p.isAttacking()) { return; }

			sf::FloatRect hb = p.plannedHitbox();
			if (hb.size.x <= 0.f) { return; }

			Player::AttackPhase phase = p.attackPhase();
			sf::Color base = baseColorFor(p.attack.current);

			sf::RectangleShape r;
			r.setPosition(hb.position);
			r.setSize(hb.size);
			r.setOutlineThickness(2.f);

			// Startup / Recovery = phantom outline only
			if (phase == Player::AttackPhase::Startup) {
				r.setFillColor(sf::Color(0, 0, 0, 0));
				r.setOutlineColor(withAlpha(base, 120));
			}
			else if (phase == Player::AttackPhase::Recovery) {
				r.setFillColor(sf::Color(0, 0, 0, 0));
				r.setOutlineColor(withAlpha(base, 160));
			}

			// Active = filled + stronger outline
			else if (phase == Player::AttackPhase::Active) {
				r.setFillColor(withAlpha(base, 90));
				r.setOutlineColor(withAlpha(base, 220));
			}
			else { return; }

			window.draw(r);
		};

		if (debugShowHitboxes) {
			drawHitboxDebug(player1);
			drawHitboxDebug(player2);
			drawDefenseDebug(player1);
			drawDefenseDebug(player2);
		}


		// UI
		window.setView(window.getDefaultView());
		// draw ui

		window.display();
	}

	return 0;
}

#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

#include "GameTypes.h"
#include "Stage.h"
#include "Player.h"

struct MenuButton {
	sf::RectangleShape box;
	sf::Text label;

	MenuButton(const sf::Font& font, const sf::Vector2f& size)
		: box(size), label(font) {
		box.setFillColor(sf::Color(40, 40, 40));
		box.setOutlineThickness(3.f);
		box.setOutlineColor(sf::Color::White);

		label.setCharacterSize(28);
		label.setFillColor(sf::Color::White);
	}

	bool contains(const sf::Vector2f& mousePos) const {
		return box.getGlobalBounds().contains(mousePos);
	}
};

static void centerLabelInButton(MenuButton& b) {
	sf::FloatRect textBounds = b.label.getLocalBounds();
	sf::FloatRect boxBounds = b.box.getGlobalBounds();

	b.label.setPosition({
		boxBounds.position.x + (boxBounds.size.x - textBounds.size.x) * 0.5f - textBounds.position.x,
		boxBounds.position.y + (boxBounds.size.y - textBounds.size.y) * 0.5f - textBounds.position.y - 2.f
		});
}

int main() {
	sf::RenderWindow window(sf::VideoMode({ 1080, 720 }), "*Pre-Alpha 4-Corners*");
	window.setFramerateLimit(60);

	GameState gameState = GameState::StageSelect;
	StageID selectedStage = StageID::Dojo;
	StageID currentStage = StageID::Dojo;
	GameMode selectedMode = GameMode::Training;
	GameMode currentMode = GameMode::Training;

	sf::Font font;
	if (!font.openFromFile("COLONNA.ttf")) {
		return -1;
	}

	sf::Text title(font);
	title.setFillColor(sf::Color::White);
	title.setCharacterSize(48);

	sf::Text subtitle(font);
	subtitle.setFillColor(sf::Color(220, 220, 220));
	subtitle.setCharacterSize(22);

	sf::Text p1DamageText(font);
	p1DamageText.setCharacterSize(32);
	p1DamageText.setFillColor(sf::Color::White);
	p1DamageText.setPosition({ 30.f, 20.f });

	sf::Text p2DamageText(font);
	p2DamageText.setCharacterSize(32);
	p2DamageText.setFillColor(sf::Color::White);
	p2DamageText.setPosition({ 900.f, 20.f });

	sf::View camera;
	float cameraZoom = 1.15f;

	sf::FloatRect blastZone(
		sf::Vector2f{ -1200.f, -800.f },
		sf::Vector2f{ 2400.f, 2200.f }
	);

	std::vector<Platform> platforms;
	std::vector<sf::Vector2f> spawns;
	sf::Clock clock;

	Player player1;
	Player player2;

	// Player 2 controls
	player2.controls.left = sf::Keyboard::Key::Left;
	player2.controls.right = sf::Keyboard::Key::Right;
	player2.controls.down = sf::Keyboard::Key::Down;
	player2.controls.jump = sf::Keyboard::Key::RShift;
	player2.controls.light = sf::Keyboard::Key::Num1;
	player2.controls.heavy = sf::Keyboard::Key::Num2;
	player2.controls.defense = sf::Keyboard::Key::Num3;

	// ------------------------
	// Pause + Menu State
	// ------------------------
	bool isPaused = false;

	enum class PauseMenuPage {
		Main,
		Training
	};

	PauseMenuPage pauseMenuPage = PauseMenuPage::Main;

	int stageSelectIndex = 0;
	int pauseMenuIndex = 0;
	int trainingMenuIndex = 0;

	// ------------------------
	// Debug / Training Toggles
	// ------------------------
	bool debugShowHitboxes = true;
	bool debugShowHurtboxes = false;
	bool debugShowTimers = true;
	bool debugShowComboHUD = true;
	bool debugShowStateOverlays = true;
	bool debugShowStateTints = true;

	// ------------------------
	// Menu Buttons
	// ------------------------
	std::vector<MenuButton> stageButtons;
	std::vector<MenuButton> pauseButtons;
	std::vector<MenuButton> trainingButtons;

	auto fmt2 = [](float value) {
		std::ostringstream out;
		out << std::fixed << std::setprecision(2) << value;
		return out.str();
	};

	auto makeVerticalButtons = [&](std::vector<MenuButton>& buttons, int count, sf::Vector2f start, sf::Vector2f size, float gap) {
		buttons.clear();
		for (int i = 0; i < count; i++) {
			MenuButton b(font, size);
			b.box.setPosition({ start.x, start.y + i * gap });
			buttons.push_back(std::move(b));
		}
	};

	makeVerticalButtons(stageButtons, 4, { 320.f, 200.f }, { 440.f, 64.f }, 78.f);
	makeVerticalButtons(pauseButtons, 5, { 320.f, 190.f }, { 440.f, 64.f }, 80.f);
	makeVerticalButtons(trainingButtons, 7, { 320.f, 130.f }, { 440.f, 64.f }, 68.f);

	auto updateStageSelectMenu = [&]() {
		title.setString("Select Stage");
		title.setPosition({ 360.f, 95.f });

		subtitle.setString("WASD / Arrows / Enter / Mouse");
		subtitle.setPosition({ 320.f, 150.f });

		stageButtons[0].label.setString(
			std::string("Stage: Dojo") + (selectedStage == StageID::Dojo ? "   <" : "")
		);
		stageButtons[1].label.setString(
			std::string("Stage: Kohona") + (selectedStage == StageID::Kohona ? "   <" : "")
		);
		stageButtons[2].label.setString(
			std::string("Mode: ") + (selectedMode == GameMode::Training ? "Training" : "Versus")
		);
		stageButtons[3].label.setString("Start Match");

		for (auto& b : stageButtons) {
			centerLabelInButton(b);
		}
	};

	auto updatePauseMenu = [&]() {
		title.setString("Paused");
		title.setPosition({ 430.f, 95.f });

		subtitle.setString("WASD / Arrows / Enter / Mouse / Esc");
		subtitle.setPosition({ 255.f, 145.f });

		pauseButtons[0].label.setString("Resume");
		pauseButtons[1].label.setString("Restart Match");
		pauseButtons[2].label.setString("Return to Stage Select");
		pauseButtons[3].label.setString("Settings (Later)");

		if (currentMode == GameMode::Training) {
			pauseButtons[4].label.setString("Training Settings");
		}
		else {
			pauseButtons[4].label.setString("");
		}

		for (auto& b : pauseButtons) {
			centerLabelInButton(b);
		}
	};

	auto updateTrainingMenu = [&]() {
		title.setString("Training Settings");
		title.setPosition({ 325.f, 90.f });

		subtitle.setString("Enter or A/D to toggle   |   Esc to go back");
		subtitle.setPosition({ 225.f, 140.f });

		trainingButtons[0].label.setString(std::string("Show Hitboxes: ") + (debugShowHitboxes ? "ON" : "OFF"));
		trainingButtons[1].label.setString(std::string("Show Hurtboxes: ") + (debugShowHurtboxes ? "ON" : "OFF"));
		trainingButtons[2].label.setString(std::string("Show State Overlays: ") + (debugShowStateOverlays ? "ON" : "OFF"));
		trainingButtons[3].label.setString(std::string("Show State Tints: ") + (debugShowStateTints ? "ON" : "OFF"));
		trainingButtons[4].label.setString(std::string("Show Timers: ") + (debugShowTimers ? "ON" : "OFF"));
		trainingButtons[5].label.setString(std::string("Show Combo HUD: ") + (debugShowComboHUD ? "ON" : "OFF"));
		trainingButtons[6].label.setString("Back");

		for (auto& b : trainingButtons) {
			centerLabelInButton(b);
		}
	};

	auto updateAllMenus = [&]() {
		updateStageSelectMenu();
		updatePauseMenu();
		updateTrainingMenu();
	};

	auto startMatch = [&]() {
		currentStage = selectedStage;
		currentMode = selectedMode;

		if (currentStage == StageID::Dojo) {
			buildTrainingStage(platforms, spawns);
		}
		else {
			buildKohonaStage(platforms, spawns);
		}

		if (!spawns.empty()) {
			player1.resetForStageStart(spawns[0]);
			if (spawns.size() > 1) {
				player2.resetForStageStart(spawns[1]);
			}
		}

		isPaused = false;
		pauseMenuPage = PauseMenuPage::Main;
		pauseMenuIndex = 0;
		trainingMenuIndex = 0;

		gameState = GameState::Playing;
		clock.restart();

		camera = window.getDefaultView();
		camera.zoom(cameraZoom);
		camera.setCenter(player1.center());

		updateAllMenus();
	};

	auto restartMatch = [&]() {
		if (currentStage == StageID::Dojo) {
			buildTrainingStage(platforms, spawns);
		}
		else {
			buildKohonaStage(platforms, spawns);
		}

		if (!spawns.empty()) {
			player1.resetForStageStart(spawns[0]);
			if (spawns.size() > 1) {
				player2.resetForStageStart(spawns[1]);
			}
		}

		isPaused = false;
		pauseMenuPage = PauseMenuPage::Main;
		pauseMenuIndex = 0;
		trainingMenuIndex = 0;

		clock.restart();
		camera = window.getDefaultView();
		camera.zoom(cameraZoom);
		camera.setCenter(player1.center());
		};

	auto activateStageSelectItem = [&]() {
		if (stageSelectIndex == 0) {
			selectedStage = StageID::Dojo;
		}
		else if (stageSelectIndex == 1) {
			selectedStage = StageID::Kohona;
		}
		else if (stageSelectIndex == 2) {
			selectedMode = (selectedMode == GameMode::Training) ? GameMode::Versus : GameMode::Training;
		}
		else if (stageSelectIndex == 3) {
			startMatch();
		}

		updateStageSelectMenu();
	};

	auto activatePauseItem = [&]() {
		switch (pauseMenuIndex) {
		case 0:
			isPaused = false;
			break;

		case 1:
			restartMatch();
			break;

		case 2:
			isPaused = false;
			gameState = GameState::StageSelect;
			stageSelectIndex = (selectedStage == StageID::Dojo) ? 0 : 1;
			break;

		case 3:
			// placeholder for later real settings
			break;

		case 4:
			if (currentMode == GameMode::Training) {
				pauseMenuPage = PauseMenuPage::Training;
				trainingMenuIndex = 0;
			}
			break;
		}

		updateAllMenus();
	};

	auto activateTrainingItem = [&]() {
		switch (trainingMenuIndex) {
		case 0:
			debugShowHitboxes = !debugShowHitboxes;
			break;
		case 1:
			debugShowHurtboxes = !debugShowHurtboxes;
			break;
		case 2:
			debugShowStateOverlays = !debugShowStateOverlays;
			break;
		case 3:
			debugShowStateTints = !debugShowStateTints;
		case 4:
			debugShowTimers = !debugShowTimers;
			break;
		case 5:
			debugShowComboHUD = !debugShowComboHUD;
			break;
		case 6:
			pauseMenuPage = PauseMenuPage::Main;
			pauseMenuIndex = 0;
			break;
		}

		updateTrainingMenu();
	};

	updateAllMenus();

	while (window.isOpen()) {
		while (const std::optional<sf::Event> event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}

			// -------------------------------
			// Keyboard input
			// -------------------------------
			if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {

				// -------------------------------
				// Stage Select
				// -------------------------------
				if (gameState == GameState::StageSelect) {
					if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
						stageSelectIndex--;
						if (stageSelectIndex < 0) stageSelectIndex = 3;
					}
					else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
						stageSelectIndex++;
						if (stageSelectIndex > 3) stageSelectIndex = 0;
					}
					else if (
						stageSelectIndex == 2 &&
						key->code == sf::Keyboard::Key::A || key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::D || key->code == sf::Keyboard::Key::Right) {
						selectedMode = (selectedMode == GameMode::Training) ? GameMode::Versus : GameMode::Training;
					}
					else if (key->code == sf::Keyboard::Key::Enter) {
						activateStageSelectItem();
					}

					if (stageSelectIndex == 0) selectedStage = StageID::Dojo;
					if (stageSelectIndex == 1) selectedStage = StageID::Kohona;

					updateStageSelectMenu();
				}

				// -------------------------------
				// Playing / Pause
				// -------------------------------
				else if (gameState == GameState::Playing) {
					if (key->code == sf::Keyboard::Key::Escape) {
						if (!isPaused) {
							isPaused = true;
							pauseMenuPage = PauseMenuPage::Main;
							pauseMenuIndex = 0;
						}
						else {
							if (pauseMenuPage == PauseMenuPage::Training) {
								pauseMenuPage = PauseMenuPage::Main;
								pauseMenuIndex = 0;
							}
							else {
								isPaused = false;
							}
						}
						updateAllMenus();
					}
					else if (isPaused) {
						if (pauseMenuPage == PauseMenuPage::Main) {
							int pauseItemMax = (currentMode == GameMode::Training) ? 4 : 3;

							if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
								pauseMenuIndex--;
								if (pauseMenuIndex < 0) pauseMenuIndex = pauseItemMax;
							}
							else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
								pauseMenuIndex++;
								if (pauseMenuIndex > pauseItemMax) pauseMenuIndex = 0;
							}
							else if (key->code == sf::Keyboard::Key::Enter) {
								activatePauseItem();
							}
						}
						else if (pauseMenuPage == PauseMenuPage::Training) {
							if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
								trainingMenuIndex--;
								if (trainingMenuIndex < 0) trainingMenuIndex = 6;
							}
							else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
								trainingMenuIndex++;
								if (trainingMenuIndex > 6) trainingMenuIndex = 0;
							}
							else if (key->code == sf::Keyboard::Key::A || key->code == sf::Keyboard::Key::Left ||
								key->code == sf::Keyboard::Key::D || key->code == sf::Keyboard::Key::Right ||
								key->code == sf::Keyboard::Key::Enter) {
								activateTrainingItem();
							}
						}
					}
				}
			}

			// -------------------------------
			// Mouse move
			// -------------------------------
			if (const auto* moved = event->getIf<sf::Event::MouseMoved>()) {
				sf::Vector2f mousePos = window.mapPixelToCoords(
					{ moved->position.x, moved->position.y },
					window.getDefaultView()
				);

				if (gameState == GameState::StageSelect) {
					for (int i = 0; i < (int)stageButtons.size(); i++) {
						if (stageButtons[i].contains(mousePos)) {
							stageSelectIndex = i;
							if (i == 0) selectedStage = StageID::Dojo;
							if (i == 1) selectedStage = StageID::Kohona;
						}
					}
					updateStageSelectMenu();
				}
				else if (gameState == GameState::Playing && isPaused) {
					if (pauseMenuPage == PauseMenuPage::Main) {
						int pauseButtonCount = (currentMode == GameMode::Training) ? 5 : 4;

						for (int i = 0; i < pauseButtonCount; i++) {
							if (pauseButtons[i].contains(mousePos)) {
								pauseMenuIndex = i;
								break;
							}
						}
					}
					else if (pauseMenuPage == PauseMenuPage::Training) {
						for (int i = 0; i < (int)trainingButtons.size(); i++) {
							if (trainingButtons[i].contains(mousePos)) {
								trainingMenuIndex = i;
							}
						}
					}
				}
			}

			// -------------------------------
			// Mouse click
			// -------------------------------
			if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (mouse->button == sf::Mouse::Button::Left) {
					sf::Vector2f mousePos = window.mapPixelToCoords(
						{ mouse->position.x, mouse->position.y },
						window.getDefaultView()
					);

					if (gameState == GameState::StageSelect) {
						for (int i = 0; i < (int)stageButtons.size(); i++) {
							if (stageButtons[i].contains(mousePos)) {
								stageSelectIndex = i;
								if (i == 0) selectedStage = StageID::Dojo;
								if (i == 1) selectedStage = StageID::Kohona;
								activateStageSelectItem();
								break;
							}
						}
					}
					else if (gameState == GameState::Playing && isPaused) {
						if (pauseMenuPage == PauseMenuPage::Main) {
							int pauseButtonCount = (currentMode == GameMode::Training) ? 5 : 4;

							for (int i = 0; i < pauseButtonCount; i++) {
								if (pauseButtons[i].contains(mousePos)) {
									pauseMenuIndex = i;
									activatePauseItem();
									break;
								}
							}
						}
						else if (pauseMenuPage == PauseMenuPage::Training) {
							for (int i = 0; i < (int)trainingButtons.size(); i++) {
								if (trainingButtons[i].contains(mousePos)) {
									trainingMenuIndex = i;
									activateTrainingItem();
									break;
								}
							}
						}
					}
				}
			}
		}

		// -------------------------------
		// Stage Select render
		// -------------------------------
		if (gameState == GameState::StageSelect) {
			window.clear(sf::Color(12, 12, 12));
			window.setView(window.getDefaultView());

			window.draw(title);
			window.draw(subtitle);

			for (int i = 0; i < (int)stageButtons.size(); i++) {
				bool selected = (i == stageSelectIndex);

				stageButtons[i].box.setFillColor(selected ? sf::Color(70, 70, 110) : sf::Color(40, 40, 40));
				stageButtons[i].box.setOutlineColor(selected ? sf::Color::Yellow : sf::Color::White);

				window.draw(stageButtons[i].box);
				window.draw(stageButtons[i].label);
			}

			window.display();
			continue;
		}

		// -------------------------------
		// Gameplay update only if NOT paused
		// -------------------------------
		float dt = clock.restart().asSeconds();
		if (dt > 0.05f) dt = 0.05f;

		if (!isPaused) {
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

					victim.takeHit(
						sf::Vector2f{ kx, kb.y },
						m->hitstun,
						m->damage,
						m->knockbackScaling,
						m->causesKnockdown,
						m->knockdownTime
					);

					victim.registerComboHit();

					attacker.startHitstop(0.05f);
					victim.startHitstop(0.05f);

					attacker.attack.hasHit = true;
					attacker.lastAttackHit = true;
				}
			};

			resolveHit(player1, player2);
			resolveHit(player2, player1);
		}
		else {
			clock.restart();
		}

		// -------------------------------
		// Render gameplay
		// -------------------------------
		window.clear();

		camera.setCenter(player1.center());
		window.setView(camera);

		for (auto& platform : platforms) window.draw(platform.shape);
		bool trainingDebugAllowed = (currentMode == GameMode::Training);
		bool showStateTintsNow = trainingDebugAllowed && debugShowStateTints;

		player1.draw(window, showStateTintsNow);
		player2.draw(window, showStateTintsNow);

		auto baseColorFor = [&](AttackID id) {
			switch (id) {
			case AttackID::Light1: return sf::Color(255, 255, 0);
			case AttackID::Light2: return sf::Color(0, 255, 0);
			case AttackID::Light3: return sf::Color(0, 200, 255);
			case AttackID::Heavy:  return sf::Color(255, 0, 0);
			case AttackID::RunLight: return sf::Color(255, 255, 255);
			default: return sf::Color(255, 255, 255);
			}
		};

		auto withAlpha = [&](sf::Color c, std::uint8_t a) {
			c.a = a;
			return c;
		};

		auto drawStateOverlays = [&](const Player& p) {
			if (!debugShowStateOverlays) { return; }
			if (!p.alive) { return; }

			// Block
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

			// Dodge Invul
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

			// Knockdown
			if (p.knockdownTimer > 0.f || p.knockdownLockTimer > 0.f) {
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

			if (phase == Player::AttackPhase::Startup) {
				r.setFillColor(sf::Color(0, 0, 0, 0));
				r.setOutlineColor(withAlpha(base, 120));
			}
			else if (phase == Player::AttackPhase::Recovery) {
				r.setFillColor(sf::Color(0, 0, 0, 0));
				r.setOutlineColor(withAlpha(base, 160));
			}
			else if (phase == Player::AttackPhase::Active) {
				r.setFillColor(withAlpha(base, 90));
				r.setOutlineColor(withAlpha(base, 220));
			}
			else {
				return;
			}

			window.draw(r);
		};

		auto drawHurtboxDebug = [&](const Player& p) {
			if (!debugShowHurtboxes) { return; }
			if (!p.alive) { return; }

			sf::FloatRect hb = p.hurtbox();

			sf::RectangleShape r;
			r.setPosition(hb.position);
			r.setSize(hb.size);
			r.setFillColor(sf::Color(0, 0, 0, 0));
			r.setOutlineThickness(2.f);
			r.setOutlineColor(sf::Color(255, 255, 255, 180));
			window.draw(r);
		};

		if (trainingDebugAllowed && debugShowHitboxes) {
			drawHitboxDebug(player1);
			drawHitboxDebug(player2);
		}

		if (trainingDebugAllowed && debugShowStateOverlays) {
			drawStateOverlays(player1);
			drawStateOverlays(player2);
		}

		if (trainingDebugAllowed && debugShowHurtboxes) {
			drawHurtboxDebug(player1);
			drawHurtboxDebug(player2);
		}

		// -------------------------------
		// UI
		// -------------------------------
		window.setView(window.getDefaultView());

		auto damageColor = [&](float dmg) {
			if (dmg < 50.f) { return sf::Color::White; }
			if (dmg < 100.f) { return sf::Color::Yellow; }
			if (dmg < 150.f) { return sf::Color(255, 165, 0); }
			return sf::Color::Red;
		};

		p1DamageText.setString("P1: " + std::to_string((int)player1.getDamage()) + "%");
		p2DamageText.setString("P2: " + std::to_string((int)player2.getDamage()) + "%");

		p1DamageText.setFillColor(damageColor(player1.getDamage()));
		p2DamageText.setFillColor(damageColor(player2.getDamage()));

		window.draw(p1DamageText);
		window.draw(p2DamageText);

		if (debugShowComboHUD && currentMode == GameMode::Training) {
			sf::Text comboDebug(font);
			comboDebug.setCharacterSize(20);
			comboDebug.setFillColor(sf::Color::White);
			comboDebug.setPosition({ 30.f, 70.f });

			comboDebug.setString(
				"P1 Combo Count: " + std::to_string(player1.comboCount) +
				"\nP1 Combo Timer: " + fmt2(player1.comboTimer) +
				"\nP1 Hitstun: " + fmt2(player1.hitstunTimer) +
				"\nP1 Blockstun: " + fmt2(player1.blockstunTimer) +
				"\nP1 Knockdown: " + fmt2(player1.knockdownTimer) +
				"\n\nP2 Combo Count: " + std::to_string(player2.comboCount) +
				"\nP2 Combo Timer: " + fmt2(player2.comboTimer) +
				"\nP2 Hitstun: " + fmt2(player2.hitstunTimer) +
				"\nP2 Blockstun: " + fmt2(player2.blockstunTimer) +
				"\nP2 Knockdown: " + fmt2(player2.knockdownTimer)
			);

			window.draw(comboDebug);
		}

		if (debugShowTimers && currentMode == GameMode::Training) {
			sf::Text timerDebug(font);
			timerDebug.setCharacterSize(18);
			timerDebug.setFillColor(sf::Color(220, 220, 220));
			timerDebug.setPosition({ 790.f, 70.f });

			timerDebug.setString(
				"P1 DodgeCD: " + fmt2(player1.dodgeCooldownTimer) +
				"\nP1 Invuln: " + fmt2(player1.spawnInvulnTimer) +
				"\nP1 Hitstop: " + fmt2(player1.hitstopTimer) +
				"\n\nP2 DodgeCD: " + fmt2(player2.dodgeCooldownTimer) +
				"\nP2 Invuln: " + fmt2(player2.spawnInvulnTimer) +
				"\nP2 Hitstop: " + fmt2(player2.hitstopTimer)
			);

			window.draw(timerDebug);
		}

		// -------------------------------
		// Pause overlay
		// -------------------------------
		if (isPaused) {
			sf::RectangleShape overlay({ 1080.f, 720.f });
			overlay.setFillColor(sf::Color(0, 0, 0, 170));
			window.draw(overlay);

			window.draw(title);
			window.draw(subtitle);

			if (pauseMenuPage == PauseMenuPage::Main) {
				int pauseButtonCount = (currentMode == GameMode::Training) ? 5 : 4;

				for (int i = 0; i < pauseButtonCount; i++) {
					bool selected = (i == pauseMenuIndex);

					pauseButtons[i].box.setFillColor(selected ? sf::Color(70, 70, 110) : sf::Color(40, 40, 40));
					pauseButtons[i].box.setOutlineColor(selected ? sf::Color::Yellow : sf::Color::White);

					window.draw(pauseButtons[i].box);
					window.draw(pauseButtons[i].label);
				}
			}
			else if (pauseMenuPage == PauseMenuPage::Training) {
				for (int i = 0; i < (int)trainingButtons.size(); i++) {
					bool selected = (i == trainingMenuIndex);

					trainingButtons[i].box.setFillColor(selected ? sf::Color(70, 70, 110) : sf::Color(40, 40, 40));
					trainingButtons[i].box.setOutlineColor(selected ? sf::Color::Yellow : sf::Color::White);

					window.draw(trainingButtons[i].box);
					window.draw(trainingButtons[i].label);
				}
			}
		}

		window.display();
	}

	return 0;
}
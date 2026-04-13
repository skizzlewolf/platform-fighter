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
#include "UI.h"


static void drawStartScreen(sf::RenderWindow& window, sf::Font& font) {
	window.clear(sf::Color(20, 20, 30));

	sf::Text title(font);
	title.setString("4-CORNERS");
	title.setCharacterSize(64);
	title.setFillColor(sf::Color::White);
	title.setPosition({ 330.f, 180.f });

	sf::Text prompt(font);
	prompt.setString("Press Any Key");
	prompt.setCharacterSize(30);
	prompt.setFillColor(sf::Color(220, 220, 220));
	prompt.setPosition({ 390.f,390.f });

	window.draw(title);
	window.draw(prompt);
}

static void drawSimpleVerticalMenu(
	sf::RenderWindow& window,
	sf::Font& font,
	const std::string& header,
	const std::vector<std::string>& options,
	int selectedIndex
) {
	window.clear(sf::Color(25, 25, 35));

	sf::Text title(font);
	title.setString(header);
	title.setCharacterSize(54);
	title.setFillColor(sf::Color::White);
	title.setPosition({ 80.f,70.f });
	window.draw(title);

	for (int i = 0; i < static_cast<int>(options.size()); i++) {
		sf::Text item(font);
		item.setString(options[i]);
		item.setCharacterSize(36);
		item.setPosition({ 120.f, 180.f + i * 70.f });
		item.setFillColor(i == selectedIndex ? sf::Color::Yellow : sf::Color::White);
		window.draw(item);
	}
}


int main() {
	sf::RenderWindow window(sf::VideoMode({ 1080, 720 }), "*Pre-Alpha 4-Corners*");
	window.setFramerateLimit(60);

	GameState gameState = GameState::StartScreen;
	StageID selectedStage = StageID::Dojo;
	StageID currentStage = StageID::Dojo;
	GameMode selectedMode = GameMode::Training;
	GameMode currentMode = GameMode::Training;

	int mainMenuIndex = 0;
	int modeMenuIndex = 0;

	std::vector<std::string> mainMenuOptions = {
		"Play",
		"Training",
		"Settings",
		"Exit"
	};

	std::vector<std::string> modeMenuOptions = {
		"1v1 Casual",
		"2v2 Casual",
		"Tournament",
		"Back"
	};

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
	StageVisuals stageVisuals;
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

	MenuUIState ui;
	DebugSettings debug;

	auto fmt2 = [](float value) {
		std::ostringstream out;
		out << std::fixed << std::setprecision(2) << value;
		return out.str();
	};

	initializeMenus(ui, font);
	
	auto refreshMenus = [&]() {
		updateAllMenus(
			title,
			subtitle,
			ui,
			selectedStage,
			selectedMode,
			currentMode,
			debug
		);
	};

	auto startMatch = [&]() {
		currentStage = selectedStage;
		currentMode = selectedMode;

		if (currentStage == StageID::Dojo) {
			buildDojoStage(platforms, spawns, stageVisuals);
		}
		else {
			buildHiddenEmberStage(platforms, spawns, stageVisuals);
		}

		if (!spawns.empty()) {
			player1.resetForStageStart(spawns[0]);
			if (spawns.size() > 1) {
				player2.resetForStageStart(spawns[1]);
			}
		}

		isPaused = false;
		ui.pauseMenuPage = PauseMenuPage::Main;
		ui.pauseMenuIndex = 0;
		ui.trainingMenuIndex = 0;

		gameState = GameState::Playing;
		clock.restart();

		camera = window.getDefaultView();
		camera.zoom(cameraZoom);
		camera.setCenter(player1.center());

		refreshMenus();
	};

	auto restartMatch = [&]() {
		if (currentStage == StageID::Dojo) {
			buildDojoStage(platforms, spawns, stageVisuals);
		}
		else {
			buildHiddenEmberStage(platforms, spawns, stageVisuals);
		}

		if (!spawns.empty()) {
			player1.resetForStageStart(spawns[0]);
			if (spawns.size() > 1) {
				player2.resetForStageStart(spawns[1]);
			}
		}

		isPaused = false;
		ui.pauseMenuPage = PauseMenuPage::Main;
		ui.pauseMenuIndex = 0;
		ui.trainingMenuIndex = 0;

		clock.restart();
		camera = window.getDefaultView();
		camera.zoom(cameraZoom);
		camera.setCenter(player1.center());
		};

	auto activateStageSelectItem = [&]() {
		if (ui.stageSelectIndex == 0) {
			selectedStage = StageID::Dojo;
		}
		else if (ui.stageSelectIndex == 1) {
			selectedStage = StageID::HiddenEmber;
		}
		else if (ui.stageSelectIndex == 2) {
			selectedMode = (selectedMode == GameMode::Training) ? GameMode::Versus : GameMode::Training;
		}
		else if (ui.stageSelectIndex == 3) {
			startMatch();
		}

		refreshMenus();
	};

	auto activatePauseItem = [&]() {
		switch (ui.pauseMenuIndex) {
		case 0:
			isPaused = false;
			break;

		case 1:
			restartMatch();
			break;

		case 2:
			isPaused = false;
			gameState = GameState::StageSelect;
			ui.stageSelectIndex = (selectedStage == StageID::Dojo) ? 0 : 1;
			break;

		case 3:
			// placeholder for later real settings
			break;

		case 4:
			if (currentMode == GameMode::Training) {
				ui.pauseMenuPage = PauseMenuPage::Training;
				ui.trainingMenuIndex = 0;
			}
			break;
		}

		refreshMenus();
	};

	auto activateTrainingItem = [&]() {
		switch (ui.trainingMenuIndex) {
		case 0:
			debug.showHitboxes = !debug.showHitboxes;
			break;
		case 1:
			debug.showHurtboxes = !debug.showHurtboxes;
			break;
		case 2:
			debug.showStateOverlays = !debug.showStateOverlays;
			break;
		case 3:
			debug.showStateTints = !debug.showStateTints;
			break;
		case 4:
			debug.showTimers = !debug.showTimers;
			break;
		case 5:
			debug.showComboHUD = !debug.showComboHUD;
			break;
		case 6:
			ui.pauseMenuPage = PauseMenuPage::Main;
			ui.pauseMenuIndex = 0;
			break;
		}

		refreshMenus();
	};

	auto activateModeMenuItem = [&]() {
		if (modeMenuIndex == 0) {
			selectedMode = GameMode::Versus;
			gameState = GameState::StageSelect;
		}
		else if (modeMenuIndex == 1) {
			selectedMode = GameMode::Versus;
			gameState = GameState::StageSelect;
		}
		else if (modeMenuIndex == 2) {
			selectedMode = GameMode::Versus;
			gameState = GameState::StageSelect;
		}
		else if (modeMenuIndex == 3) {
			gameState = GameState::MainMenu;
		}

		refreshMenus();
	};

	refreshMenus();

	while (window.isOpen()) {
		while (const std::optional<sf::Event> event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}


			// -------------------------------
			// Keyboard input
			// -------------------------------
			if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {

				//--------------------------------
				// Start Screen
				//--------------------------------
				if (gameState == GameState::StartScreen) {
					gameState = GameState::MainMenu;
				}

				//--------------------------------
				// Main Menu
				//--------------------------------
				else if (gameState == GameState::MainMenu) {
					if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
						mainMenuIndex--;
						if (mainMenuIndex < 0) { mainMenuIndex = (int)mainMenuOptions.size() -1; }
					}
					else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
						mainMenuIndex++;
						if (mainMenuIndex >= (int)mainMenuOptions.size()) { mainMenuIndex = 0; }
					}
					else if (key->code == sf::Keyboard::Key::Enter) {
						if (mainMenuIndex == 0) {
							gameState = GameState::ModeSelect;
						}
						else if (mainMenuIndex == 1) {
							selectedMode = GameMode::Training;
							gameState = GameState::StageSelect;
						}
						else if (mainMenuIndex == 2) {
							gameState = GameState::Settings;
						}
						else if (mainMenuIndex == 3) {
							window.close();
						}
					}

				}

				//--------------------------------
				// Mode Select
				//--------------------------------
				else if (gameState == GameState::ModeSelect) {
					if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
						modeMenuIndex--;
						if (modeMenuIndex < 0) { modeMenuIndex = (int)modeMenuOptions.size() -1; }
					}
					else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
						modeMenuIndex++;
						if (modeMenuIndex >= (int)modeMenuOptions.size()) { modeMenuIndex = 0; }
					}
					else if (key->code == sf::Keyboard::Key::Enter) {
						activateModeMenuItem();
					}
					else if (key->code == sf::Keyboard::Key::Escape) {
						gameState = GameState::MainMenu;
					}
				}

				// -------------------------------
				// Stage Select
				// -------------------------------
				else if (gameState == GameState::StageSelect) {
					if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
						ui.stageSelectIndex--;
						if (ui.stageSelectIndex < 0) ui.stageSelectIndex = 3;
					}
					else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
						ui.stageSelectIndex++;
						if (ui.stageSelectIndex > 3) ui.stageSelectIndex = 0;
					}
					else if (
						ui.stageSelectIndex == 2 &&
						(
							key->code == sf::Keyboard::Key::A ||
							key->code == sf::Keyboard::Key::Left ||
							key->code == sf::Keyboard::Key::D ||
							key->code == sf::Keyboard::Key::Right
							)
						) {
						selectedMode = (selectedMode == GameMode::Training) ? GameMode::Versus : GameMode::Training;
					}
					else if (key->code == sf::Keyboard::Key::Enter) {
						activateStageSelectItem();
					}

					if (ui.stageSelectIndex == 0) selectedStage = StageID::Dojo;
					if (ui.stageSelectIndex == 1) selectedStage = StageID::HiddenEmber;

					refreshMenus();
				}

				// -------------------------------
				// Playing / Pause
				// -------------------------------
				else if (gameState == GameState::Playing) {
					if (key->code == sf::Keyboard::Key::Escape) {
						if (!isPaused) {
							isPaused = true;
							ui.pauseMenuPage = PauseMenuPage::Main;
							ui.pauseMenuIndex = 0;
						}
						else {
							if (ui.pauseMenuPage == PauseMenuPage::Training) {
								ui.pauseMenuPage = PauseMenuPage::Main;
								ui.pauseMenuIndex = 0;
							}
							else {
								isPaused = false;
							}
						}
						refreshMenus();
					}
					else if (isPaused) {
						if (ui.pauseMenuPage == PauseMenuPage::Main) {
							int pauseItemMax = (currentMode == GameMode::Training) ? 4 : 3;

							if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
								ui.pauseMenuIndex--;
								if (ui.pauseMenuIndex < 0) ui.pauseMenuIndex = pauseItemMax;
							}
							else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
								ui.pauseMenuIndex++;
								if (ui.pauseMenuIndex > pauseItemMax) ui.pauseMenuIndex = 0;
							}
							else if (key->code == sf::Keyboard::Key::Enter) {
								activatePauseItem();
							}
						}
						else if (ui.pauseMenuPage == PauseMenuPage::Training) {
							if (key->code == sf::Keyboard::Key::W || key->code == sf::Keyboard::Key::Up) {
								ui.trainingMenuIndex--;
								if (ui.trainingMenuIndex < 0) ui.trainingMenuIndex = 6;
							}
							else if (key->code == sf::Keyboard::Key::S || key->code == sf::Keyboard::Key::Down) {
								ui.trainingMenuIndex++;
								if (ui.trainingMenuIndex > 6) ui.trainingMenuIndex = 0;
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
					for (int i = 0; i < (int)ui.stageButtons.size(); i++) {
						if (ui.stageButtons[i].contains(mousePos)) {
							ui.stageSelectIndex = i;
							if (i == 0) selectedStage = StageID::Dojo;
							if (i == 1) selectedStage = StageID::HiddenEmber;
						}
					}
					refreshMenus();
				}
				else if (gameState == GameState::Playing && isPaused) {
					if (ui.pauseMenuPage == PauseMenuPage::Main) {
						int pauseButtonCount = (currentMode == GameMode::Training) ? 5 : 4;

						for (int i = 0; i < pauseButtonCount; i++) {
							if (ui.pauseButtons[i].contains(mousePos)) {
								ui.pauseMenuIndex = i;
								break;
							}
						}
					}
					else if (ui.pauseMenuPage == PauseMenuPage::Training) {
						for (int i = 0; i < (int)ui.trainingButtons.size(); i++) {
							if (ui.trainingButtons[i].contains(mousePos)) {
								ui.trainingMenuIndex = i;
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
						for (int i = 0; i < (int)ui.stageButtons.size(); i++) {
							if (ui.stageButtons[i].contains(mousePos)) {
								ui.stageSelectIndex = i;
								if (i == 0) selectedStage = StageID::Dojo;
								if (i == 1) selectedStage = StageID::HiddenEmber;
								activateStageSelectItem();
								break;
							}
						}
					}
					else if (gameState == GameState::Playing && isPaused) {
						if (ui.pauseMenuPage == PauseMenuPage::Main) {
							int pauseButtonCount = (currentMode == GameMode::Training) ? 5 : 4;

							for (int i = 0; i < pauseButtonCount; i++) {
								if (ui.pauseButtons[i].contains(mousePos)) {
									ui.pauseMenuIndex = i;
									activatePauseItem();
									break;
								}
							}
						}
						else if (ui.pauseMenuPage == PauseMenuPage::Training) {
							for (int i = 0; i < (int)ui.trainingButtons.size(); i++) {
								if (ui.trainingButtons[i].contains(mousePos)) {
									ui.trainingMenuIndex = i;
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
		// Start Screen render
		// -------------------------------
		if (gameState == GameState::StartScreen) {
			drawStartScreen(window, font);
			window.display();
			continue;
		}

		//------------------------------
		// Main Menu Render
		//------------------------------
		if (gameState == GameState::MainMenu) {
			drawSimpleVerticalMenu(window, font, "MAIN MENU", mainMenuOptions, mainMenuIndex);
			window.display();
			continue;
		}
		
		//-------------------------------
		// Mode Select render
		//-------------------------------
		if (gameState == GameState::ModeSelect) {
			drawSimpleVerticalMenu(window, font, "MODE SELECT", modeMenuOptions, modeMenuIndex);
			window.display();
			continue;
		}

		//---------------------------
		// Settings render
		//---------------------------
		if (gameState == GameState::Settings) {
			window.clear(sf::Color(20, 20, 30));

			sf::Text titleText(font);
			titleText.setString("SETTINGS");
			titleText.setCharacterSize(54);
			titleText.setFillColor(sf::Color::White);
			titleText.setPosition({ 80.f,70.f });

			sf::Text info(font);
			info.setString("Settings screen placeholder\nPress ENTER or ESCAPE to go back");
			info.setCharacterSize(28);
			info.setFillColor(sf::Color(220, 220, 220));
			info.setPosition({ 100.f,180.f });
			window.draw(titleText);
			window.draw(info);

			window.display();
			continue;
		}

		//----------------------------
		// Stage Select render
		//----------------------------
		if (gameState == GameState::StageSelect) {
			drawStageSelectMenu(window, title, subtitle, ui);
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


		//Stage layers
		drawStageBackground(window, stageVisuals);
		drawStageMidground(window, stageVisuals);
		drawStagePlatforms(window, platforms);


		bool trainingDebugAllowed = (currentMode == GameMode::Training);
		bool showStateTintsNow = trainingDebugAllowed && debug.showStateTints;

		player1.draw(window, showStateTintsNow);
		player2.draw(window, showStateTintsNow);

		drawStageForeground(window, stageVisuals);

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
			if (!debug.showStateOverlays) { return; }
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
			if (!debug.showHurtboxes) { return; }
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

		if (trainingDebugAllowed && debug.showHitboxes) {
			drawHitboxDebug(player1);
			drawHitboxDebug(player2);
		}

		if (trainingDebugAllowed && debug.showStateOverlays) {
			drawStateOverlays(player1);
			drawStateOverlays(player2);
		}

		if (trainingDebugAllowed && debug.showHurtboxes) {
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

		if (debug.showComboHUD && currentMode == GameMode::Training) {
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

		if (debug.showTimers && currentMode == GameMode::Training) {
			sf::Text timerDebug(font);
			timerDebug.setCharacterSize(18);
			timerDebug.setFillColor(sf::Color(220, 220, 220));
			timerDebug.setPosition({ 790.f, 70.f });

			timerDebug.setString(
				"P1 DodgeCD: " + fmt2(player1.dodgeCooldownTimer) +
				"\nP1 Invuln: " + fmt2(player1.spawnInvulnTimer) +
				"\nP1 Hitstop: " + fmt2(player1.hitstopTimer) +
				"\nP1 Fast Fall: " + std::string(player1.fastFalling ? "YES" : "NO") +
				"\n\nP2 DodgeCD: " + fmt2(player2.dodgeCooldownTimer) +
				"\nP2 Invuln: " + fmt2(player2.spawnInvulnTimer) +
				"\nP2 Hitstop: " + fmt2(player2.hitstopTimer) +
				"\nP2 Fast Fall: " + std::string(player2.fastFalling ? "YES" : "NO")
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

			drawPauseMenu(window, title, subtitle, ui,currentMode);
		}

		window.display();
	}

	return 0;
}
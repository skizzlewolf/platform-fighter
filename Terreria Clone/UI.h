#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "GameTypes.h"

struct MenuButton {
	sf::RectangleShape box;
	sf::Text label;

	MenuButton(const sf::Font& font, const sf::Vector2f& size);
	bool contains(const sf::Vector2f& mousePos) const;
};

enum class PauseMenuPage {
	Main,
	Training
};

struct DebugSettings {
	bool showHitboxes = true;
	bool showHurtboxes = false;
	bool showTimers = true;
	bool showComboHUD = true;
	bool showStateOverlays = true;
	bool showStateTints = true;
};

struct MenuUIState {
	PauseMenuPage pauseMenuPage = PauseMenuPage::Main;

	int stageSelectIndex = 0;
	int pauseMenuIndex = 0;
	int trainingMenuIndex = 0;

	std::vector<MenuButton> stageButtons;
	std::vector<MenuButton> pauseButtons;
	std::vector<MenuButton> trainingButtons;
};

void centerLabelInButton(MenuButton& b);

void makeVerticalButtons(
	std::vector<MenuButton>& buttons,
	const sf::Font& font,
	int count,
	sf::Vector2f start,
	sf::Vector2f size,
	float gap
);

void initializeMenus(MenuUIState& ui, const sf::Font& font);

void updateStageSelectMenu(
	sf::Text& title,
	sf::Text& subtitle,
	MenuUIState& ui,
	StageID selectedStage,
	GameMode selectedMode
);

void updatePauseMenu(
	sf::Text& title,
	sf::Text& subtitle,
	MenuUIState& ui,
	GameMode currentMode
);

void updateTrainingMenu(
	sf::Text& title,
	sf::Text& subtitle,
	MenuUIState& ui,
	const DebugSettings& debug
);

void updateAllMenus(
	sf::Text& title,
	sf::Text& subtitle,
	MenuUIState& ui,
	StageID selectedStage,
	GameMode selectedMode,
	GameMode currentMode,
	const DebugSettings& debug
);

void drawStageSelectMenu(
	sf::RenderWindow& window,
	const sf::Text& title,
	const sf::Text& subtitle,
	const MenuUIState& ui
);

void drawPauseMenu(
	sf::RenderWindow& window,
	const sf::Text& title,
	const sf::Text& subtitle,
	const MenuUIState& ui,
	GameMode currentMode
);
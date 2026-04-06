#include "UI.h"

MenuButton::MenuButton(const sf::Font& font, const sf::Vector2f& size)
	: box(size), label(font) {
	box.setFillColor(sf::Color(40, 40, 40));
	box.setOutlineThickness(3.f);
	box.setOutlineColor(sf::Color::White);

	label.setCharacterSize(28);
	label.setFillColor(sf::Color::White);
}

bool MenuButton::contains(const sf::Vector2f& mousePos) const {
	return box.getGlobalBounds().contains(mousePos);
}

void centerLabelInButton(MenuButton& b) {
	sf::FloatRect textBounds = b.label.getLocalBounds();
	sf::FloatRect boxBounds = b.box.getGlobalBounds();

	b.label.setPosition({
		boxBounds.position.x + (boxBounds.size.x - textBounds.size.x) * 0.5f - textBounds.position.x,
		boxBounds.position.y + (boxBounds.size.y - textBounds.size.y) * 0.5f - textBounds.position.y - 2.f
		});
}

void makeVerticalButtons(
	std::vector<MenuButton>& buttons,
	const sf::Font& font,
	int count,
	sf::Vector2f start,
	sf::Vector2f size,
	float gap
) {
	buttons.clear();

	for (int i = 0; i < count; i++) {
		MenuButton b(font, size);
		b.box.setPosition({ start.x, start.y + i * gap });
		buttons.push_back(std::move(b));
	}
}

void initializeMenus(MenuUIState& ui, const sf::Font& font) {
	makeVerticalButtons(ui.stageButtons, font, 4, { 320.f, 200.f }, { 440.f, 64.f }, 78.f);
	makeVerticalButtons(ui.pauseButtons, font, 5, { 320.f, 190.f }, { 440.f, 64.f }, 80.f);
	makeVerticalButtons(ui.trainingButtons, font, 7, { 320.f, 130.f }, { 440.f, 64.f }, 68.f);
}

void updateStageSelectMenu(
	sf::Text& title,
	sf::Text& subtitle,
	MenuUIState& ui,
	StageID selectedStage,
	GameMode selectedMode
) {
	title.setString("Select Stage");
	title.setPosition({ 360.f, 95.f });

	subtitle.setString("WASD / Arrows / Enter / Mouse");
	subtitle.setPosition({ 320.f, 150.f });

	ui.stageButtons[0].label.setString(
		std::string("Stage: Dojo") + (selectedStage == StageID::Dojo ? "   <" : "")
	);
	ui.stageButtons[1].label.setString(
		std::string("Stage: Kohona") + (selectedStage == StageID::Kohona ? "   <" : "")
	);
	ui.stageButtons[2].label.setString(
		std::string("Mode: ") + (selectedMode == GameMode::Training ? "Training" : "Versus")
	);
	ui.stageButtons[3].label.setString("Start Match");

	for (auto& b : ui.stageButtons) {
		centerLabelInButton(b);
	}
}

void updatePauseMenu(
	sf::Text& title,
	sf::Text& subtitle,
	MenuUIState& ui,
	GameMode currentMode
) {
	title.setString("Paused");
	title.setPosition({ 430.f, 95.f });

	subtitle.setString("WASD / Arrows / Enter / Mouse / Esc");
	subtitle.setPosition({ 255.f, 145.f });

	ui.pauseButtons[0].label.setString("Resume");
	ui.pauseButtons[1].label.setString("Restart Match");
	ui.pauseButtons[2].label.setString("Return to Stage Select");
	ui.pauseButtons[3].label.setString("Settings (Later)");

	if (currentMode == GameMode::Training) {
		ui.pauseButtons[4].label.setString("Training Settings");
	}
	else {
		ui.pauseButtons[4].label.setString("");
	}

	for (auto& b : ui.pauseButtons) {
		centerLabelInButton(b);
	}
}

void updateTrainingMenu(
	sf::Text& title,
	sf::Text& subtitle,
	MenuUIState& ui,
	const DebugSettings& debug
) {
	title.setString("Training Settings");
	title.setPosition({ 325.f, 90.f });

	subtitle.setString("Enter or A/D to toggle   |   Esc to go back");
	subtitle.setPosition({ 225.f, 140.f });

	ui.trainingButtons[0].label.setString(std::string("Show Hitboxes: ") + (debug.showHitboxes ? "ON" : "OFF"));
	ui.trainingButtons[1].label.setString(std::string("Show Hurtboxes: ") + (debug.showHurtboxes ? "ON" : "OFF"));
	ui.trainingButtons[2].label.setString(std::string("Show State Overlays: ") + (debug.showStateOverlays ? "ON" : "OFF"));
	ui.trainingButtons[3].label.setString(std::string("Show State Tints: ") + (debug.showStateTints ? "ON" : "OFF"));
	ui.trainingButtons[4].label.setString(std::string("Show Timers: ") + (debug.showTimers ? "ON" : "OFF"));
	ui.trainingButtons[5].label.setString(std::string("Show Combo HUD: ") + (debug.showComboHUD ? "ON" : "OFF"));
	ui.trainingButtons[6].label.setString("Back");

	for (auto& b : ui.trainingButtons) {
		centerLabelInButton(b);
	}
}

void updateAllMenus(
	sf::Text& title,
	sf::Text& subtitle,
	MenuUIState& ui,
	StageID selectedStage,
	GameMode selectedMode,
	GameMode currentMode,
	const DebugSettings& debug
) {
	updateStageSelectMenu(title, subtitle, ui, selectedStage, selectedMode);
	updatePauseMenu(title, subtitle, ui, currentMode);
	updateTrainingMenu(title, subtitle, ui, debug);
}

void drawStageSelectMenu(
	sf::RenderWindow& window,
	const sf::Text& title,
	const sf::Text& subtitle,
	const MenuUIState& ui
) {
	window.clear(sf::Color(12, 12, 12));
	window.setView(window.getDefaultView());

	window.draw(title);
	window.draw(subtitle);

	for (int i = 0; i < (int)ui.stageButtons.size(); i++) {
		bool selected = (i == ui.stageSelectIndex);

		MenuButton b = ui.stageButtons[i];
		b.box.setFillColor(selected ? sf::Color(70, 70, 110) : sf::Color(40, 40, 40));
		b.box.setOutlineColor(selected ? sf::Color::Yellow : sf::Color::White);

		window.draw(b.box);
		window.draw(b.label);
	}
}

void drawPauseMenu(
	sf::RenderWindow& window,
	const sf::Text& title,
	const sf::Text& subtitle,
	const MenuUIState& ui,
	GameMode currentMode
) {
	window.setView(window.getDefaultView());

	window.draw(title);
	window.draw(subtitle);

	if (ui.pauseMenuPage == PauseMenuPage::Main) {
		int pauseButtonCount = (currentMode == GameMode::Training) ? 5 : 4;

		for (int i = 0; i < pauseButtonCount; i++) {
			bool selected = (i == ui.pauseMenuIndex);

			MenuButton b = ui.pauseButtons[i];
			b.box.setFillColor(selected ? sf::Color(70, 70, 110) : sf::Color(40, 40, 40));
			b.box.setOutlineColor(selected ? sf::Color::Yellow : sf::Color::White);

			window.draw(b.box);
			window.draw(b.label);
		}
	}
	else {
		for (int i = 0; i < (int)ui.trainingButtons.size(); i++) {
			bool selected = (i == ui.trainingMenuIndex);

			MenuButton b = ui.trainingButtons[i];
			b.box.setFillColor(selected ? sf::Color(70, 70, 110) : sf::Color(40, 40, 40));
			b.box.setOutlineColor(selected ? sf::Color::Yellow : sf::Color::White);

			window.draw(b.box);
			window.draw(b.label);
		}
	}
}
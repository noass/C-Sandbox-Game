#include "imgui.h"
#include "imgui-SFML.h"
#include <SFML/Graphics.hpp>

#include <array>
#include <cmath>
#include <string>
#include <random>
#include <iostream>

sf::Color hsvToRgb(float h, float s, float v);

sf::Color hsvToRgb(float h, float s, float v) {
	int hi = static_cast<int>(h / 60.0f) % 6;
	float f = h / 60.0f - hi;
	float p = v * (1.0f - s);
	float q = v * (1.0f - f * s);
	float t = v * (1.0f - (1.0f - f) * s);

	switch (hi) {
	case 0: return sf::Color(v * 255, t * 255, p * 255);
	case 1: return sf::Color(q * 255, v * 255, p * 255);
	case 2: return sf::Color(p * 255, v * 255, t * 255);
	case 3: return sf::Color(p * 255, q * 255, v * 255);
	case 4: return sf::Color(t * 255, p * 255, v * 255);
	default: return sf::Color(v * 255, p * 255, q * 255);
	}
}

class World {
private:
	int width;
	int height;

	std::vector<std::vector<int>> privategrid;
	std::vector<std::vector<int>> grid;
public:
	bool colorChangingSand = true;
	int sandSize;
	float hue;
	int sandCount = 0;

	World(int gridSize, int sandSize) : width(gridSize), height(gridSize), sandSize(sandSize) {
		privategrid.resize(width, std::vector<int>(height, 0));
		grid.resize(width, std::vector<int>(height, 0));
	}

	int getWidth() const {
		return width;
	}

	int getHeight() const {
		return height;
	}

	void update() {
		const int w_minus = width-1;
		const int h_minus = height-2;

		for (int i = 0; i <= w_minus; ++i) {
			for (int j = 0; j <= h_minus; ++j) {
				if(!colorChangingSand && grid[i][j] > 0)
				{
					grid[i][j] = 60.0f;
				}
				if (privategrid[i][j] > 0) {
					if (privategrid[i][j + 1] == 0) {
						if (j <= h_minus) {
							grid[i][j] = 0;
							grid[i][j + 1] = hue;
						}
					}
					else {
						bool direction = std::rand() % 2 > 0;
						int next_i = direction ? i - 1 : i + 1;
						
						if ((next_i >= 0 && next_i <= w_minus) && privategrid[next_i][j + 1] == 0) 
						{
							grid[i][j] = 0;
							grid[next_i][j + 1] = hue;
						}
					}
				}
			}
		}
	}

	void draw(sf::RenderWindow& window) {
		for (int i = 0; i < width; ++i) {
			for (int j = 0; j < height; ++j) {
				if (grid[i][j] > 0) {
					sf::RectangleShape shape(sf::Vector2f(sandSize, sandSize));
					shape.setPosition(i * sandSize, j * sandSize);

					sf::Color color = hsvToRgb(grid[i][j], 1.0f, 1.0f);
					shape.setFillColor(color);

					if (hue >= 360.0f) {
						hue = 1.0f;
					}

					window.draw(shape);
				}
				privategrid[i][j] = grid[i][j];
			}
		}
	}

	void placeSand(int x, int y) {
		grid[x][y] = hue;
		//sandCount++;
	}

	void reset() {
		for (auto& row : privategrid) {
			std::fill(row.begin(), row.end(), 0);
		}
		for (auto& row : grid) {
			std::fill(row.begin(), row.end(), 0);
		}
		sandCount = 0;
	}
};

int WinMain() {
	int gridSize = 200;
	int sandSize = 5;

	float fps = 0.0f;
	sf::Clock FPSclock;
	sf::Time previousTime = FPSclock.getElapsedTime();
	sf::Time currentTime;

	sf::RenderWindow window(sf::VideoMode(gridSize * sandSize, gridSize * sandSize),
		"Sandbox Game", sf::Style::Close);

	ImGui::SFML::Init(window);
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigWindowsResizeFromEdges = false;
	bool guiInitSettingsSet = false;

	sf::Clock deltaClock;

	const float updateRate = 0.01f;

	float countdownMS = updateRate;

	World world(gridSize, sandSize);

	sf::Clock clock;

	int brushSize = 1;

	while (window.isOpen()) {
		window.clear();

		sf::Event event;

		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if ((!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered())  && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
				sf::Vector2i localPosition = sf::Mouse::getPosition(window);

				int gridX = std::floor(localPosition.x / world.sandSize);
				int gridY = std::floor(localPosition.y / world.sandSize);

				// Check if the calculated grid indices are within bounds
				if (gridX >= 0 && gridX <= world.getWidth() && gridY >= 0 && gridY <= world.getHeight()) {
					for (int x = -brushSize / 2; x <= brushSize / 2; ++x) {
						for (int y = -brushSize / 2; y <= brushSize / 2; ++y) {
							// Check if the calculated indices after applying brush size are within bounds
							if (gridX + x >= 0 && gridX + x < world.getWidth() && gridY + y >= 0 && gridY + y < world.getHeight()) 
							{
								world.placeSand(gridX + x, gridY + y);
							}
						}
					}
				}
				if (world.colorChangingSand)
				{
					world.hue += 0.1f;
				}
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
				world.reset();
			}
		}
		ImGui::SFML::Update(window, deltaClock.restart());
		ImGui::Begin("GUI", nullptr, ImGuiWindowFlags_NoResize);
		if (!guiInitSettingsSet)
		{ 
			ImGui::SetWindowPos(ImVec2(0, 0)); 
			ImGui::SetWindowSize(ImVec2(0, 0));
			guiInitSettingsSet = true;
		}
		ImGui::Text("FPS: %d", (int)fps);
		int windowHovered = 0;
		windowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) + ImGui::IsAnyItemHovered();
		if (windowHovered > 1) windowHovered = 1;
		ImGui::Text("Is GUI hovered: %d", windowHovered);
		//ImGui::Text("Sand count: %d", world.sandCount);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		if (ImGui::Button("ERASE ALL"))
		{
			world.reset();
		}
		ImGui::Checkbox("Color changing sand", &world.colorChangingSand);
		ImGui::SliderInt("Brush size", &brushSize, 1, 10, "%d");
		//ImGui::Separator();
		//ImGui::SliderInt("Grid size", &gridSize, 200, 1000, "%d");
		//ImGui::SliderInt("Sand size", &sandSize, 1, 8, "%d");
		ImGui::End();

		float sec = clock.restart().asSeconds();
		countdownMS -= sec;

		if (countdownMS < 0.0f) {
			world.update();
			countdownMS = updateRate;
		}

		world.draw(window);
		ImGui::SFML::Render(window);
		window.display();

		currentTime = FPSclock.getElapsedTime();
		fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds());
		previousTime = currentTime;
	}
	ImGui::SFML::Shutdown();
}

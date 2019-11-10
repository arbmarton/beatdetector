#pragma once

#include <glad/glad.h>

#include <array>

class Shader;

class SpectrogramBar
{
public:
	SpectrogramBar() = delete;
	SpectrogramBar(const float xPos, const float yPos, const float widthArg, const float heightArg);
	SpectrogramBar(const SpectrogramBar& rhs) = delete;
	SpectrogramBar(SpectrogramBar&& rhs) noexcept;
	SpectrogramBar& operator=(const SpectrogramBar& rhs) = delete;
	SpectrogramBar& operator=(SpectrogramBar&& rhs) noexcept;
	~SpectrogramBar();

	void draw() const;

private:
	float xCoordBottomLeft; // 0.0-1.0
	float yCoordBottomLeft; // 0.0-1.0
	float height; // 0.0-1.0
	float width; // 0.0-1.0

	std::array<float, 12> vertices;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	static Shader* shader;

	constexpr static unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};
};
#pragma once

namespace prp {
	class Renderer {
#pragma region RULE OF FIVE
	public:
		~Renderer() = delete;
		
		Renderer(Renderer&&) = delete;
		Renderer(const Renderer&) = delete;

		Renderer& operator=(Renderer&& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;

#pragma endregion
	};
}
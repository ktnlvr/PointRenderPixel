#pragma once

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")
#include <GLFW/glfw3.h>

namespace prp {
	class Renderer {
#pragma region PROPERTIES
	public:
		std::string title;

#pragma endregion

#pragma region SINGLETON
	public:
		static Renderer& GetInstance() {
			static Renderer instance;
			return instance;
		}

	protected:
		Renderer() {}

#pragma endregion

#pragma region RULE OF FIVE
	public:
		~Renderer() = default;
		
		Renderer(Renderer&&) = delete;
		Renderer(const Renderer&) = delete;

		Renderer& operator=(Renderer&& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;

#pragma endregion
	};
	typedef Renderer& RendererReference;
	inline auto GetRendererInstance() noexcept -> Renderer& { return Renderer::GetInstance(); }
}
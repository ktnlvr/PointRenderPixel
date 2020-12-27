#pragma once

#include <memory>

namespace prp {
	class Renderer : std::enable_shared_from_this<Renderer> {
#pragma region SINGLETON
	public:
		static std::shared_ptr<Renderer> GetInstance() noexcept {
			if (instance.get() == nullptr) return std::shared_ptr<Renderer>(new Renderer());
			else return instance;
		}

	protected:
		inline static std::shared_ptr<Renderer> instance;

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

	typedef std::shared_ptr<Renderer> SharedRenderer;

	inline auto GetRendererInstance() noexcept -> SharedRenderer { return Renderer::GetInstance(); }
}
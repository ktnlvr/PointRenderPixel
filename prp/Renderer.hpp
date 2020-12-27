#pragma once

#include <mutex>
#include <thread>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")
#include <GLFW/glfw3.h>

namespace prp {
	class Renderer {
#pragma region PROPERTIES
	public:
		

#pragma endregion

#pragma region SINGLETON
	public:
		static Renderer& GetInstance() {
			static Renderer instance;
			return instance;
		}

	protected:
		Renderer() {
			theThread = std::thread(TheThread);
		}

#pragma endregion

#pragma region RULE OF FIVE
	public:
		~Renderer() = default;
		
		Renderer(Renderer&&) = delete;
		Renderer(const Renderer&) = delete;

		Renderer& operator=(Renderer&& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;

#pragma endregion

#pragma region FLOW CONTROL
	public:
		static void TheThread() {
			auto& instance = Renderer::GetInstance();
			std::unique_lock<std::mutex> lock(instance.mtx);
			// Wait for Renderer::isRunning to be true
			instance.lock.wait(lock, []() { return Renderer::GetInstance().isRunning.load(); });
			// Free all the locks
			lock.unlock(); 
			instance.lock.notify_all();	
			
			// Actual renderer code
			printf("foo");
		}

		std::mutex mtx;
		std::condition_variable lock;
		std::atomic_bool isRunning;
		std::thread theThread;
#pragma endregion

#pragma METHODS
		void Start() {
			isRunning = true;
			lock.notify_all();
		}

#pragma endregion
	};
	typedef Renderer& RendererReference;
	inline auto GetRendererInstance() noexcept -> Renderer& { return Renderer::GetInstance(); }
}
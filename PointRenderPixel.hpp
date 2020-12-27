#pragma once

#define PRIME_RENDERER [](){ return glfwInit(); }
#define TERMINATE_RENDERER []() { return glfwTerminate(); }

#include <mutex>
#include <thread>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")
#include <GLFW/glfw3.h>

// look at Renderer::Start
#define JOIN_ON_START

namespace prp {
	class Renderer {
#pragma region PROPERTIES
	public:


#pragma endregion

#pragma region GLFW PROPERTIES
	public:
		GLFWwindow* glfw_window = nullptr;

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

			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
			instance.glfw_window = glfwCreateWindow(640, 480, "PointRenderPixel", NULL, NULL);

			std::unique_lock<std::mutex> lock(instance.mtx);
			// Wait for Renderer::isRunning to be true
			instance.lock.wait(lock, []() { return Renderer::GetInstance().isRunning.load(); });
			// Free all the locks
			lock.unlock();
			instance.lock.notify_all();
			
			GLFWwindow*& window = instance.glfw_window;
			glfwMakeContextCurrent(window);
			while (!glfwWindowShouldClose(window)) {
				glfwSwapBuffers(window);
				glfwPollEvents();
			}
			glfwDestroyWindow(window);
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
			// Make main thread to wait for this thread to join
#ifdef JOIN_ON_START
			theThread.join();
#endif
		}

#pragma endregion
	};
	typedef Renderer& RendererReference;
	inline auto GetRendererInstance() noexcept -> Renderer& { return Renderer::GetInstance(); }
}
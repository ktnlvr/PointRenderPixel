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
	struct vec2i { int x, y; };

	class Renderer {
#pragma region PROPERTIES
	private:
		std::string title = "PointRenderPixel";
		vec2i initialScreenPosition{ 33, 33 };
		// WARNING, window is measured in points, not in plain pixels'
		vec2i windowSize = { 256, 256 };
		int pointSize = 1;
		double deltaTime = 0;

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
			instance.glfw_window = glfwCreateWindow(instance.windowSize.x * instance.pointSize, instance.windowSize.y * instance.pointSize, instance.title.c_str(), NULL, NULL);

			if (!instance.glfw_window)
				throw std::exception("Invalid window, created");

			std::unique_lock<std::mutex> lock(instance.mtx);
			// Wait for Renderer::isRunning to be true
			instance.lock.wait(lock, []() { return Renderer::GetInstance().isRunning.load(); });
			// Free all the locks
			lock.unlock();
			instance.lock.notify_all();
			
			GLFWwindow*& window = instance.glfw_window;
			glfwMakeContextCurrent(window);

			// Used for measuring deltaTime
			double lastFrameTime = 0;
			while (!glfwWindowShouldClose(window)) {

				// Calculate deltaTime
				double currentFrame = glfwGetTime();
				instance.deltaTime = currentFrame - lastFrameTime;
				lastFrameTime = currentFrame;

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

#pragma region METHODS
		void Construct(const char* title, vec2i position, vec2i size, int pointSize) {
			this->title = title;
			this->windowSize = size;
			this->initialScreenPosition = position;
			this->pointSize = pointSize;
		}


		void Start() {
			isRunning = true;
			lock.notify_all();
			// Make main thread to wait for this thread to join
#ifdef JOIN_ON_START
			theThread.join();
#endif
		}

#pragma endregion

#pragma region WINDOWING 
		inline void SetWindowSize(vec2i size) {
			windowSize = size;
			glfwSetWindowSize(glfw_window, size.x * pointSize, size.y * pointSize);
		}

		inline void GetWindowSize(vec2i& out) {
			glfwGetWindowSize(glfw_window, &out.x, &out.y);
		}

		inline void SetWindowPosition(vec2i pos) {
			glfwSetWindowPos(glfw_window, pos.x, pos.y);
		}
		
		inline void GetWindowPosition(vec2i& out) {
			glfwGetWindowPos(glfw_window, &out.x, &out.y);
		}

		inline void SetWindowTitle(const char* str) {
			title = str;
			glfwSetWindowTitle(glfw_window, title.c_str());
		}

		inline void GetWindowTitle(std::string& out) {
			out = title;
		}

#pragma endregion
	};
	typedef Renderer& RendererReference;
	inline auto GetRendererInstance() noexcept -> Renderer& { return Renderer::GetInstance(); }
}
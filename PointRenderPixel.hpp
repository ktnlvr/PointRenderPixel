#pragma once

#define PRIME_RENDERER [](){ return glfwInit(); }
#define TERMINATE_RENDERER []() { return glfwTerminate(); }

#include <mutex>
#include <thread>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")
#include <GLFW/glfw3.h>

#ifndef POINT_SIZE
#define POINT_SIZE 4
#endif

namespace prp {
	struct vec2i { int x, y; };
	struct vec2f { float x, y; };

	class Renderer {
#pragma region PROPERTIES
	private:
		std::string title = "PointRenderPixel";
		vec2i initialScreenPosition{ 33, 33 };

		// WARNING, window is measured in points, not in plain pixels'
		vec2i windowSize = { 256, 256 };
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
			// Does something important
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

			instance.glfw_window = glfwCreateWindow(
				instance.windowSize.x * POINT_SIZE, 
				instance.windowSize.y * POINT_SIZE,
				instance.title.c_str(), NULL, NULL);

			// Probably will be replaced later, don't rely on it too much
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
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0, instance.windowSize.x, instance.windowSize.y, 0, 0, 1);

			instance.OnBeginCallback(instance);

			// Used for measuring deltaTime
			double lastFrameTime = 0;
			while (!glfwWindowShouldClose(window)) {
				glPointSize(POINT_SIZE);
				// Calculate deltaTime
				double currentFrame = glfwGetTime();
				instance.deltaTime = currentFrame - lastFrameTime;
				lastFrameTime = currentFrame;


				// Yes, all drawing is done as pixels
				// Yes, it is fine
				glBegin(GL_POINTS);
				instance.OnTickCallback(instance);
				glEnd();

				// do GLFW stuff, read more on their docs
				glfwSwapBuffers(window);
				glfwPollEvents();

				instance.OnTickLateCallback(instance);
			}
			instance.OnFinishCallback(instance);
			glfwDestroyWindow(window);
		}

		std::mutex mtx;
		std::condition_variable lock;
		std::atomic_bool isRunning;
		std::thread theThread;
#pragma endregion

#pragma region METHODS
		// Set default data for a console window
		void Construct(const char* title, vec2i position, vec2i size) {
			this->title = title;
			this->windowSize = size;
			this->initialScreenPosition = position;
		}

		// Start the engine thread! Yaaay!
		void Start() {
			isRunning = true;
			lock.notify_all();
			// Make main thread to wait for this thread to join
#ifdef JOIN_ON_START
			theThread.join();
#endif
		}

#pragma endregion

#pragma region CALLBACKS
	public:
		// Default Tick for running all the calculations
		void(*OnTickCallback)(Renderer& self) = [](Renderer& self) {};
		// After all other Ticks have fired
		void(*OnTickLateCallback)(Renderer& self) = [](Renderer& self) {};
		// Runs when the engine just starts
		void(*OnBeginCallback)(Renderer& self) = [](Renderer& self) {};
		// Called when the main loop is done, but before window destruction
		void(*OnFinishCallback)(Renderer& self) = [](Renderer& self) {};

#pragma endregion

#pragma region WINDOWING 
		inline void SetWindowSize(vec2i size) {
			windowSize = size;
			glfwSetWindowSize(glfw_window, size.x * POINT_SIZE, size.y * POINT_SIZE);
		}

		inline void GetWindowSize(vec2i& out) const {
			glfwGetWindowSize(glfw_window, &out.x, &out.y);
		}

		inline void SetWindowPosition(vec2i pos) {
			glfwSetWindowPos(glfw_window, pos.x, pos.y);
		}
		
		inline void GetWindowPosition(vec2i& out) const {
			glfwGetWindowPos(glfw_window, &out.x, &out.y);
		}

		inline void SetWindowTitle(const char* str) {
			title = str;
			glfwSetWindowTitle(glfw_window, title.c_str());
		}

		inline void GetWindowTitle(std::string& out) const {
			out = title;
		}

#pragma endregion

#pragma region DRAWING
public:
		void DrawPoint(vec2i pos) {
			// Still have to use float for 0.5 offset, IMO weird
			glVertex3f((float)pos.x + 0.5, (float)pos.y + 0.5, 0);
		} 

#pragma endregion
	};
	// Applied everyone, consider using it when storing Renderer 
	typedef Renderer& RendererReference;
	inline auto GetRendererInstance() noexcept -> Renderer& { return Renderer::GetInstance(); }
}
#pragma once

#define PRIME_RENDERER [](){ return glfwInit(); }
#define TERMINATE_RENDERER []() { return glfwTerminate(); }

#include <mutex>
#include <thread>
#include <functional>

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "opengl32.lib")
#include <glfw3.h>

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
		unsigned int glfw_frames = 0;
		unsigned int glfw_FPS = 0;

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
			glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);

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

			glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
				Renderer::GetInstance().cursorPosition = { (int)xpos / POINT_SIZE, (int)ypos / POINT_SIZE};
				});

			instance.OnBeginCallback(instance);

			// Used for measuring deltaTime
			double lastFrameTime = 0;
			double framesTimer = 0;
			glPointSize(POINT_SIZE);

			while (!glfwWindowShouldClose(window)) {
				glfwWaitEvents();
				glfwPollEvents();

				// Calculate deltaTime
				double currentFrame = glfwGetTime();
				instance.deltaTime = currentFrame - lastFrameTime;
				lastFrameTime = currentFrame;

				// Display fps
				instance.glfw_frames++;
				framesTimer += instance.deltaTime;

				if (framesTimer >= 1.0) {
					instance.glfw_FPS = instance.glfw_frames;
					instance.glfw_frames = 0;
					framesTimer = 0;
				}


				// Yes, all drawing is done as pixels
				// Yes, it is fine
				glBegin(GL_POINTS);
				instance.OnTickCallback(instance);
				glEnd();

				// do GLFW stuff, read more on their docs
				glFlush();

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
		std::function<void(Renderer&)> OnTickCallback = [](Renderer& self) {};
		// After all other Ticks have fired
		std::function<void(Renderer&)> OnTickLateCallback = [](Renderer& self) {};
		// Runs when the engine just starts
		std::function<void(Renderer&)> OnBeginCallback = [](Renderer& self) {};
		// Called when the main loop is done, but before window destruction
		std::function<void(Renderer&)> OnFinishCallback = [](Renderer& self) {};

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

		inline unsigned int FPS() const {
			return glfw_FPS;
		}

#pragma endregion

#pragma region DRAWING
public:
		void DrawPoint(vec2i pos) {
			// Still have to use float for 0.5 offset, IMO weird
			glVertex3f((float)pos.x + 0.5, (float)pos.y + 0.5, 0);
		}

		// Clear everything, starts with draw just for consistency
		inline void DrawClear() {
			glEnd();
			glClear(GL_COLOR_BUFFER_BIT);
			glBegin(GL_POINTS);
		}

		// Draw line with Bresenham's algorithm
		void DrawLine(vec2i pos0, vec2i pos1) {
			int x0 = pos0.x,
				y0 = pos0.y,
				x1 = pos1.x,
				y1 = pos1.y;

			int dx = abs(x1 - x0),
				dy = -abs(y1 - y0);

			int sx = x0 < x1 ? 1 : -1;
			int sy = y0 < y1 ? 1 : -1;

			int err = dx + dy;

			while (!(x0 == x1 && y0 == y1)) {
				DrawPoint({ x0, y0 });
				int e2 = 2 * err;
				if (e2 >= dy) {
					err += dy;
					x0 += sx;
				} 
				if (e2 <= dx) {
					err += dx;
					y0 += sy;
				}
			}
		}

		void DrawLine(vec2i, vec2i, vec2i...);

		void DrawRect(vec2i topleft, vec2i size) {
			vec2i topright = { topleft.x + size.x, topleft.y };
			vec2i bottomright = { topleft.x + size.x, topleft.y + size.y };
			vec2i bottomleft = { topleft.x, topleft.y + size.y };
			DrawLine(topleft, topright);
			DrawLine(topright, bottomright);
			DrawLine(bottomright, bottomleft);
			DrawLine(bottomleft, topleft);
		}

		// Fill rectangle on the screen
		void DrawRectFill(vec2i topleft, vec2i size) {
			vec2i s = { size.x / abs(size.x), size.y / abs(size.y) };
			for (int i = 0; abs(i) < size.x; i += s.x) {
				for (int j = 0; abs(j) < size.y; j += s.y) {
					DrawPoint({ i, j });
				}
			}
		}

		void DrawCircle(vec2i, int r);
		void DrawCircleFill(vec2i, int r);
		void DrawTri(vec2i, vec2i, vec2i);
		void DrawTriFill(vec2i, vec2i, vec2i);
		void DrawQuad(vec2i, vec2i, vec2i, vec2i);
		void DrawQuadFill(vec2i, vec2i, vec2i, vec2i);
		void DrawPolygon(vec2i...);
		void DrawPolygonFilled(vec2i...);
		void DrawPolygon(vec2i*, size_t);
		void DrawPolygonFilled(vec2i*, size_t);

#pragma endregion

#pragma region INPUT
public:
	vec2i cursorPosition = { 0, 0 };

#pragma endregion 
	};
	// Applied everyone, consider using it when storing Renderer 
	typedef Renderer& RendererReference;
	inline auto GetRendererInstance() noexcept -> Renderer& { return Renderer::GetInstance(); }
}
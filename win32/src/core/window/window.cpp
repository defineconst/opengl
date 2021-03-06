#include "window.h"
#include "../event/event.h"
#include "../timer/timer.h"
#include "../object/object.h"
#include "../render/renderer.h"
#include "../event/event_enum.h"

Window::Window()
    : _window(nullptr)
{
}

Window::~Window()
{
}

bool Window::Create(const std::string & title, int x, int y, uint w, uint h)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#if _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    _window = glfwCreateWindow(CW_DEFAULT, CW_DEFAULT, title.c_str(), nullptr, nullptr);
    if (nullptr == _window)
    {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(_window);
    glfwSetWindowUserPointer(_window, this);

    //  初始化GLEW
    if (GLEW_OK != glewInit())
    {
        _window = nullptr;
        glfwTerminate();
        return false;
    }
#if _DEBUG
    glDebugMessageCallback(Window::OnGLDebugProc, this);
#endif
    Move(x, y, w, h);
    return true;
}

void Window::Move(int x)
{
    Move(x, GetY(), GetW(), GetH());
}

void Window::Move(int x, int y)
{
    Move(x, y, GetW(), GetH());
}

void Window::Move(int x, int y, uint w)
{
    Move(x, y, w, GetH());
}

void Window::Move(int x, int y, uint w, uint h)
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    OnSize(_window, w, h);
    glfwSetWindowPos(_window, static_cast<int>(x), static_cast<int>(y));
    glfwSetWindowSize(_window, static_cast<int>(w), static_cast<int>(h));
}

void Window::SetFPS(size_t fps)
{
	_renderInfo.renderFPS = fps;
    //  渲染一帧需要的时间
    _renderInfo.renderTimeLast = 0.0f;
    _renderInfo.renderTimeStep = 1.0f / fps;
}

size_t Window::GetX() const
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    int x, y;
    glfwGetWindowPos(_window, &x, &y);
    return static_cast<size_t>(x);
}

size_t Window::GetY() const
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    int x, y;
    glfwGetWindowPos(_window, &x, &y);
    return static_cast<size_t>(y);
}

size_t Window::GetW() const
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    int w, h;
    glfwGetWindowSize(_window, &w, &h);
    return static_cast<size_t>(w);
}

size_t Window::GetH() const
{
    ASSERT_LOG(nullptr != _window, "_window Error");

    int w, h;
    glfwGetWindowSize(_window, &w, &h);
    return static_cast<size_t>(h);
}

void Window::Loop()
{
    glClearColor(0, 0, 0, 1);

    //  绑定Input
    glfwSetKeyCallback(_window, Window::OnKeybord);
    glfwSetCursorPosCallback(_window, Window::OnMouseMove);
    glfwSetWindowSizeCallback(_window, Window::OnSize);
    glfwSetMouseButtonCallback(_window, Window::OnMouseButton);
    glfwSetWindowCloseCallback(_window, Window::OnClose);
    while (!glfwWindowShouldClose(_window)) { Update(); }
    _window = nullptr;
}

void Window::OnGLError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message) const
{
    std::cout << SFormat("source: {0}, type: {1}, id: {2}, severity: {3}, length: {4}, message: {5}", source, type, id, severity, length, message) << std::endl;
}

void Window::Update()
{
    auto lasttime = time_tool::Now();
    auto difftime = -_renderInfo.renderTimeLast + lasttime;
    if ( difftime >= _renderInfo.renderTimeStep)
    {
        auto dt = difftime / _renderInfo.renderTimeStep;
        
        _renderInfo.renderTimeLast = lasttime;

		glfwPollEvents();
        
		Global::Ref().RefTimer().Update(lasttime);
        Global::Ref().RefObject().RootUpdate(dt);
        Global::Ref().RefRenderer().RenderOnce();
        glfwSwapBuffers(_window);

		std::cout <<
			SFormat("Err: {0}, FPS: {1} DiffTime: {2} RenderCount: {3} mVertexCount: {4}", 
					glGetError(), _renderInfo.renderFPS / dt, dt,
					Global::Ref().RefRenderer().GetRenderCount(),
					Global::Ref().RefRenderer().GetVertexCount())
			<< std::endl;
    }
}

void Window::OnMouseButton(GLFWwindow * window, int btn, int act, int stat)
{
    auto self = (Window *)glfwGetWindowUserPointer(window);
    EventMouseParam param;
    param.act               = act;
    param.btn               = btn;
    param.stat              = stat;
    param.x                 = self->_mouseInfo.x;
    param.y                 = self->_mouseInfo.y;
    param.dx                = 0;
    param.dy                = 0;
    self->_mouseInfo.act    = act;
    self->_mouseInfo.btn    = btn;
    self->_mouseInfo.stat   = stat;
    Global::Ref().RefEvent().Post(EventTypeEnum::kWINDOW_MOUSE_BUTTON, param);
}

void Window::OnMouseMove(GLFWwindow * window, double x, double y)
{
    auto self = (Window *)glfwGetWindowUserPointer(window);
    EventMouseParam param;
    param.act           = self->_mouseInfo.act;
    param.btn           = self->_mouseInfo.btn;
    param.stat          = self->_mouseInfo.stat;
    param.x             = (float)x;
    param.y             = (float)y;
    param.dx            = self->_mouseInfo.x - param.x;
    param.dy            = self->_mouseInfo.y - param.y;
    self->_mouseInfo.x  = param.x;
    self->_mouseInfo.y  = param.y;
    Global::Ref().RefEvent().Post(EventTypeEnum::kWINDOW_MOUSE_MOVEED, param);
}

void Window::OnKeybord(GLFWwindow * window, int key, int scan, int act, int stat)
{
    EventKeyParam param;
    param.key = key;
    param.act = act;
    param.stat = stat;
    Global::Ref().RefEvent().Post(EventTypeEnum::kWINDOW_KEYBOARD, param);
}

void Window::OnSize(GLFWwindow * window, int w, int h)
{
    glViewport(0, 0, w, h);
}

void Window::OnClose(GLFWwindow * window)
{
}

void GLAPIENTRY Window::OnGLDebugProc(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam)
{
    //switch (type)
    //{
    //case GL_DEBUG_TYPE_ERROR:
    //case GL_DEBUG_TYPE_PERFORMANCE:
    //case GL_DEBUG_TYPE_PORTABILITY:
    //case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    //case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    //    {
    //        auto window = reinterpret_cast<const Window *>(userParam);
    //        window->OnGLError(source, type, id, severity, length, message);
    //        ASSERT_LOG(false, "OpenGL Error.");
    //    }
    //    break;
    //}
}


#include <windows.h>
#include <GL/gl.h>
#include <cstdio>

void quad(float left, float right, float depth)
{
    glBegin(GL_QUADS);
    glVertex3f(left, -1, -depth); glVertex3f(right, -1, -depth);
    glVertex3f(right, 1, -depth); glVertex3f(left, 1, -depth);
    glEnd();
}

bool overlayDepthTest(bool writeDepth)
{
    glDepthMask(GL_TRUE);
    glClearColor(0, 0, 0, 1);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glFrustum(-.0004, .0004, -.0004, .0004, .01, 300);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glColor3f(0, 0, 1); quad(-1, 0, 24);
    glDepthMask(writeDepth ? GL_TRUE : GL_FALSE);
    glColor3f(1, 0, 0); quad(-1, 1, 25);
    glColor3f(0, 1, 0); quad(-1, 1, 25.0005f);
    unsigned char left[3] = {}, right[3] = {};
    glFinish();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(16, 32, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, left);
    glReadPixels(48, 32, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, right);
    glDepthMask(GL_TRUE);
    const bool occlusionPreserved = left[2] == 255 && left[0] == 0 && left[1] == 0;
    const bool painterLayerVisible = right[1] == 255 && right[0] == 0 && right[2] == 0;
    std::printf("overlay writeDepth=%d occlusion=%d foregroundGreen=%d rightRGB=(%u,%u,%u)\n",
        writeDepth, occlusionPreserved, painterLayerVisible, right[0], right[1], right[2]);
    return occlusionPreserved && (painterLayerVisible == !writeDepth);
}

int main()
{
    WNDCLASSA cls = {};
    cls.lpfnWndProc = DefWindowProcA;
    cls.hInstance = GetModuleHandleA(nullptr);
    cls.lpszClassName = "McEngineDepthProbe";
    RegisterClassA(&cls);
    HWND window = CreateWindowA(cls.lpszClassName, "", WS_OVERLAPPED, 0, 0, 64, 64, nullptr, nullptr, cls.hInstance, nullptr);
    HDC dc = GetDC(window);
    PIXELFORMATDESCRIPTOR format = {};
    format.nSize = sizeof(format); format.nVersion = 1;
    format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    format.iPixelType = PFD_TYPE_RGBA; format.cColorBits = 32; format.cDepthBits = 24;
    SetPixelFormat(dc, ChoosePixelFormat(dc, &format), &format);
    HGLRC context = wglCreateContext(dc); wglMakeCurrent(dc, context);
    using Generate = void(APIENTRY *)(GLsizei, GLuint *);
    using Bind = void(APIENTRY *)(GLenum, GLuint);
    using Storage = void(APIENTRY *)(GLenum, GLenum, GLsizei, GLsizei);
    using Multisample = void(APIENTRY *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei);
    using Query = void(APIENTRY *)(GLenum, GLenum, GLint *);
    auto generate = reinterpret_cast<Generate>(wglGetProcAddress("glGenRenderbuffers"));
    auto bind = reinterpret_cast<Bind>(wglGetProcAddress("glBindRenderbuffer"));
    auto storage = reinterpret_cast<Storage>(wglGetProcAddress("glRenderbufferStorage"));
    auto multi = reinterpret_cast<Multisample>(wglGetProcAddress("glRenderbufferStorageMultisample"));
    auto query = reinterpret_cast<Query>(wglGetProcAddress("glGetRenderbufferParameteriv"));
    if (!generate || !bind || !storage || !multi || !query) return 1;
    std::printf("renderer=%s version=%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
    const GLenum formats[] = {GL_DEPTH_COMPONENT, 0x81A6, 0x81A7, 0x8CAC};
    const int samples[] = {0, 2, 4};
    for (auto bits : formats) for (auto count : samples)
    {
        GLuint buffer; generate(1, &buffer); bind(0x8D41, buffer);
        while (glGetError() != GL_NO_ERROR) {}
        if (count) multi(0x8D41, count, bits, 64, 64); else storage(0x8D41, bits, 64, 64);
        GLenum error = glGetError();
        GLint actual = 0, depth = 0;
        query(0x8D41, 0x8D44, &actual); query(0x8D41, 0x8D54, &depth);
        std::printf("requested=%x samples=%d actual=%x depthBits=%d error=%x\n", bits, count, actual, depth, error);
    }
    glViewport(0, 0, 64, 64);
    const bool overlaysPassed = overlayDepthTest(true) && overlayDepthTest(false);
    wglMakeCurrent(nullptr, nullptr); wglDeleteContext(context); ReleaseDC(window, dc); DestroyWindow(window);
    return overlaysPassed ? 0 : 2;
}

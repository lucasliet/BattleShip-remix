// Phase 0 spike of the PS5 port: a native title that exercises what libultraship needs
// (exceptions, RTTI, shared_ptr, std::thread, thread_local, std::filesystem, the asm
// coroutine backend, a big heap, writable paths, AudioOut, the pad and an OpenGL 3.3 core
// context through the ps5-opengl SDK) and writes a receipt the lead reads back over FTP.
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "coroutine.h"

extern "C" {
int sceUserServiceInitialize(void*);
int sceUserServiceGetInitialUser(int*);
int sceUserServiceTerminate();
int scePadInit();
int scePadOpen(int, int, int, const void*);
int scePadReadState(int, void*);
int scePadClose(int);
int32_t sceAudioOutInit(void);
int32_t sceAudioOutOpen(int32_t userId, int32_t type, int32_t index, uint32_t len, uint32_t freq, uint32_t param);
int32_t sceAudioOutOutput(int32_t handle, const void* p);
int32_t sceAudioOutClose(int32_t handle);
int sceKernelUsleep(uint32_t microseconds);

// ps5-opengl app_heap.c: a title asks for a heap beyond its 128 MiB default here (direct memory).
const size_t ps5_opengl_heap_size = 1536u * 1024u * 1024u;

// coroutine_android.cpp reports yields to the watchdog; the spike has none.
void port_watchdog_note_yield(void) {}
}

namespace {

constexpr const char* kDataDir = "/data/BattleShip-Remix";
constexpr const char* kRom = "/data/BattleShip-Remix/baserom.remix.z64";
std::FILE* gReceipt = nullptr;

void report(const char* test, bool ok, const std::string& detail = "") {
    std::printf("[spike] %s: %s %s\n", test, ok ? "ok" : "FAIL", detail.c_str());
    if (gReceipt != nullptr) {
        std::fprintf(gReceipt, "%s: %s %s\n", test, ok ? "ok" : "FAIL", detail.c_str());
        std::fflush(gReceipt);
    }
}

struct Base { virtual ~Base() = default; };
struct Derived : Base { int value = 7; };

void testExceptions() {
    bool caught = false;
    try {
        throw std::runtime_error("boom");
    } catch (const std::exception& e) {
        caught = std::strcmp(e.what(), "boom") == 0;
    }
    report("exceptions", caught);
}

void testRtti() {
    std::unique_ptr<Base> base = std::make_unique<Derived>();
    auto* derived = dynamic_cast<Derived*>(base.get());
    report("rtti", derived != nullptr && derived->value == 7, typeid(*base).name());
}

void testSharedPtr() {
    auto shared = std::make_shared<std::vector<int>>(1000, 3);
    std::weak_ptr<std::vector<int>> weak = shared;
    bool ok = weak.lock() != nullptr && shared.use_count() == 1 && (*shared)[999] == 3;
    shared.reset();
    report("shared_ptr", ok && weak.expired());
}

thread_local int gThreadLocalCounter = 0;

void testThreads() {
    std::atomic<int> sum{0};
    std::mutex mutex;
    int guarded = 0;
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; i++) {
        workers.emplace_back([&] {
            for (int n = 0; n < 1000; n++) {
                sum++;
                gThreadLocalCounter++;
                std::lock_guard<std::mutex> lock(mutex);
                guarded++;
            }
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }
    report("threads", sum == 4000 && guarded == 4000 && gThreadLocalCounter == 0,
           "sum=" + std::to_string(sum.load()) + " tls_main=" + std::to_string(gThreadLocalCounter));
}

int gCoroutineSteps = 0;

void coroutineBody(void*) {
    for (int i = 0; i < 3; i++) {
        gCoroutineSteps++;
        port_coroutine_yield();
    }
}

void testCoroutines() {
    port_coroutine_init_main();
    PortCoroutine* co = port_coroutine_create(coroutineBody, nullptr, 256 * 1024);
    if (co == nullptr) {
        report("coroutines", false, "create failed");
        return;
    }
    int resumes = 0;
    while (!port_coroutine_is_finished(co) && resumes < 10) {
        port_coroutine_resume(co);
        resumes++;
    }
    port_coroutine_destroy(co);
    report("coroutines", gCoroutineSteps == 3 && resumes == 4, "steps=" + std::to_string(gCoroutineSteps));
}

bool writeReadBack(const std::string& path) {
    {
        std::FILE* out = std::fopen(path.c_str(), "wb");
        if (out == nullptr) return false;
        std::fputs("battleship", out);
        std::fclose(out);
    }
    char buf[16] = {};
    std::FILE* in = std::fopen(path.c_str(), "rb");
    if (in == nullptr) return false;
    size_t n = std::fread(buf, 1, sizeof(buf) - 1, in);
    std::fclose(in);
    std::remove(path.c_str());
    return n == 10 && std::strcmp(buf, "battleship") == 0;
}

void testFilesystem() {
    std::error_code ec;
    std::filesystem::create_directories(kDataDir, ec);
    bool dataDir = std::filesystem::is_directory(kDataDir, ec);
    report("fs_data_dir", dataDir, ec.message());
    report("fs_data_write", writeReadBack(std::string(kDataDir) + "/probe.txt"));
    report("fs_download0_write", writeReadBack("/download0/probe.txt"));
    report("fs_app0_param", std::filesystem::exists("/app0/sce_sys/param.json", ec));
}

void testBigHeap() {
    const size_t bytes = 512u * 1024u * 1024u;
    std::vector<uint8_t> block;
    try {
        block.resize(bytes, 0xA5);
    } catch (const std::bad_alloc&) {
        report("heap_512mb", false, "bad_alloc");
        return;
    }
    report("heap_512mb", block[bytes - 1] == 0xA5);
}

void testRomRead() {
    std::FILE* rom = std::fopen(kRom, "rb");
    if (rom == nullptr) {
        report("rom_read", true, "skipped: no ROM at /data/BattleShip-Remix");
        return;
    }
    auto start = std::chrono::steady_clock::now();
    std::vector<uint8_t> bytes(64u * 1024u * 1024u);
    size_t n = std::fread(bytes.data(), 1, bytes.size(), rom);
    std::fclose(rom);
    uint32_t sum = 0;
    for (size_t i = 0; i < n; i += 4096) sum += bytes[i];
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    report("rom_read", n == bytes.size(), "bytes=" + std::to_string(n) + " ms=" + std::to_string(ms) + " sum=" + std::to_string(sum));
}

void testAudio() {
    int32_t init = sceAudioOutInit();
    const uint32_t frames = 256;
    int32_t handle = sceAudioOutOpen(0xFF, 0, 0, frames, 48000, 1);
    if (handle < 1) {
        report("audio", false, "init=" + std::to_string(init) + " open=" + std::to_string(handle));
        return;
    }
    std::vector<int16_t> grain(frames * 2);
    double phase = 0.0;
    for (int g = 0; g < 48000 / (int)frames; g++) {
        for (uint32_t i = 0; i < frames; i++) {
            auto sample = (int16_t)(std::sin(phase) * 6000.0);
            grain[i * 2] = sample;
            grain[i * 2 + 1] = sample;
            phase += 2.0 * M_PI * 440.0 / 48000.0;
        }
        if (sceAudioOutOutput(handle, grain.data()) < 0) {
            report("audio", false, "output");
            sceAudioOutClose(handle);
            return;
        }
    }
    sceAudioOutOutput(handle, nullptr);
    report("audio", sceAudioOutClose(handle) == 0, "1 s sine at 440 Hz");
}

struct alignas(8) PadState {
    uint32_t buttons;
    unsigned char unused0[72];
    int32_t connected;
    unsigned char unused1[40];
};

int gPad = -1;

void openPad() {
    int user = 0;
    if (sceUserServiceInitialize(nullptr) != 0 || sceUserServiceGetInitialUser(&user) != 0 || scePadInit() < 0) {
        report("pad_open", false);
        return;
    }
    gPad = scePadOpen(user, 0, 0, nullptr);
    report("pad_open", gPad >= 0, "handle=" + std::to_string(gPad) + " user=" + std::to_string(user));
}

uint32_t readPadButtons() {
    PadState state{};
    if (gPad < 0 || scePadReadState(gPad, &state) < 0 || !state.connected) return 0;
    return state.buttons;
}

const char* kVertexShader = R"(#version 330 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec3 color;
out vec3 vColor;
void main() { vColor = color; gl_Position = vec4(pos, 0.0, 1.0); })";

const char* kFragmentShader = R"(#version 330 core
in vec3 vColor;
out vec4 frag;
uniform float phase;
void main() { frag = vec4(vColor * (0.6 + 0.4 * sin(phase)), 1.0); })";

GLuint compile(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        report("gl_shader", false, log);
    }
    return shader;
}

void testGraphics() {
    const EGLint configAttributes[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
                                       EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE};
    const EGLint contextAttributes[] = {EGL_CONTEXT_MAJOR_VERSION_KHR, 3, EGL_CONTEXT_MINOR_VERSION_KHR, 3,
                                        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR, EGL_NONE};
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLConfig config = nullptr;
    EGLint count = 0;
    if (display == EGL_NO_DISPLAY || !eglInitialize(display, nullptr, nullptr) || !eglBindAPI(EGL_OPENGL_API) ||
        !eglChooseConfig(display, configAttributes, &config, 1, &count) || count != 1) {
        report("gl_egl_init", false, "egl=0x" + std::to_string(eglGetError()));
        return;
    }
    EGLSurface surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)0, nullptr);
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    if (surface == EGL_NO_SURFACE || context == EGL_NO_CONTEXT || !eglMakeCurrent(display, surface, surface, context)) {
        report("gl_egl_context", false, "egl=0x" + std::to_string(eglGetError()));
        return;
    }
    EGLint width = 0, height = 0;
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
    report("gl_egl_context", true, std::string((const char*)glGetString(GL_VERSION)) + " / " +
                                       (const char*)glGetString(GL_RENDERER) + " " + std::to_string(width) + "x" +
                                       std::to_string(height));

    GLuint program = glCreateProgram();
    glAttachShader(program, compile(GL_VERTEX_SHADER, kVertexShader));
    glAttachShader(program, compile(GL_FRAGMENT_SHADER, kFragmentShader));
    glLinkProgram(program);
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    report("gl_program", linked != 0);

    const float vertices[] = {-0.6f, -0.5f, 1, 0, 0, 0.6f, -0.5f, 0, 1, 0, 0.0f, 0.6f, 0, 0, 1};
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 20, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 20, (void*)8);
    glUseProgram(program);
    GLint phaseLocation = glGetUniformLocation(program, "phase");
    eglSwapInterval(display, 1);

    auto start = std::chrono::steady_clock::now();
    int frames = 0;
    uint32_t buttonsSeen = 0;
    for (; frames < 600; frames++) {
        glViewport(0, 0, width, height);
        glClearColor(0.05f, 0.05f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUniform1f(phaseLocation, frames * 0.05f);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        if (!eglSwapBuffers(display, surface)) {
            report("gl_swap", false, "egl=0x" + std::to_string(eglGetError()));
            break;
        }
        buttonsSeen |= readPadButtons();
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    report("gl_frames", frames == 600 && glGetError() == GL_NO_ERROR,
           "frames=" + std::to_string(frames) + " ms=" + std::to_string(ms) + " fps=" +
               std::to_string(ms > 0 ? frames * 1000 / ms : 0));
    report("pad_buttons", true, "mask=0x" + std::to_string(buttonsSeen));

    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
}

} // namespace

int main() {
    gReceipt = std::fopen("/download0/battleship-spike.log", "w");
    std::printf("[spike] start\n");
    testExceptions();
    testRtti();
    testSharedPtr();
    testThreads();
    testCoroutines();
    testFilesystem();
    testBigHeap();
    testRomRead();
    openPad();
    testAudio();
    testGraphics();
    if (gPad >= 0) scePadClose(gPad);
    sceUserServiceTerminate();
    report("done", true);
    if (gReceipt != nullptr) std::fclose(gReceipt);
    std::FILE* copy = std::fopen("/data/BattleShip-Remix/spike.log", "w");
    if (copy != nullptr) {
        std::FILE* in = std::fopen("/download0/battleship-spike.log", "r");
        char line[512];
        while (in != nullptr && std::fgets(line, sizeof(line), in) != nullptr) std::fputs(line, copy);
        if (in != nullptr) std::fclose(in);
        std::fclose(copy);
    }
    return 0;
}

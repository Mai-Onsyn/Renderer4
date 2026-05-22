module;
export module Application;
import FramebufferWindow;
import Types;
import FrameBuffer;
import Renderer;
import CPU3DRenderer;

export class Application {
    FramebufferWindow* window;
    Renderer* renderer;
public:
    Application(const String& title, const Int32 width, const Int32 height) {
        window = new FramebufferWindow(width, height, title);
        renderer = new CPU3DRenderer();
    }
    ~Application() {
        delete window;
        delete renderer;
    }

    void run() {
        while (!window->shouldClose()) {

        }
    }
};
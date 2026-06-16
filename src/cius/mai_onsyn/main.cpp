#include <iostream>
#include <memory>

// #define USE_2D

#ifdef USE_3D
import Types;
import Application;
import Scene3D;
import Vertex;
import Transform;
import Mesh;
import Entity;
import Light;
import Logger;
import CPU3DRenderer;
import Scene;

void makeTestScene(Application<CPU3DRenderer>* app) {
    const auto task = makeSceneOperation<Scene3D>([](Scene3D* scene) {
        String localData = "Hello";
        Transform triangle{};
        Mesh trianglePiece;
        trianglePiece.vertices.emplace_back(Vertex{{-1.732, -1, 0}, {0, 0, -1}});
        trianglePiece.vertices.emplace_back(Vertex{{1.732, -1, 0}, {0, 0, -1}});
        trianglePiece.vertices.emplace_back(Vertex{{0, 2, 0}, {0, 0, -1}});
        trianglePiece.triangles.emplace_back(0, 1, 2);
        Entity testEntity{"Test Entity", move(trianglePiece), move(triangle)};
        scene->addEntity(move(testEntity));

        Light light{"World Light", {0, 100, 0}, {255, 255, 255, 255}};
        scene->addLight(move(light));

        Log::debug(scene->toString());
    });
    app->addSceneUpdate(task);
}

int main() {
    system("chcp 65001");
    Application<CPU3DRenderer> app("DisplayWindow", 1440, 900);
    makeTestScene(&app);
    app.run();

    return 0;
}

#endif

#ifdef USE_2D
import Application;
import CPU2DRenderer;
import Scene;
import Scene2D;
import Box;
import Text;
import Graphics2D;
import Color;
import Types;

void makeTestScene(Application<CPU2DRenderer>* app) {
    const auto task = makeSceneOperation<Scene2D>([](Scene2D* scene) {
        Box box{
            Graphics2D::Rect2D{{400, 400}, {200, 100}},
            Graphics2D::Alignment::Center,
            Color::Red,
            Color::Blue,
            1,
            0
        };

        Box p{
            Graphics2D::Rect2D{{400, 400}, {10, 10}},
            Graphics2D::Alignment::Center,
            Color::Blue,
            Color::Blue,
            1,
            0
        };

        Text text {
            "Ave Mujika",
            {150, 50},
            Graphics2D::Alignment::TopLeft,
            32,
            Color::Black
        };

        scene->addBox(move(box));
        scene->addBox(move(p));
        scene->addText(move(text));
    });
    app->addSceneUpdate(task);
}

int main() {
    system("chcp 65001");
    Application<CPU2DRenderer> app("DisplayWindow", 1440, 900);
    makeTestScene(&app);
    app.run();

    return 0;
}

#endif

#define LINK_SET

#ifdef LINK_SET
import LinkSetSceneDrawer;
import Application;
import CPU2DRenderer;
import Types;
import Thread;

int main() {
    system("chcp 65001");
    Application<CPU2DRenderer> app("链表集合运算演示", 1440, 900);



    app.run();

    return 0;
}

#endif

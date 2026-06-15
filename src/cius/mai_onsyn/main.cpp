#include <iostream>
#include <memory>

#define USE_2D

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

int main() {
    system("chcp 65001");
    Application<CPU2DRenderer> app("DisplayWindow", 1440, 900);
    app.run();

    return 0;
}

#endif
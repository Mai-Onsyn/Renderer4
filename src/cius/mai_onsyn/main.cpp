#include <iostream>
#include <memory>

import Application;
import Time;
import Types;
import Tile;
import MathUtil;
import ThreadPool;
import Thread;
import Logger;
import Scene;
import Mesh;
import Light;
import Transform;
import Entity;
import Vertex;
import Matrix;
import Color;

void makeTestScene(Application* app) {
    const auto task = makeSceneOperation([](Scene* scene) {
        String localData = "Hello";
        Transform triangle{};
        Mesh trianglePiece;
        trianglePiece.vertices.emplace_back(Vertex{{-0.747, -1, 4}, {0, 0, -1}});
        trianglePiece.vertices.emplace_back(Vertex{{0.747, -1, 4}, {0, 0, -1}});
        trianglePiece.vertices.emplace_back(Vertex{{0, 1, 4}, {0, 0, -1}});
        trianglePiece.triangles.emplace_back(0, 1, 2);
        Entity testEntity{"Test Entity", trianglePiece, triangle};
        scene->addEntity(move(testEntity));

        Light light{"World Light", {0, 100, 0}, {255, 255, 255, 255}};
        scene->addLight(move(light));

        Log::debug(scene->toString());
    });
    app->addSceneUpdate(task);
}

int main() {
    system("chcp 65001");
    Application app("DisplayWindow", 800, 600);
    makeTestScene(&app);
    app.run();

    return 0;
}
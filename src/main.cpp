#include <iostream>
#include <memory>

#define USE_3D

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
import Texture;
import Matrix;
import OBJ;
import SponzaPalaceScene;

void makeTestScene(Application<CPU3DRenderer<Scene3D>>* app) {
    const auto task = makeSceneOperation<Scene3D>([](Scene3D* scene) {
        String localData = "Hello";
        Transform triangle{};
        Mesh trianglePiece;
        UniquePtr<Texture> texture(new Texture("./assets/textures/uv_map_test.png"));;
        trianglePiece.vertices.emplace_back(Vertex{{-1, 35+1, 0}, {0, 0, -1}, {0, 0}});
        trianglePiece.vertices.emplace_back(Vertex{{-1, 35-1, 0}, {0, 0, -1}, {0, 1}});
        trianglePiece.vertices.emplace_back(Vertex{{1, 35-1, 0}, {0, 0, -1}, {1, 1}});
        trianglePiece.triangles.emplace_back(0, 2, 1, texture.get());
        trianglePiece.texture.push_back(move(texture));
        Entity testEntity{"Test Entity", move(trianglePiece), move(triangle)};
        scene->addEntity(move(testEntity));

        // Light light{"World Light Red", {20, 60, -30}, {255, 0, 0, 255}};
        // scene->addLight(move(light));
        // Light light2{"World Light Blue", {-20, 60, -30}, {0, 0, 255, 255}};
        // scene->addLight(move(light2));
        // Light light3{"World Light Green", {0, 60, -30}, {0, 255, 0, 255}};
        // scene->addLight(move(light3));

        Light whiteLight{"World Light White", {-30, 60, -30}, {255, 255, 255, 255}};
        scene->addLight(move(whiteLight));
        Light sponzaPalaceLight{"Sponza Palace Light", {-56, 2.5, 0}, {255, 255, 255, 255}};
        sponzaPalaceLight.a = 0.007;
        sponzaPalaceLight.b = 0.003;
        scene->addLight(move(sponzaPalaceLight));
    });
    app->addSceneUpdate(task);
    const auto task2 = makeSceneOperation<Scene3D>([](Scene3D* scene) {
        Transform trans{};
        Mesh mesh = OBJ::toMesh(OBJ::load("./assets/meshes/mika/mika test.obj"));
        Log::debug("Mesh Mika Llegal: %b", mesh.check());
        Entity testEntity{"Misono Mika", move(mesh), trans};
        scene->addEntity(move(testEntity));

        Mesh mesh2 = OBJ::toMesh(OBJ::load("./assets/meshes/the-utah-teapot/teapot.obj"));
        Log::debug("Mesh Teapot Llegal: %b", mesh2.check());
        Entity testEntity2{"Teapot", move(mesh2), Transform{Matrix4x4::scale(0.3) * Matrix4x4::translate({50, 0, 0})}};
        scene->addEntity(move(testEntity2));

        Mesh mesh3 = OBJ::toMesh(OBJ::load("./assets/meshes/Sponza Palace/scene.obj"));
        Log::debug("Mesh Teapot Llegal: %b", mesh3.check());
        Entity testEntity3{"Sponza Palace", move(mesh3), Transform{Matrix4x4::scale(1) * Matrix4x4::translate({-50, 0, 0})}};
        scene->addEntity(move(testEntity3));

        Log::debug(scene->toString());
    });
    app->addSceneUpdate(task2);
}

int main() {
    system("chcp 65001");
    Application<CPU3DRenderer<Scene3D>> app("DisplayWindow", 1440, 900);
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

// #define LINK_SET

#ifdef LINK_SET
#include <thread>
import LinkSetScene;
import Application;
import CPU2DRenderer;
import Types;
import Thread;
import Logger;
import Scene2D;
import InputManager;
import FramebufferWindow;


int main() {
    system("chcp 65001");
    Application<CPU2DRenderer<LinkSetScene>> app("链表集合运算演示", 1440, 900);
    app.run();
    return 0;
}

#endif

// #define LINK_SET_CONSOLE

#ifdef LINK_SET_CONSOLE
#include <string>
#include "MainPanel.hpp"
#include <conio.h>

int main() {
    system("chcp 65001");
    MainPanel mainPanel = MainPanel();
    mainPanel.show();
    while (true) {
        int choice =getChoice();
        choice = tolower(choice);
        switch (choice) {
            case 'q': {
                mainPanel.setSetA();
                mainPanel.show();
                break;
            }
            case 'w': {
                mainPanel.setSetB();
                mainPanel.show();
                break;
            }
            case'e':{
                mainPanel.indexOfSet();
                mainPanel.show();
                break;
            }
            case'r': {
                mainPanel.insertSet();
                mainPanel.show();
                break;
            }
            case 't': {
                mainPanel.intersectionSet();
                mainPanel.show();
                break;
            }
            case 'a': {
                mainPanel.unionSet();
                mainPanel.show();
                break;
            }
            case 's': {
                mainPanel.differenceSet();
                mainPanel.show();
                break;
            }
            case 'd': {
                mainPanel.symmetricDifference();
                mainPanel.show();
                break;
            }
            case 'f': {
                mainPanel.clearSet();
                mainPanel.show();
                break;
            }
            case 27: {
                return 0;
            }
            default: {

            }
        }
    }
}
#endif

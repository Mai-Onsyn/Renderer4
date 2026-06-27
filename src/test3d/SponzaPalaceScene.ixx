module;
export module SponzaPalaceScene;
import Scene3D;
import Types;
import InputManager;
import FramebufferWindow;
import Entity;
import Mesh;
import OBJ;
import Matrix;
import Transform;
import Logger;
import Light;

export class SponzaPalaceScene final : public Scene3D {
public:
    SponzaPalaceScene() {
        Mesh mesh3 = OBJ::toMesh(OBJ::load("./assets/meshes/Sponza Palace/scene.obj"));
        Log::debug("Mesh Teapot Llegal: %b", mesh3.check());
        Entity testEntity3{"Sponza Palace", move(mesh3), Transform{Matrix4x4::translate({0, 30, -8}) * Matrix4x4::rotate(3.1415 / 2, {0, 1, 0})}};
        this->addEntity(move(testEntity3));

        Light sponzaPalaceLight{"Sponza Palace Light", {-1.4, 42, -1.3}, {255, 255, 255, 255}};
        sponzaPalaceLight.a = 0.007;
        sponzaPalaceLight.b = 0.003;
        sponzaPalaceLight.intensity = 1.2;
        this->addLight(move(sponzaPalaceLight));
    }

    void update(InputManager* input, const Int32 tps, const Int32 windowWidth, const Int32 windowHeight, FramebufferWindow* window) override {
        Scene3D::update(input, tps, windowWidth, windowHeight, window);
    }
};
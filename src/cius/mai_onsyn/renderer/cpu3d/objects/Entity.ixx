module;
export module Entity;
import Mesh;
import Transform;
import Types;
import Format;
import Logger;

export struct Entity {
    String name;
    Mesh mesh;
    Transform transform;

    Entity(String name, Mesh mesh, Transform transform) : name(move(name)), mesh(move(mesh)), transform(move(transform)) {}

    String toString() const {
        return format("Entity{name=%s, mesh=%s, transform=%s}", name, mesh.toString(), transform.toString());
    }
};
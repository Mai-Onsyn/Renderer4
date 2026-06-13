module;
#include <algorithm>
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

    template<typename S, typename M, typename T>
    Entity(S&& name, M&& mesh, T&& transform)
        : name(std::forward<S>(name)),
          mesh(std::forward<M>(mesh)),
          transform(std::forward<T>(transform)) {}

    String toString() const {
        return format("Entity{name=%s, mesh=%s, transform=%s}", name, mesh.toString(), transform.toString());
    }
};
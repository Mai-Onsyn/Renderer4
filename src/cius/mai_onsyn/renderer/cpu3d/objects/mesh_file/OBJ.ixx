module;
#include <fstream>
#include <ranges>
#include <string>
#include <map>
#include <unordered_map>
#include <filesystem>
#include "json.hpp"
export module OBJ;
import Types;
import Color;
import Vectors;
import Functions;
import Mesh;
import Vertex;
import Triangle;
import Texture;
import Logger;
import Image;

using JSON = nlohmann::json;

export struct MtlObject {
    String name;
    Color Ka{64, 64, 64};           // 环境光颜色
    Color Kd{200, 200, 200};           // 漫反射颜色
    Color Ks{0, 0, 0};           // 镜面反射颜色
    Float Ns{1};           // 镜面反射指数
    Float d{1};            // 透明度
    // Color Tf;           // 滤光透射率
    // String map_Ka;      // 环境光纹理
    String map_Kd;      // 漫反射纹理
    String map_Ks;      // 镜面反射纹理
    // String map_Ns;      // 镜面反射高光纹理
    String map_d;       // 透明度纹理
    // String disp;        // 位移纹理
    // String decal;       // 贴花纹理
    String map_bump;    // 法线纹理
};

export struct OBJFace {
    List<UInt32> vertexIndices;
    List<UInt32> uvIndices;
    List<UInt32> normalIndices;
};

export struct FaceMtlRange {
    UInt32 start;
    UInt32 end;
    String mtlName;
};

export struct OBJFile {
    Map<String, MtlObject> mtls;
    List<Vector3D> vertices;
    List<Vector3D> normals;
    List<Vector2D> uvs;
    List<OBJFace> faces;
    List<FaceMtlRange> faceMtls;
};

struct IndexTuple {
    UInt32 v, uv, n;

    Boolean operator==(const IndexTuple& rhs) const {
        return v == rhs.v && uv == rhs.uv && n == rhs.n;
    }
};

struct IndexTupleHash {
    UInt64 operator()(const IndexTuple& t) const {
        UInt64 seed = 0;
        auto hash_combine = [](UInt64& seed, UInt32 val) {
            seed ^= std::hash<UInt32>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        };
        hash_combine(seed, t.v);
        hash_combine(seed, t.uv);
        hash_combine(seed, t.n);
        return seed;
    }
};

export namespace OBJ {
    UInt8 stocp(const String& str) {
        return static_cast<UInt8>(std::stof(str) * 255);
    }

    Map<String, MtlObject> loadMtl(const String& path) {
        std::filesystem::path mtlFileDir = std::filesystem::path(path).parent_path();

        Map<String, MtlObject> mtls;

        std::ifstream fis(path);
        if (!fis.is_open()) return mtls;

        String line;
        line.reserve(256);

        String currentMtlName;
        while (std::getline(fis, line)) {
            String trimmed = trim(line);
            if (trimmed.empty()) continue;
            // collapseSpaces(trimmed);

            List<String> tokens = split(trimmed, " ");
            if (tokens.empty()) continue;
            if (tokens[0] == "newmtl" && tokens.size() >= 2) {
                currentMtlName = trim(trimmed.substr(7));
                MtlObject mtl;
                mtl.name = currentMtlName;
                mtls[currentMtlName] = move(mtl);
            }
            else if (tokens[0] == "Ka" && tokens.size() >= 4) {
                mtls[currentMtlName].Ka = {stocp(tokens[1]), stocp(tokens[2]), stocp(tokens[3])};
            }
            else if (tokens[0] == "Kd" && tokens.size() >= 4) {
                mtls[currentMtlName].Kd = {stocp(tokens[1]), stocp(tokens[2]), stocp(tokens[3])};
            }
            else if (tokens[0] == "Ks" && tokens.size() >= 4) {
                mtls[currentMtlName].Ks = {stocp(tokens[1]), stocp(tokens[2]), stocp(tokens[3])};
            }
            else if (tokens[0] == "Ns" && tokens.size() >= 2) {
                mtls[currentMtlName].Ns = stof(tokens[1]);
            }
            else if (tokens[0] == "d" && tokens.size() >= 2) {
                mtls[currentMtlName].d = stof(tokens[1]);
            }
            else if (tokens[0] == "map_Kd" && tokens.size() >= 2) {
                mtls[currentMtlName].map_Kd = mtlFileDir.string() + '/' + trim(trimmed.substr(7));
            }
            else if (tokens[0] == "map_Ks" && tokens.size() >= 2) {
                mtls[currentMtlName].map_Ks = mtlFileDir.string() + '/' + trim(trimmed.substr(7));
            }
            else if (tokens[0] == "map_d" && tokens.size() >= 2) {
                mtls[currentMtlName].map_d = mtlFileDir.string() + '/' + trim(trimmed.substr(6));
            }
            else if (tokens[0] == "map_bump" && tokens.size() >= 2) {
                mtls[currentMtlName].map_bump = mtlFileDir.string() + '/' + trim(trimmed.substr(9));
            }
        }
        return mtls;
    }

    OBJFile load(const String& path) {
        std::filesystem::path objFileDir = std::filesystem::path(path).parent_path();
        std::filesystem::path configFile = objFileDir / "properties.json";

        Boolean flipUV_Y = false;
        if (std::filesystem::exists(configFile)) {
            JSON json = JSON::parse(std::ifstream(configFile));
            if (json.contains("flipUV_Y") && json["flipUV_Y"].is_boolean())
                flipUV_Y = json["flipUV_Y"].get<Boolean>();
        }

        OBJFile obj;
        std::ifstream fis(path);
        if (!fis.is_open()) return obj;

        String line;
        line.reserve(256);

        FaceMtlRange mtlRange;
        Boolean firstUseMtl = true;
        while (std::getline(fis, line)) {
            String trimmed = trim(line);
            if (trimmed.empty()) continue;
            collapseSpaces(trimmed);

            List<String> tokens = split(trimmed, " ");
            if (tokens.empty()) continue;
            const String& lineType = tokens[0];
            if (lineType == "v" && tokens.size() >= 4) {
                obj.vertices.emplace_back(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]));
            }
            else if (lineType == "vt" && tokens.size() >= 3) {
                Float v = stof(tokens[2]);
                obj.uvs.emplace_back(stof(tokens[1]), flipUV_Y ? 1.0f - v : v);
            }
            else if (lineType == "vn" && tokens.size() >= 4) {
                obj.normals.emplace_back(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]));
            }
            else if (lineType == "f") {
                OBJFace face;
                for (UInt8 i = 1; i < tokens.size(); i++) {
                    List<String> faceTokens = split(tokens[i], "/");
                    if (faceTokens.size() >= 1) face.vertexIndices.emplace_back(stoi(faceTokens[0]) - 1);
                    if (faceTokens.size() >= 2 && !trim(faceTokens[1]).empty()) face.uvIndices.emplace_back(stoi(faceTokens[1]) - 1);
                    if (faceTokens.size() >= 3 && !trim(faceTokens[2]).empty()) face.normalIndices.emplace_back(stoi(faceTokens[2]) - 1);
                }
                obj.faces.emplace_back(face);
            }
            else if (lineType == "mtllib" && tokens.size() >= 2) {
                obj.mtls.merge(loadMtl(objFileDir.string() + '/' + trim(trimmed.substr(7))));
            }
            else if (lineType == "usemtl" && tokens.size() >= 2) {
                if (firstUseMtl) {
                    mtlRange.start = obj.faces.size();
                    firstUseMtl = false;
                } else {
                    mtlRange.end = obj.faces.size();
                    obj.faceMtls.push_back(mtlRange);
                    mtlRange.start = obj.faces.size();
                }
                mtlRange.mtlName = trim(trimmed.substr(7));
            }
        }
        if (!firstUseMtl) {
            mtlRange.end = obj.faces.size();
            obj.faceMtls.push_back(mtlRange);
        }

        return obj;
    }

    Mesh toMesh(const OBJFile& obj) {
        Mesh mesh;

        Map<String, UInt32> mtlIndexes;
        UInt32 mtlWritePos = 0;
        Map<String, Image> loaded;
        for (const auto& [name, Ka, Kd, Ks, Ns, d, map_Kd, map_Ks, map_d, map_bump] : obj.mtls | std::views::values) {
            auto texture = make_unique<Texture>(Ka, Kd, Ks, Ns, d);
            if (!map_Kd.empty()) {
                if (!loaded.contains(map_Kd)) {
                    Image img = Image::fromFile(map_Kd);
                    loaded[map_Kd] = img;
                }
                texture->map_Kd = loaded[map_Kd];
            }
            if (!map_Ks.empty()) {
                if (!loaded.contains(map_Ks)) {
                    Image img = Image::fromFile(map_Ks);
                    loaded[map_Ks] = img;
                }
                texture->map_Ks = loaded[map_Ks];
            }
            if (!map_d.empty()) {
                if (!loaded.contains(map_d)) {
                    Image img = Image::fromFile(map_d);
                    loaded[map_d] = img;
                }
                texture->map_d = loaded[map_d];
            }
            if (!map_bump.empty()) {
                if (!loaded.contains(map_bump)) {
                    Image img = Image::fromFile(map_bump);
                    loaded[map_bump] = img;
                }
                texture->map_bump = loaded[map_bump];
            }
            mesh.texture.push_back(move(texture));
            mtlIndexes[name] = mtlWritePos++;
        }

        mesh.vertices.reserve(obj.vertices.size());
        mesh.triangles.reserve(obj.faces.size());
        std::unordered_map<IndexTuple, UInt32, IndexTupleHash> uniqueVertices;

        List<FaceMtlRange> ranges;
        if (obj.faceMtls.empty() && !obj.faces.empty()) {
            FaceMtlRange defaultRange;
            defaultRange.start = 0;
            defaultRange.end = obj.faces.size();
            defaultRange.mtlName = "";  // 空名称，后续查找材质时会失败，texture 为 nullptr
            ranges.push_back(defaultRange);

            mesh.texture.push_back(make_unique<Texture>());
            mesh.texture.back()->map_Kd = Image(1, 1, std::make_shared<UInt8[]>(4));
        } else {
            ranges = obj.faceMtls;  // 直接使用已有的范围
        }

        for (const auto& [start, end, mtlName] : ranges) {
            Texture* texture = nullptr;
            if (mtlIndexes.contains(mtlName)) {
                texture = mesh.texture[mtlIndexes[mtlName]].get();
            } else {
                texture = mesh.texture.back().get();
            }

            for (UInt32 i = start; i < end; i++) {
                const OBJFace& face = obj.faces[i];
                const UInt32 faceVertexCount = face.vertexIndices.size();

                List<UInt32> vertexIndices;
                vertexIndices.reserve(faceVertexCount);

                for (UInt32 j = 0; j < faceVertexCount; j++) {
                    IndexTuple tuple{
                        face.vertexIndices[j],
                        face.uvIndices.size() <= j ? 0 : face.uvIndices[j],
                        face.normalIndices.size() <= j ? 0 : face.normalIndices[j]
                    };

                    UInt32 tupleIndex;
                    if (auto it = uniqueVertices.find(tuple); it != uniqueVertices.end()) {
                        tupleIndex = it->second;
                    } else {
                        Vertex vertex;
                        vertex.pos = obj.vertices[tuple.v];

                        if (face.uvIndices.size() > j) {
                            vertex.uv = obj.uvs[tuple.uv];
                        } else {
                            vertex.uv = {0, 0};
                        }
                        if (face.normalIndices.size() > j) {
                            vertex.normal = obj.normals[tuple.n];
                        } else {
                            vertex.normal = {0, 1, 0};
                        }
                        mesh.vertices.push_back(vertex);
                        uniqueVertices[tuple] = mesh.vertices.size() - 1;
                        tupleIndex = mesh.vertices.size() - 1;
                    }
                    vertexIndices.push_back(tupleIndex);
                }

                for (UInt32 t = 1; t < faceVertexCount - 1; t++) {
                    Triangle triangle;
                    triangle.v1 = vertexIndices[0];
                    triangle.v2 = vertexIndices[t];
                    triangle.v3 = vertexIndices[t + 1];
                    triangle.texture = texture;
                    mesh.triangles.push_back(triangle);
                }
            }
        }

        return mesh;
    }
}
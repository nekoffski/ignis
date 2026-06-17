#pragma once

#include <vector>

#include "Vertex.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Buffer.hh"

namespace ignis::render {

struct Primitive {
    u32 firstVertex;
    u32 firstIndex;
    u32 indexCount;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Primitive> primitives;
};

struct Mesh {
    rhi::BufferHandle vertexBuffer;
    rhi::BufferHandle indexBuffer;
    std::vector<Primitive> primitives;
};

}  // namespace ignis::render

// #pragma once

// #include <functional>
// #include <vector>

// #include "RenderBundle.hh"
// #include "RenderGraph.hh"
// #include "ignis/core/Concepts.hh"
// #include "ignis/core/Core.hh"

// namespace ignis {

// // struct RenderPassDescription {};

// class RenderGraphLayout : public NonCopyable, public NonMovable {
//    public:
//     using ExecuteCallback = std::function<void()>;

//     class RenderPassLayout : public NonCopyable, public NonMovable {
//        public:
//         explicit RenderPassLayout(const Name& name) : m_name(name) {}

//         RenderPassLayout& access(ResourceAccess access, ResourceTag tag) {
//             return *this;
//         }

//        private:
//         Name m_name;
//         ExecuteCallback m_executeCallback;
//     };

//     explicit RenderGraphLayout() = default;

//     RenderPassLayout& addPass(const Name& name, ExecuteCallback callback) {
//         m_passes.push_back(std::make_unique<RenderPassLayout>(name));
//         // m_passes.back()->setCallback(std::move(callback));
//         return *m_passes.back();
//     }

//    private:
//     std::vector<std::unique_ptr<RenderPassLayout>> m_passes;
// };

// }  // namespace ignis

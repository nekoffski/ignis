#pragma once

#include <functional>
#include <vector>

#include "RBundle.hh"
#include "RGraph.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

// struct RenderPassDescription {};

class RGraphLayout : public NonCopyable, public NonMovable {
   public:
    using ExecuteCallback = std::function<void()>;

    class RPassLayout : public NonCopyable, public NonMovable {
       public:
        explicit RPassLayout(const Name& name) : m_name(name) {}

        RPassLayout& access(RAcess access, RResourceTag tag) { return *this; }

       private:
        Name m_name;
        ExecuteCallback m_executeCallback;
    };

    explicit RGraphLayout() = default;

    RPassLayout& addPass(const Name& name, ExecuteCallback callback) {
        m_passes.push_back(std::make_unique<RPassLayout>(name));
        // m_passes.back()->setCallback(std::move(callback));
        return *m_passes.back();
    }

   private:
    std::vector<std::unique_ptr<RPassLayout>> m_passes;
};

}  // namespace ignis

---
name: code-review
description: "Review Ignis C++ code for correctness, design, and architecture. Use when: reviewing git changes, inspecting a specific module, or auditing the full project. Supports targeted review by focus area: code quality, design, architecture, performance, safety."
argument-hint: "[git | <module> | project] [--focus code|design|architecture|performance|safety]"
---

# Ignis Code Review

## When to Use

Invoke `/code-review` when you want a structured review of:

- Your staged/unstaged **git changes** before committing
- A **specific module** (e.g. `rhi`, `renderer`, `core`, `asset`)
- The **entire project** for a broad audit

Optionally narrow the review to one or more focus areas with `--focus`.

## Parameters

| Argument          | Description                                                                |
| ----------------- | -------------------------------------------------------------------------- |
| `git`             | Review current git diff (staged + unstaged)                                |
| `<module>`        | Review a named module: `core`, `rhi`, `renderer`, `asset`, `engine`        |
| `project`         | Review the entire `src/ignis/` tree                                        |
| `--focus <areas>` | Comma-separated: `code`, `design`, `architecture`, `performance`, `safety` |

If no `--focus` is given, default to **all areas**: code quality + design + architecture.

## Procedure

### 1. Determine Scope

- **`git`** → run `git diff HEAD` and `git diff --cached` to gather changed files and their diffs.
- **`<module>`** → locate the module directory under `src/ignis/<module>/` and read all `.hh` / `.cpp` files.
- **`project`** → walk `src/ignis/` recursively; prioritise reading public headers (`.hh`) first, then implementations.

### 2. Load Context

Always read [project guidelines](../../copilot-instructions.md) before reviewing. Key rules to enforce:

- **Layer isolation**: Vulkan (`VkXxx`) must never appear outside `rhi/vk/`
- **Error handling**: No exceptions; use `Result<T>` / `Opt<Error>`; `log::panic` for unrecoverable states
- **Type aliases**: `u32`, `f32`, `Str`, etc. — never raw `uint32_t` / `float` / `std::string` in new code
- **Naming**: `m_camelCase` members, `camelCase()` methods, `PascalCase` classes, `.hh` headers
- **Resource safety**: GPU resources via typed handles only; no raw Vulkan handles above `rhi/vk/`
- **Non-copyable by default**: any class owning GPU state must inherit `NonCopyable` / `NonMovable`
- **Platform neutrality**: no platform-specific assumptions above `rhi/vk/`

### 3. Perform Review by Focus Area

Run only the requested focus areas (or all if none specified):

#### Code Quality (`code`)

- Correct use of type aliases and naming conventions
- No redundant copies; prefer references / handles
- Dead code, unreachable branches, unused variables
- Proper use of `Result<T>` / `Opt<Error>` — no swallowed errors
- Missing `NonCopyable` / `NonMovable` on owning types
- Keeping good modern C++ practices in mind (e.g., RAII, smart pointers if needed, no raw `new`)
- Consistent formatting and style
- Clear and descriptive naming
- Thread-safety where relevant (e.g., concurrent access to shared resources)
- Proper use of `const` and references to avoid unnecessary copying
- No platform-specific code or assumptions in cross-platform layers
- Proper encapsulation and minimal public API surface
- No hardcoded limits or magic numbers without explanation

#### Design (`design`)

- Single-responsibility; classes/functions not doing too much
- No unnecessary abstractions for one-off operations
- Builder / factory patterns used consistently (see `RenderGraphLayout`)
- Handle lifecycle respected (no dangling handles)
- `IGNIS_BIT_ENUM` used for flag enums; `checkFlag()` used consistently
- Clear ownership semantics for resources (who creates, who destroys)
- No circular dependencies between modules

#### Architecture (`architecture`)

- Layer violations: does code in `renderer/` touch `rhi/vk/`? does `rhi/` touch `renderer/`?
- Public API surface of each module — is `VkXxx` leaking?
- Dependency direction follows the 4-layer stack
- New files placed in the correct layer
- No platform-specific code above `rhi/vk/`
- Proper use of handles to abstract GPU resources; no raw pointers to GPU resources outside `rhi/vk/`
- Consistent use of `Result<T>` / `Opt<Error>` for error handling across layers

#### Performance (`performance`)

- Unnecessary heap allocations in hot paths
- Copies where moves suffice
- Pool / generational handles used instead of dynamic allocation for GPU resources
- No synchronous GPU waits in render-loop paths (flag for review)
- Efficient data structures for the use case (e.g., `std::vector` vs `std::list`)
- Proper use of `const` to enable compiler optimizations
- No redundant state changes or API calls in `rhi/vk/`
- Efficient algorithms for the task (e.g., O(n) vs O(n^2))
- Proper use of multithreading where applicable (e.g., resource loading)
- No busy-waiting or inefficient polling loops

#### Safety (`safety`)

- OWASP-relevant: no buffer overruns, no unchecked casts, no use-after-free (especially handle invalidation)
- `VK_ASSERT` used on every Vulkan call result inside `rhi/vk/`
- `log::expect` for precondition checks at system boundaries
- Proper ownership and lifecycle management of resources (no leaks, no double-frees)
- No raw pointers to GPU resources outside `rhi/vk/`
- Proper error handling with `Result<T>` / `Opt<Error>` — no ignored errors
- No unsafe casts or reinterpretations without justification
- Proper use of `const` to prevent unintended modifications
- Thread-safety considerations for shared resources (e.g., mutexes, atomic operations)
- No platform-specific code or assumptions that could lead to undefined behavior on certain platforms

### 4. Report

Structure your output as:

```
## Review: <scope> [focus: <areas>]

### Summary
<2-4 sentence overall assessment>

### Findings

#### 🔴 Critical
<issues that must be fixed — layer violations, memory safety, dropped errors>

#### 🟡 Warnings
<issues worth fixing — naming, missing NonCopyable, design concerns>

#### 🟢 Suggestions
<optional improvements — perf, style, consistency>

### Verdict
<Approve / Request changes / Needs discussion>
```

If the scope is large (full project), group findings by module.

# Modern real-time renderer techniques and an Ignis development roadmap

Research snapshot: 2026-08-09  
Scope: desktop Linux, Vulkan, initially headless  
Source policy: Vulkan specifications/guides/samples, GPU-vendor documentation, and papers or presentations from the original authors. Recommendations and Ignis-specific conclusions are explicitly identified as such.

## Executive conclusion

The most valuable "modern renderer" work for Ignis is not an advanced shading algorithm. It is a coherent execution foundation:

1. capability discovery, resource lifetime safety, frames in flight, suballocated memory, and GPU observability;
2. Vulkan 1.3 dynamic rendering, Synchronization 2, and timeline semaphores;
3. a real render-graph compiler that owns resource state transitions and transient lifetimes;
4. descriptor-indexing-based bindless resources;
5. compute culling plus `vkCmdDrawIndexedIndirectCount`;
6. a conventional clustered Forward+ or compact deferred renderer;
7. temporal features only after motion vectors and history resources are correct;
8. mesh shaders, visibility-buffer shading, ray tracing, virtual geometry, descriptor heaps, and device-generated commands as measured, capability-gated experiments.

This order matters. Bindless resource access is highly applicable to Ignis, but implementing it directly on top of the present handle, allocator, submission, and shader-layout systems would make resource lifetime bugs much harder to diagnose. Likewise, GPU-driven drawing depends on compute commands, indirect commands, explicit barriers, a GPU scene representation, and a binding model that lets many draws select their own data.

The recommended near-term Vulkan baseline remains **Vulkan 1.3 plus queried optional features**, not Vulkan 1.4 or Roadmap 2026. Ignis already requests Vulkan 1.3 in configuration, but logical-device creation currently enables only sampler anisotropy and does not build a `VkPhysicalDeviceFeatures2` capability chain ([configuration](../../cfg/ignis.toml), [bootstrap](../../src/ignis/rhi/vk/VKBootstrap.cpp)). Vulkan 1.3 made dynamic rendering and Synchronization 2 core, while timeline semaphores and descriptor indexing are core Vulkan 1.2 facilities ([Khronos version guide](https://docs.vulkan.org/guide/latest/versions.html)).

## Recommendation matrix

| Technique | State in 2026 | Value to Ignis | Prerequisites | Recommendation |
|---|---|---:|---|---|
| Dynamic rendering + Synchronization 2 + timeline semaphores | Mature/core | Very high | Feature negotiation, new command/submission vocabulary | **Build first** |
| Render graph with automatic barriers and transient lifetimes | Mature architecture pattern | Very high | Resource-use declarations, modern synchronization | **Build first** |
| Suballocated GPU memory + budget telemetry | Mature | Very high | Resource creation refactor | **Build first** |
| Validation, debug labels, GPU timestamps, deterministic headless tests | Mature | Very high | Query pools and test harness | **Build first** |
| Descriptor indexing / global bindless tables | Mature/core | High | Lifetime-safe slots, fallback descriptors, shader ABI | **Adopt after foundations** |
| Compute culling + multi-draw indirect count | Mature | High for large scenes | Compute dispatch, barriers, GPU scene buffers, bindless | **Primary GPU-driven path** |
| Clustered Forward+ | Mature | High for a general renderer | Depth, compute light lists, material model | **Recommended first lighting architecture** |
| Compact deferred shading | Mature | Medium-high | Multiple render targets, bandwidth budget | Keep as an alternative/experiment |
| Temporal AA/upscaling + dynamic resolution | Mature but integration-heavy | Medium-high | Motion vectors, jitter, history invalidation, exposure | Add after renderer correctness |
| Descriptor buffers | Ratified extension, optional | Medium | Buffer device address, allocator, feature fallback | Prototype only after descriptor indexing |
| Mesh shaders | Ratified extension, optional hardware path | Context-dependent | Meshlet asset pipeline and classic fallback | Defer; measure against MDI |
| Visibility-buffer shading | Mature idea, specialized architecture | Context-dependent | Bindless/BDA, material indirection, derivative handling | Defer until scene scale justifies it |
| Ray queries / ray-tracing pipelines | Mature optional Vulkan extensions | Context-dependent and costly | Acceleration structures, memory, denoising/temporal stack | Add as an optional hybrid feature |
| Virtual geometry / fine-grained mesh streaming | Proven in major engines, very high complexity | Low near-term | Almost every subsystem above | Research project, not roadmap baseline |
| `VK_EXT_descriptor_heap` | Newly ratified EXT in 2026 | Potentially high later | New drivers, shader/compiler/tool support, new ABI | **Watch; do not baseline yet** |
| `VK_EXT_device_generated_commands` / work-graph-style execution | Ratified/active frontier | Unclear before profiling | Mature GPU-driven renderer | **Watch; prefer MDI first** |

## What Ignis has today

Ignis has useful low-level pieces, but the high-level renderer is still a design shell:

- The RHI has typed handles, buffers, textures, shaders, descriptor-backed bind groups, graphics pipelines, explicit queue workloads, and direct/indexed draws ([device](../../src/ignis/rhi/Device.hh), [commands](../../src/ignis/rhi/Command.hh), [resource manager](../../src/ignis/rhi/ResourceManager.hh)).
- SPIR-V reflection already discovers stage inputs, descriptor bindings, and push constants, giving Ignis a good place to enforce a stable shader ABI ([shader parser](../../src/ignis/asset/ShaderParser.cpp)).
- `BufferUsage` already names indirect, shader-device-address, acceleration-structure, and descriptor-buffer uses, but the RHI has no dispatch, indirect draw, explicit barrier, or query commands yet ([buffer API](../../src/ignis/rhi/Buffer.hh), [command API](../../src/ignis/rhi/Command.hh)).
- Bind groups allocate conventional descriptor sets per shader layout and update one descriptor at a time; this is neither descriptor indexing nor a global bindless table ([bind group](../../src/ignis/rhi/vk/VKBindGroup.cpp), [descriptor pool](../../src/ignis/rhi/vk/VKResourceManager.cpp)).
- Every buffer and image owns a separate `vkAllocateMemory` allocation, which will not scale to a renderer with many transient or streamed resources ([buffer allocation](../../src/ignis/rhi/vk/VKBuffer.cpp), [image allocation](../../src/ignis/rhi/vk/VKTexture.cpp)).
- The submission model uses an 8-bit workload receipt, one semaphore and fence per pending workload, and explicit host waits; it is suitable for small demonstrations but not a multi-frame graph scheduler ([workload](../../src/ignis/rhi/Workload.hh), [device submission](../../src/ignis/rhi/vk/VKDevice.cpp), [synchronization types](../../src/ignis/rhi/vk/VKSync.hh)).
- Image transitions use the original barrier API and derive stage/access masks primarily from layouts. Real render-graph synchronization must instead derive them from the actual producer and consumer usages ([texture transitions](../../src/ignis/rhi/vk/VKTexture.cpp)).
- Graphics pipelines are monolithic and tied to `VkRenderPass`; `VK_NULL_HANDLE` is passed as the pipeline cache ([pipeline description](../../src/ignis/rhi/Pipeline.hh), [Vulkan pipeline](../../src/ignis/rhi/vk/VKPipeline.cpp)).
- `RenderGraph`, `FrameBundle`, and `Scene` are empty, while `RenderGraphLayout` records almost no pass state and its render callback receives only a `FrameBundle`, not a command encoder ([render graph](../../src/ignis/render/RenderGraph.hh), [layout](../../src/ignis/render/RenderGraphLayout.cpp), [frame bundle](../../src/ignis/render/FrameBundle.hh), [scene](../../src/ignis/render/Scene.hh)).
- Headless image readback already exists in the sandbox. That is a strong starting point for deterministic GPU integration tests ([shader sandbox](../../sandbox/shader.cpp), [render-pass sandbox](../../sandbox/renderpass.cpp)).

The architectural implication is that Ignis can evolve in place; it does not need a rewrite. The typed RHI and shader reflection are worth keeping. The resource lifetime, execution, render-graph, and shader-binding contracts need to become deeper before adding sophisticated passes.

### Recommended module seams

Keep the external `Renderer` interface close to its current small shape: compile a graph, enqueue frame data, and wait or query completion. Deepen the implementation behind that interface rather than adding public methods for barriers, descriptor allocation, queue ownership, transient pooling, or Vulkan features.

Four modules should carry most of the complexity:

1. **Render-graph compiler** — a pure, GPU-independent module that turns pass/resource declarations into an immutable execution plan: ordered passes, lifetimes, physical-resource requirements, barriers, and queue dependencies. Its interface is the primary test surface for graph correctness.
2. **Frame executor** — consumes a compiled plan and frame data, resolves resources, records commands, submits queue work, advances timelines, and retires deferred destruction. Frames-in-flight and synchronization remain implementation details.
3. **Resource table** — owns stable CPU handles, GPU-visible indices, fallback descriptors, descriptor updates, and timeline-delayed slot reuse. Scene and material code see resource IDs; descriptor-indexed sets, descriptor buffers, and descriptor heaps are interchangeable adapters behind this seam.
4. **Asset pipeline** — converts source glTF/images/shaders into validated runtime mesh, material, texture, and shader data. Runtime rendering should not know tinyobj, fastgltf, DXC invocation details, or source-file layouts.

The existing RHI remains the seam between renderer policy and Vulkan execution. Because Vulkan is currently its only adapter, avoid expanding the RHI with speculative cross-backend abstractions. Add backend-neutral concepts only when the renderer needs them, and keep Vulkan-specific capability structs and extension mechanisms inside `rhi/vk/`.

## 1. Modern Vulkan execution should be the foundation

### Dynamic rendering

Dynamic rendering replaces single-subpass `VkRenderPass` and framebuffer objects with `vkCmdBeginRendering`/`vkCmdEndRendering` and attachment descriptions supplied while recording. Khronos states that it offers similar performance to a single-pass render-pass object while simplifying the interface; it is core in Vulkan 1.3. It is not itself a synchronization command, so barriers remain the application's responsibility ([Khronos design proposal](https://docs.vulkan.org/features/latest/features/proposals/VK_KHR_dynamic_rendering.html), [Khronos announcement](https://www.khronos.org/blog/streamlining-render-passes)).

**Ignis fit:** excellent and immediate. `PipelineDescription` should use color/depth formats and sample counts rather than a `RenderPassHandle`; `CmdBeginRenderPass` should become a dynamic-rendering command whose attachments include view, load/store operations, clear value, and layout. Keep the public name "render pass" if it is useful domain language, but remove the Vulkan render-pass object from the backend-neutral contract. Dynamic rendering aligns directly with a render graph because the graph chooses physical attachments at execution time.

The exception is a future tile-GPU target that intentionally exploits subpasses/local reads. Vulkan 1.4's dynamic-rendering-local-read functionality fills much of that gap, but Ignis's present Linux desktop/headless target does not justify carrying both models now ([Khronos local-read sample](https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering_local_read/README.html)).

### Synchronization 2 and timeline semaphores

Synchronization 2 replaces overloaded stage/access APIs with `VkDependencyInfo` and `Vk*MemoryBarrier2`, and `vkQueueSubmit2` puts semaphore stage masks and values next to their waits/signals. Timeline semaphores carry monotonically increasing 64-bit values, support host query/wait/signal, and permit wait-before-signal; the Vulkan specification makes explicit that synchronization and memory visibility are application responsibilities ([Synchronization 2 guide](https://docs.vulkan.org/guide/latest/extensions/VK_KHR_synchronization2.html), [timeline semaphore sample](https://docs.vulkan.org/samples/latest/samples/extensions/timeline_semaphore/README.html), [synchronization specification](https://docs.vulkan.org/spec/latest/chapters/synchronization.html)).

**Ignis fit:** excellent and immediate. Replace `WorkloadReceipt = u8` with a structure such as `{QueueId, uint64_t value}` backed by one timeline per queue (or one carefully ordered global timeline). Use fences only to throttle CPU frames in flight when appropriate. Resource destruction and descriptor-slot reuse should be retired against the last timeline value that can reference them. The render graph should emit precise `VkImageMemoryBarrier2`/`VkBufferMemoryBarrier2` records from declared usages. Khronos's synchronization examples explicitly show the compute-write-to-indirect-draw and attachment-write-to-shader-read dependencies Ignis will need ([Khronos synchronization examples](https://docs.vulkan.org/guide/latest/synchronization_examples.html)).

Do not add asynchronous compute merely because a compute queue exists. Khronos's sample treats queue overlap as a performance experiment with platform-specific tradeoffs; the graph should make async scheduling possible, while profiling decides whether a pass moves off the graphics queue ([Khronos async-compute sample](https://docs.vulkan.org/samples/latest/samples/performance/async_compute/README.html)).

## 2. The render graph should own hazards, lifetimes, and scheduling

Frostbite's FrameGraph describes rendering as a graph of passes and resources so features remain decoupled while the engine can preserve execution efficiency ([original GDC session](https://www.gdcvault.com/play/1024612/FrameGraph-)). Vulkan requires explicit synchronization and warns that incorrect or overly broad synchronization can produce both correctness bugs and idle GPU time ([Khronos synchronization guide](https://docs.vulkan.org/guide/latest/synchronization.html)). These two concerns belong together: the graph has the global resource-use information needed to generate correct, narrow barriers.

**Recommended Ignis graph contract:** each pass declares, separately from its execution callback:

- queue class: graphics, compute, or transfer;
- reads and writes for every buffer/image, including access kind, shader stage or pipeline domain, subresource range, and required format/extent;
- attachment load/store/clear intent;
- whether a resource is transient, persistent history, or imported/exported;
- optional side effects that prevent dead-pass elimination.

Compilation should validate tags, reject cycles and read-before-produce errors, topologically order live passes, compute first/last use, allocate/alias compatible transient resources, choose layouts, insert Synchronization 2 barriers and queue ownership transfers, and produce queue timeline waits/signals. Execution callbacks should receive a restricted pass command encoder and resolved resource views. They should not be allowed to hide undeclared resource accesses.

This requires replacing the current callback `std::function<void(const FrameBundle&)>`, which cannot record RHI commands or resolve graph resources ([current layout](../../src/ignis/render/RenderGraphLayout.hh)). A useful split is:

```text
setup(builder)  -> declarations only -> compiled graph
execute(context, frame data)          -> commands using declared resources
```

Begin with correctness, visualization, and barrier logs. Transient aliasing and cross-queue scheduling are optimizations to add only after the compiled dependency model is testable.

## 3. "Bindless" has three distinct Vulkan implementation levels

### Descriptor indexing: the mature choice for Ignis

Descriptor indexing is commonly called bindless because shaders can index large descriptor arrays rather than rebinding individual resources for every draw. Its important facilities include runtime-sized arrays, partially bound arrays, update-after-bind, and non-uniform indexing. HLSL must use `NonUniformResourceIndex` when the index can vary between shader invocations ([Khronos descriptor-indexing sample](https://docs.vulkan.org/samples/latest/samples/extensions/descriptor_indexing/README.html)). It is core in Vulkan 1.2 ([Khronos version guide](https://docs.vulkan.org/guide/latest/versions.html)).

"Bindless" does not mean no API binding. A typical Vulkan implementation binds a small number of long-lived global descriptor sets, then passes compact texture/material/buffer indices in instance data or push constants. It moves complexity from per-draw binding to allocation, lifetime tracking, shader indexing, and validation. Khronos specifically notes that uninitialized or stale descriptors become harder to debug and recommends GPU-assisted validation for shader-time checks ([descriptor-indexing debugging guidance](https://docs.vulkan.org/samples/latest/samples/extensions/descriptor_indexing/README.html#debugging-descriptor-indexing)).

**Recommended Ignis design:** introduce backend-neutral `TextureId`, `BufferId`, and `SamplerId` values that resolve to slots in global descriptor arrays. A material stores indices, not `BindGroupHandle`s. Use a fallback texture/buffer for every unused slot. A slot is not recycled until all timeline values that could reference it have completed. Keep a small conventional set for frame/view data if that makes the shader ABI clearer. Support devices without the required descriptor-indexing feature bits through the existing bind-group path during the transition.

Ignis's existing handle contains an ID and generation, but the pool retrieves resources by raw ID and does not advance or validate generations ([handle](../../src/ignis/rhi/ResourceHandle.hh), [pool](../../src/ignis/core/Pool.hh)). That must be corrected before GPU-visible indices can safely outlive CPU objects.

### Descriptor buffers: optional follow-up, not the definition of bindless

`VK_EXT_descriptor_buffer` stores opaque descriptors in application-managed buffers and avoids descriptor pools, but retains descriptor-set layouts and set-offset binding. Implementations expose descriptor sizes and alignments, and GPU writes/copies to descriptor-buffer memory require explicit synchronization ([Khronos descriptor-buffer sample](https://docs.vulkan.org/samples/latest/samples/extensions/descriptor_buffer_basic/README.html), [Khronos descriptor-buffer guide](https://docs.vulkan.org/guide/latest/descriptor_buffer.html)).

Descriptor buffers can reduce CPU object-management overhead, but they are not required for a bindless renderer. For Ignis, descriptor indexing over conventional descriptor sets provides the shader and material architecture first, with much wider confidence. A later backend can implement the same high-level resource-index contract using descriptor buffers without changing scene/material code.

### `VK_EXT_descriptor_heap`: important 2026 direction, too early to baseline

Khronos released the newly ratified `VK_EXT_descriptor_heap` alongside Roadmap 2026. Unlike descriptor indexing, it replaces the descriptor-set model; unlike descriptor buffers, it removes set layouts and pipeline layouts from heap-based pipelines, exposes resource and sampler heaps, and supports direct shader heap access or mappings for existing set/binding-decorated shaders. Khronos says the EXT is intended to replace the old descriptor-set mechanism and is gathering developer feedback before a future KHR form ([Khronos 2026 announcement](https://www.khronos.org/blog/vulkan-introduces-roadmap-2026-and-new-descriptor-heap-extension), [extension design](https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_descriptor_heap.html), [ratified extension reference](https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_descriptor_heap.html)).

Roadmap 2026 itself targets newer mid/high-end devices shipping in 2026 or shortly thereafter, requires Vulkan 1.4, and raises many limits; it does **not** make descriptor heaps a universal core baseline ([Roadmap 2026 specification](https://docs.vulkan.org/spec/latest/appendices/roadmap.html)). The extension also requires new SPIR-V/compiler mappings, driver support, and capture/validation support. NVIDIA's Linux tooling added descriptor-heap capture/replay in Nsight Graphics 2026.2, illustrating that ecosystem rollout is happening now rather than being long established ([Nsight Graphics 2026.2 notes](https://developer.nvidia.com/nsight-graphics/get-started)).

**Ignis decision:** implement the engine-level bindless contract with core descriptor indexing now. Keep descriptor allocation behind an internal backend interface. Revisit descriptor heaps when at least the intended AMD, Intel, NVIDIA, and Mesa driver matrix plus DXC/SPIR-V reflection and RenderDoc/Nsight workflows have been verified in Ignis CI. Do not design current public APIs around descriptor sets, descriptor buffers, or descriptor heaps; expose only stable resource IDs.

## 4. GPU-driven rendering: mature indirect draws before frontier command generation

The mature GPU-driven pattern is:

```text
CPU uploads scene changes
        |
compute: frustum/LOD/occlusion culling
        |
visible-instance + indirect-command + draw-count buffers
        |
vkCmdDrawIndexedIndirectCount
```

Khronos's multi-draw-indirect sample demonstrates compute-generated draw data, GPU frustum culling, large shared vertex/index buffers, indexed texture arrays, and execution of many draws with one API command. It identifies reduced CPU command-generation and binding overhead as the principal benefits ([Khronos GPU rendering sample](https://docs.vulkan.org/samples/latest/samples/performance/multi_draw_indirect/README.html)). Vulkan's synchronization examples give the exact compute-write to indirect-read barrier required between the two stages ([Khronos synchronization examples](https://docs.vulkan.org/guide/latest/synchronization_examples.html)).

**Ignis fit:** high after the foundation phases. Add `CmdDispatch`, `CmdDrawIndirect`, `CmdDrawIndexedIndirect`, and count variants, plus buffer-range barriers and buffer device address support. Store geometry in large vertex/index arenas, transforms and bounds in an instance buffer, and materials as bindless indices. Start with frustum culling; add LOD selection; add a previous-frame hierarchical-Z occlusion pass only after measurements show a need. Preserve a CPU/direct-draw path for reference tests and small scenes.

### Device-generated commands and work graphs: watchlist

`VK_EXT_device_generated_commands` lets GPU data select more command state than ordinary indirect draws, including execution sets and tokenized command layouts. The Vulkan specification warns that when the layout contains only one action token, ordinary indirect draw/dispatch may be faster because DGC preprocessing has overhead ([DGC specification](https://docs.vulkan.org/spec/latest/chapters/device_generated_commands/generatedcommands.html)). The extension's stated target is device-driven scene management, including traversal, LOD, culling, and generated work ([DGC design](https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_device_generated_commands.html)). Work-graph-style shader enqueue systems push dynamic work creation further, but current Vulkan routes remain extension-led and architecture-sensitive; AMD's original 2025 research similarly presents indirect execution as the established baseline for dynamic GPU work generation ([AMD work-graphs paper](https://gpuopen.com/download/publications/Real-Time_Procedural_Generation_with_GPU_Work_Graphs-GPUOpen_preprint.pdf)).

**Ignis decision:** defer DGC and work graphs. If compute culling plus indirect-count draws later becomes limited by material/pipeline state changes, prototype DGC behind the same GPU-scene interface and compare preprocessing cost, frame time, driver coverage, and capture-tool quality. It should not shape the first GPU-driven implementation.

## 5. Choose a comprehensible first shading architecture

### Recommended default: clustered Forward+

Forward+ uses a compute pass to build lists of lights affecting screen regions, then a forward material pass shades using only the relevant list. The original AMD paper describes lower memory traffic than compute-based deferred lighting and avoids deferred shading's restrictions on material and lighting models ([Harada, McKee, and Yang](https://takahiroharada.wordpress.com/wp-content/uploads/2015/04/forward_plus.pdf)). Clustered shading partitions depth as well as screen space; the original paper reports better worst-case behavior than 2D tiled shading for problematic depth distributions and supports either forward or deferred shading ([Olsson, Billeter, and Assarsson](https://diglib.eg.org/items/6342d4d6-5220-4376-a5c6-a153058f4a3c)).

**Ignis fit:** strong. It exercises compute, the render graph, depth, storage buffers, and synchronization without requiring a large G-buffer or specialized geometry fetching. It supports transparent materials naturally and leaves room for MSAA. Build a minimal physically based material model, depth prepass only if measurement supports it, clustered light-list construction, opaque forward shading, shadow maps, transparency, and tone mapping.

### Deferred shading: still valid

A compact deferred path remains a good experiment when the renderer needs many screen-space effects or a uniform opaque material model. It increases render-target bandwidth and complicates MSAA/transparency, so it should be selected from target-content measurements rather than fashion. Dynamic rendering local reads can reduce off-chip traffic on supporting tile architectures, but that is outside the initial desktop focus ([Khronos local-read sample](https://docs.vulkan.org/samples/latest/samples/extensions/dynamic_rendering_local_read/README.html)).

### Visibility buffer: powerful but not a beginner deferred path

A visibility buffer stores primitive/instance identity, then reconstructs and shades only visible surfaces. The original paper proposed replacing a 20-plus-byte G-buffer with identifiers encoded in as few as four bytes, trading bandwidth for later triangle and attribute reconstruction ([Burns and Hunt](https://jcgt.org/published/0002/02/04/)). Epic's Nanite renderer uses a visibility-buffer stage as one component of a much larger virtualized-geometry system ([Epic's Nanite deep dive](https://advances.realtimerendering.com/s2021/Karis_Nanite_SIGGRAPH_Advances_2021_final.pdf)).

**Ignis fit:** defer. It becomes attractive with huge geometry, a bindless material system, buffer device addresses, packed mesh data, and expensive overdraw. It also introduces difficult derivative/LOD, alpha-tested material, and material-divergence problems. First implement the same scene through clustered Forward+; a visibility-buffer branch can later reuse the GPU scene and be compared fairly.

## 6. Temporal reconstruction and dynamic resolution are systems, not post-process toggles

Modern temporal upscalers reconstruct a display-resolution image from a lower-resolution color buffer plus history. AMD FSR 2 requires render-resolution color, depth, and motion vectors, with reactive mask and exposure inputs recommended for quality; it supports Vulkan and dynamic resolution. Its integration guide requires camera jitter, correct motion-vector space and scale, history reset on camera cuts, mip bias changes, and careful placement of post effects ([AMD FSR 2 source/integration guide](https://github.com/GPUOpen-Effects/FidelityFX-FSR2), [AMD product documentation](https://gpuopen.com/fidelityfx-superresolution-2/)). Intel XeSS likewise requires jitter and motion vectors, with depth when low-resolution motion vectors need internal dilation ([Intel XeSS developer guide](https://www.intel.com/content/www/us/en/developer/articles/technical/xess-sr-developer-guide.html)).

**Ignis fit:** valuable later, especially for expensive lighting or ray effects, but premature now. The renderer first needs:

- current and previous camera transforms;
- current and previous object/skinning transforms;
- stable per-object identities;
- a velocity target with a documented convention;
- jittered projection while retaining unjittered data where needed;
- persistent history resources represented explicitly in the render graph;
- resize, camera-cut, disocclusion, and exposure-change invalidation;
- per-pass resolution domains and a separation between render and display extents.

Implement a simple TAA reference first, with debug views for motion vectors, history weight, disocclusion, and reactive regions. Then integrate one open cross-vendor upscaler, preferably the current FidelityFX SDK successor rather than freezing the architecture to the older FSR 2 repository. Dynamic resolution should use delayed GPU timestamp feedback with hysteresis and bounded step changes, not instantaneous CPU frame time.

## 7. Mesh shaders and virtual geometry solve different-sized problems

`VK_EXT_mesh_shader` replaces the fixed pre-rasterization path with compute-like task and mesh stages that can cull, select LOD, decompress data, and emit a bounded number of primitives directly to the rasterizer. Limits and preferred output patterns are queried per device; the feature remains optional ([Khronos mesh-shader design](https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_mesh_shader.html), [Khronos culling sample](https://docs.vulkan.org/samples/latest/samples/extensions/mesh_shader_culling/README.html)). NVIDIA's original guidance emphasizes that mesh shaders are not universally faster than the traditional pipeline and that meshlet size, output size, occupancy, and task-shader overhead must be measured ([NVIDIA mesh-shader guidance](https://developer.nvidia.com/blog/using-mesh-shaders-for-professional-graphics/)).

**Ignis fit:** adopt meshlets before adopting mesh shaders. An offline meshlet format with bounds and LOD metadata benefits compute culling and conventional indexed draws too. Once GPU-driven MDI works, add `VK_EXT_mesh_shader` as an optional raster backend and compare it on multiple vendors. Always retain the vertex/index fallback.

Virtual geometry is a much larger system. Epic's Nanite presentation describes hierarchical clusters, fine-grained demand streaming, GPU culling, software and hardware rasterization paths, visibility buffering, and a virtual-memory-like geometry residency model ([Epic Nanite deep dive](https://advances.realtimerendering.com/s2021/Karis_Nanite_SIGGRAPH_Advances_2021_final.pdf)). It is not equivalent to enabling mesh shaders.

**Ignis decision:** do not put "Nanite-like virtual geometry" on the main development path. Revisit only after ordinary mesh streaming, meshlets, GPU LOD/culling, memory budgets, sparse/streaming infrastructure, and representative massive content all exist. A bounded experiment should answer one question, such as whether cluster-granular streaming beats mesh-granular streaming for a chosen dataset.

## 8. Ray tracing should start as a hybrid query

Vulkan's cross-vendor ray-tracing family separates acceleration structures, ray queries, and ray-tracing pipelines. Ray queries can run inside graphics or compute shaders and reuse conventional pipeline/resource systems; full ray-tracing pipelines add ray-generation/hit/miss stages and shader binding tables. Applications own BLAS/TLAS building, scratch memory, updates, synchronization, and residency ([Khronos ray-tracing guide](https://docs.vulkan.org/guide/latest/extensions/ray_tracing.html), [acceleration-structure specification](https://docs.vulkan.org/spec/latest/chapters/accelstructures.html), [basic ray-tracing sample](https://docs.vulkan.org/samples/latest/samples/extensions/ray_tracing_basic/README.html)).

**Ignis fit:** optional and late. Begin with ray-query shadows or ambient occlusion in an existing compute/fragment path. This tests shared scene geometry, BLAS/TLAS lifetime, descriptor integration, and graph synchronization without introducing a second pipeline model. A full reflection/GI/path-tracing renderer also needs stochastic sampling, accumulation, denoising, motion vectors, and strong temporal handling. Advanced sampling such as ReSTIR is active and proven research, but it compounds those prerequisites rather than replacing them ([original ReSTIR paper](https://research.nvidia.com/sites/default/files/pubs/2020-07_Spatiotemporal-reservoir-resampling/ReSTIR.pdf)).

## 9. Pipeline and shader management must prevent runtime hitches

Vulkan pipeline creation can compile shaders and cause visible frame-time spikes. Khronos recommends hashing reusable state, creating known pipelines early, persisting `VkPipelineCache` data between runs, and avoiding uncached creation at draw time ([Khronos pipeline-management sample](https://docs.vulkan.org/samples/latest/samples/performance/pipeline_cache/README.html)). `VK_KHR_pipeline_binary` gives applications explicit keys and binary blobs for their own cache infrastructure, while graphics pipeline libraries and shader objects offer more flexible compilation/state models on supporting implementations ([pipeline-binary design](https://docs.vulkan.org/features/latest/features/proposals/VK_KHR_pipeline_binary.html), [graphics-pipeline-library design](https://docs.vulkan.org/features/latest/features/proposals/VK_EXT_graphics_pipeline_library.html), [shader-object sample](https://docs.vulkan.org/samples/latest/samples/extensions/shader_object/README.html)).

**Recommended Ignis sequence:** 

1. Define a canonical shader/pipeline key: source/content hash, compiler version/options, entry points, specialization values, resource-layout ABI version, attachment formats, raster/depth/blend state, and device/driver cache identity.
2. Persist a normal `VkPipelineCache`, warm known pipelines off the render path, expose pipeline-creation feedback/timings, and use an explicit fallback/error pipeline.
3. Make reflection validation a build/load error: stage interfaces, descriptor classes/counts, push-constant overlap, and vertex layout must agree.
4. Keep HLSL + DXC + SPIR-V while it works. A language/toolchain migration is not a renderer feature.
5. Evaluate pipeline binaries/libraries or shader objects only after actual compile-time or permutation measurements justify the extra backend paths.

Dynamic rendering is particularly valuable here because it removes `RenderPassHandle` identity from pipeline compatibility and replaces it with attachment formats. Bindless materials also reduce pipeline permutations by moving resource selection into data.

## 10. Memory allocation and residency are prerequisite infrastructure

Vulkan treats suballocation as a first-class approach; OS/driver allocations are slow and devices expose a maximum allocation count. The Khronos guide explicitly suggests using Vulkan Memory Allocator rather than writing this difficult subsystem from scratch ([Khronos allocation guide](https://docs.vulkan.org/guide/latest/memory_allocation.html)). AMD's maintained VMA supports block suballocation, persistent mapping, non-coherent flush/invalidate handling, aliasing, custom pools, buffer device address, memory budgets, statistics, and debugging ([official VMA repository](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)). `VK_EXT_memory_budget` reports estimated per-process heap usage and a changing budget that applications can use before allocation failures or performance degradation ([extension reference](https://docs.vulkan.org/refpages/latest/refpages/source/VK_EXT_memory_budget.html)). NVIDIA notes that Linux does not provide the same automatic video-memory overcommit paging behavior as Windows, making explicit budget discipline especially important for Ignis's target ([NVIDIA Vulkan guidance](https://developer.nvidia.com/blog/vulkan-dos-donts/)).

**Ignis decision:** integrate VMA behind the existing resource wrappers unless writing an allocator is itself a project goal. Create:

- persistently mapped per-frame upload/readback rings;
- device-local arenas for static geometry and GPU scene data;
- transient image/buffer pools keyed by memory requirements and compatibility;
- dedicated allocations only where queried/recommended;
- memory budget and high-watermark telemetry;
- timeline-deferred destruction queues.

The render graph can later alias non-overlapping transient resources through VMA virtual blocks or equivalent allocation metadata. Do not implement sparse residency or eviction before ordinary suballocation and budget reporting are stable.

## 11. Testing and profiling are renderer features

Khronos calls the validation layer the first line of defense and exposes synchronization validation, GPU-assisted validation, shader debug printf, and best-practices checks. `VK_EXT_debug_utils` names objects and marks command regions for capture tools ([Khronos development-tools guide](https://docs.vulkan.org/guide/latest/development_tools.html), [debug-utils sample](https://docs.vulkan.org/samples/latest/samples/extensions/debug_utils/README.html)). Vulkan timestamp queries measure approximate GPU intervals and must be interpreted with queue/stage and `timestampPeriod` constraints ([Khronos timestamp sample](https://docs.vulkan.org/samples/latest/samples/api/timestamp_queries/README.html)). Nsight Systems supports Vulkan tracing on Linux and correlates API calls, GPU workloads, frame duration, and debug labels; Nsight Graphics provides frame capture, shader profiling/debugging, and ray-tracing inspection on supported hardware ([Nsight Systems Vulkan guide](https://developer.nvidia.com/blog/nsight-systems-adds-vulkan-support/), [Nsight Graphics](https://developer.nvidia.com/nsight-graphics)).

**Recommended Ignis quality gates:** 

- Every GPU object and graph pass gets a stable debug name/label.
- Debug CI runs core, synchronization, and best-practices validation; scheduled/targeted runs add GPU-assisted validation because instrumentation is expensive.
- Unit tests cover graph cycle detection, lifetime intervals, barrier synthesis, alias compatibility, descriptor slot retirement, and pipeline-key stability without a GPU.
- Headless integration tests render fixed scenes to images and compare tolerance-aware metrics plus selected exact buffers (IDs, depth, motion, indirect commands). Keep deterministic camera paths and seeds.
- GPU timestamps surround graph passes and queue submissions; record CPU submission time, draw/triangle/dispatch counts, transient bytes, descriptor occupancy, upload bytes, and memory budget.
- Establish benchmark scenes for many objects, many lights, overdraw, alpha-tested foliage, streaming, and shader/pipeline warmup. A technique graduates from experimental only with cross-vendor measurements.

## Concrete development plan

### Phase 0 — make the baseline trustworthy

**Deliverables**

- Add automated headless tests for buffer/texture transfers, clear, indexed draw, and one textured mesh.
- Fix handle generations and double-destroy/underflow behavior; distinguish pool capacity from live size.
- Add debug names/labels, synchronization validation, GPU-assisted validation mode, timestamp query support, and a minimal per-frame metrics dump.
- Define supported Linux GPU/driver targets and record their features/limits at startup.

**Exit criteria:** repeated headless runs are deterministic; stale handles fail safely; validation is clean; a captured frame has named resources/passes; CPU and GPU timings are visible.

### Phase 1 — modern Vulkan RHI

**Deliverables**

- Query features/properties with `vkGetPhysicalDeviceFeatures2`/`vkGetPhysicalDeviceProperties2`; negotiate required versus optional capabilities explicitly.
- Add compute pipelines and commands, dispatch, indirect draw/count, buffer/image copy regions, Synchronization 2 barriers, timestamps, and dynamic rendering.
- Replace render-pass-coupled pipelines with attachment-format/sample descriptions.
- Replace 8-bit workload receipts with queue timeline values; add two or three frames in flight and timeline-retired destruction.
- Integrate VMA and upload/readback rings; expose memory budgets.

**Exit criteria:** a headless sample executes transfer -> compute -> indirect graphics -> readback through timeline dependencies without queue-idle calls, with validation clean.

### Phase 2 — compile and execute the render graph

**Deliverables**

- Redesign `RenderGraphLayout` around setup and execute callbacks.
- Add logical transient, imported, exported, and history buffers/images.
- Compile DAG order, validate uses, cull dead passes, derive lifetimes, layouts, barriers, and queue dependencies.
- Execute using dynamic rendering and timeline submissions.
- Add DOT/text graph dumps and unit tests for compiler decisions.
- Add transient pooling; add aliasing only after lifetime tests are strong.

**Exit criteria:** a multi-pass scene (upload/cull, shadow, depth or opaque, lighting, tone map, readback) declares no manual image layouts or cross-pass barriers outside the graph.

### Phase 3 — scene, materials, and bindless resources

**Deliverables**

- Finish geometry import and define stable mesh/primitive/material/instance data.
- Establish a versioned shader ABI for frame/view, scene instances, materials, textures, samplers, and push data.
- Add global descriptor-indexing tables with non-uniform HLSL access, fallback descriptors, slot telemetry, and timeline-delayed reuse.
- Retain the current bind-group path as fallback/reference until the new path is tested on the target matrix.
- Add content-hashed shader/pipeline caches and asynchronous warmup.

**Exit criteria:** a scene with many materials/textures renders with a constant small number of descriptor-set binds, and destroying/recreating resources under multiple frames in flight produces no stale access under GPU-assisted validation.

### Phase 4 — first useful renderer

**Deliverables**

- Camera, transforms, physically based material subset, environment lighting, shadow map, transparency, and tone mapping.
- Clustered Forward+ light-list compute and debug visualizations.
- Optional compact deferred branch only for measurement, not as a second permanent architecture by default.
- Representative benchmark and golden-image scenes.

**Exit criteria:** correct multiple-light/material rendering, transparent objects, resize/history-independent headless output, and documented GPU timings on at least two vendors.

### Phase 5 — GPU-driven scene submission

**Deliverables**

- Geometry arenas, instance/bounds/material buffers, and indirect/count buffers.
- Compute frustum culling, LOD selection, compacted visible lists, and `vkCmdDrawIndexedIndirectCount`.
- Optional previous-frame Hi-Z occlusion with conservative visibility and camera-cut behavior.
- CPU/direct-draw reference path and per-stage counters.

**Exit criteria:** a high-object-count benchmark demonstrates lower CPU submission cost without regressions in GPU time or visibility correctness across target vendors.

### Phase 6 — temporal pipeline and scaling

**Deliverables**

- Motion vectors for camera and objects, jitter, history resources, invalidation rules, and debug views.
- TAA reference implementation.
- Render/display resolution separation, GPU-budget controller, and dynamic resolution.
- Integrate one cross-vendor temporal upscaler behind a small adapter.

**Exit criteria:** automated camera/object-motion sequences show stable vectors and controlled history reset; temporal quality and frame-time behavior are measured rather than judged only from still frames.

### Phase 7 — opt-in experiments

Each experiment must keep a fallback and have a written success metric:

- meshlet asset generation, then optional `VK_EXT_mesh_shader` against MDI;
- ray-query shadows/AO, then wider hybrid effects only if valuable;
- visibility-buffer branch against clustered Forward+ or deferred;
- descriptor-buffer backend, then descriptor-heap backend only after ecosystem validation;
- DGC only if ordinary indirect rendering is state-change or CPU limited;
- cluster-granular/virtual geometry only for content that exceeds mesh-level streaming.

## Principal risks and controls

| Risk | Consequence | Control |
|---|---|---|
| Resource slot reused while GPU still references it | Rare wrong texture/buffer or device loss | Generational CPU handles, fallback descriptors, timeline retirement |
| Graph access omitted or underspecified | Missing barrier, data race, bad layout | Restricted pass context, declaration validation, sync validation |
| Optional feature becomes architectural baseline | Unsupported devices or multiple half-working paths | Capability table and mandatory fallback before feature lands |
| Bindless shader indexes diverge | Wrong material/resource with weak CPU visibility | GPU-assisted validation, debug IDs, descriptor occupancy views |
| Pipeline creation occurs during a frame | Stutter | Hashing, persisted cache, warmup, feedback/timing, fallback pipeline |
| Per-resource Vulkan allocations exhaust limits or fragment | Allocation failures and streaming stalls | VMA suballocation, rings/pools, budgets and telemetry |
| Async compute increases contention | Worse GPU time despite overlap | Graph support but opt-in scheduling driven by traces |
| Temporal inputs are subtly wrong | Ghosting, shimmer, unstable exposure | Motion/history debug views and deterministic motion sequences |
| Advanced path has no comparable baseline | Complexity without demonstrated value | Keep CPU draw/classic raster/reference temporal paths |

## Final recommendation

For Ignis, "modern" should mean **data-oriented and GPU-capable, but evidence-driven**. Build a Vulkan 1.3 render-graph engine with explicit capability negotiation, timeline-managed lifetimes, suballocated memory, observable synchronization, and a stable shader ABI. Then use mature descriptor indexing for bindless resources and mature compute-culling plus indirect-count draws for GPU-driven rendering.

Treat the newest API mechanisms as implementation options, not engine concepts. In particular, preserve one high-level global resource-index model that can be backed today by descriptor-indexed sets, later by descriptor buffers, and eventually by `VK_EXT_descriptor_heap`. Preserve one GPU-scene model that can be consumed today by MDI, later by mesh shaders or DGC. This gives Ignis a credible path to modern rendering without committing its public architecture to whichever extension is newest in 2026.

# Ignis renderer implementation plan

Status: implementation in progress since 2026-08-09  
Research basis: [Modern real-time renderer techniques and an Ignis development roadmap](research/modern-renderer-techniques.md)

## Progress

- Milestone 0 slices 0.1-0.5 are implemented: GTest/CTest, correct pool
  accounting, generational keys with wrap retirement, typed RHI handles and
  domain errors, plus headless texture round-trip, render-pass clear, and
  indexed-triangle regression tests.
- Milestone 1.1 is implemented: Vulkan capabilities are queried through
  `Features2`/`Properties2`, exposed through a backend-neutral snapshot, and
  checked against the renderer baseline. Software and integrated devices are
  accepted unless `vulkan.requireDiscreteGPU` is explicitly enabled.
- Milestone 1.2 now uses per-queue 64-bit timeline points, `vkQueueSubmit2`, and
  Synchronization 2 image barriers. Transfer/graphics dependency tests pass;
  the compute leg remains pending until compute commands arrive in slice 1.5.
- Milestone 1.3 frame contexts and deferred destruction are next.

## Delivery rules

- Work in dependency order; do not begin a milestone until the previous milestone's exit criteria pass.
- Implement vertical slices: one externally observable behavior, one failing test, the minimum implementation, then the next behavior.
- Keep `Renderer` as the small application-facing interface. Synchronization, descriptor allocation, transient resources, and Vulkan extension details stay behind deeper modules.
- Require a fallback and a benchmark before an optional GPU feature becomes part of the normal renderer path.
- Keep Vulkan-specific types inside `rhi/vk/`.
- A milestone is complete only when its automated tests, validation checks, documentation, and representative sample pass.

## Test seams requiring confirmation

Tests will observe behavior only through these interfaces:

1. **Core allocation seam:** `Pool<T>` allocation, lookup, destruction, capacity, live count, generation, and stale-key behavior.
2. **Render-graph seam:** `RenderGraphCompiler::compile(description)` returns either a deterministic immutable execution plan or a domain error. Tests do not inspect compiler helpers.
3. **Resource-table seam:** allocate/update/retire stable resource IDs and resolve their GPU table positions; tests do not inspect descriptor allocator internals.
4. **Renderer seam:** compile, enqueue, query/wait, and observe frame results or errors.
5. **Headless RHI seam:** create a `Device`, submit backend-neutral work, wait for a timeline point, and verify readback plus validation output.

The first implementation slice starts only after these seams are accepted.

## Target module shape

```text
Application
    |
Renderer                          compile / enqueue / query / wait
    |-- RenderGraphCompiler       declarations -> immutable plan
    |-- FrameExecutor             plan + frame data -> submitted work
    |-- ResourceTable             stable IDs -> GPU-visible resources
    `-- AssetPipeline             source assets -> runtime data
                |
               RHI               backend-neutral GPU vocabulary
                |
         Vulkan adapter           Vulkan objects and execution
```

## Milestone 0 — trustworthy lifetime primitives

Purpose: prevent stale CPU handles, incorrect pool accounting, and untestable GPU regressions before introducing frames in flight or bindless indices.

### Slice 0.1 — test harness

- Enable CTest behind `BUILD_TESTING`.
- Add GTest through Conan and discover individual test cases through CTest.
- Make Debug and Release test commands explicit in the build documentation.
- Add no GPU requirement to core tests.

### Slice 0.2 — correct pool accounting

- Replace ambiguous `Pool::size()` with `liveCount()` and `capacity()`.
- Make destruction report invalid and double-destroy attempts without underflowing the live count.
- Test empty, full, destroy, reuse, and pool-full behavior through the pool interface.

### Slice 0.3 — generational pool keys

- Introduce an opaque pool key containing slot ID and generation.
- Make allocation return a key; make lookup and destruction require the complete key.
- Advance generation before a freed slot is reused.
- Reject stale keys after reuse.
- Define and test generation-wrap behavior.

### Slice 0.4 — typed handle integration

- Route RHI and renderer typed handles through generational pool keys.
- Remove raw-ID lookup and destruction from resource-manager implementation paths.
- Return domain errors for invalid handles instead of silently accepting them.
- Update command preprocessing so stale handles fail before command recording.

### Slice 0.5 — baseline headless regression suite

- Convert buffer upload/download, render-pass clear, and indexed triangle sandboxes into repeatable CTest integration cases where hardware is available.
- Preserve standalone sandbox executables for manual debugging.
- Verify deterministic readback and validation-layer cleanliness.

### Exit criteria

- Core tests run without a Vulkan device.
- Live count never underflows and differs correctly from capacity.
- Reused slots invalidate all previous keys.
- Invalid RHI handles fail safely before Vulkan recording.
- Existing headless examples retain their output.

## Milestone 1 — modern Vulkan RHI and frame execution

Purpose: establish the execution and lifetime model required by the render graph and bindless resources.

### Slice 1.1 — device capabilities

- Query features and properties through `vkGetPhysicalDeviceFeatures2` and `vkGetPhysicalDeviceProperties2` chains.
- Publish a backend-neutral capability snapshot.
- Separate required baseline features from optional renderer tiers.
- Log selected device, driver, limits, queues, memory heaps, and negotiated features.
- Stop requiring a discrete GPU unless configuration explicitly requests it.

### Slice 1.2 — Synchronization 2 and timelines

- Replace legacy barriers and `vkQueueSubmit` with Synchronization 2 and `vkQueueSubmit2`.
- Replace `WorkloadReceipt = u8` with queue timeline points.
- Use a 64-bit monotonic timeline per queue.
- Test transfer -> compute -> graphics -> readback dependencies.

### Slice 1.3 — frame contexts and deferred destruction

- Add two or three configurable frames in flight.
- Give each frame reusable command pools, query pools, staging space, and transient CPU metadata.
- Retire Vulkan resources and descriptor slots only after their last timeline point completes.
- Remove normal-frame host waits; retain explicit wait/query operations.

### Slice 1.4 — VMA and data movement

- Replace per-buffer and per-image `vkAllocateMemory` calls with VMA allocations.
- Add persistently mapped upload and readback rings.
- Add memory-budget and high-watermark telemetry.
- Add explicit buffer/image copy regions and mip-generation support needed by assets.

### Slice 1.5 — dynamic rendering and missing commands

- Replace Vulkan render-pass/framebuffer resources with dynamic-rendering attachment descriptions.
- Key pipelines by attachment formats and sample count instead of `RenderPassHandle`.
- Add compute pipelines, dispatch, indirect/count draws, timestamps, and debug labels.
- Keep barriers as an internal execution concept rather than an application-facing command.

### Exit criteria

- A test runs transfer -> compute -> indirect graphics -> readback without queue-idle calls.
- Two or more frames can remain in flight.
- Destroying an in-use resource delays actual destruction safely.
- Validation and synchronization validation are clean.
- GPU and memory metrics are observable.

## Milestone 2 — render-graph compiler and frame executor

Purpose: make the graph the single owner of pass ordering, hazards, transient lifetimes, and queue dependencies.

### Slice 2.1 — graph description

- Replace the current fluent stubs with separate setup and execute callbacks.
- Declare logical buffers/images, reads, writes, attachments, queue class, and side effects.
- Represent transient, imported, exported, and persistent-history resources.
- Give execution callbacks a restricted pass context that resolves only declared resources.

### Slice 2.2 — pure graph compilation

- Validate duplicate tags, missing producers, read-before-write, incompatible descriptions, and cycles.
- Cull passes that do not contribute to exported resources or side effects.
- Produce deterministic topological ordering.
- Return domain errors rather than logging or aborting.

### Slice 2.3 — lifetimes and synchronization plan

- Compute first and last use for each logical resource.
- Derive layouts, stage/access masks, buffer ranges, image subresources, and queue ownership transfers.
- Produce queue timeline waits and signals.
- Add textual and DOT graph dumps.

### Slice 2.4 — physical resources and execution

- Resolve imported resources and allocate transient physical resources.
- Record passes through the RHI and submit batches through the frame executor.
- Add transient pooling; add memory aliasing only after lifetime tests cover it.

### Exit criteria

- A multi-pass headless scene declares no manual cross-pass layout transitions or barriers.
- Compiler tests cover invalid graphs, ordering, culling, lifetimes, and synchronization plans without a GPU.
- Graph dumps and GPU debug labels match the compiled plan.

## Milestone 3 — assets, GPU scene, shader ABI, and bindless resources

Purpose: establish stable data contracts that can feed both direct and GPU-driven rendering.

### Slice 3.1 — asset pipeline

- Finish OBJ extraction for the existing sample.
- Add glTF mesh, material, texture, and scene import through fastgltf.
- Convert source assets into validated runtime data outside the frame loop.
- Upload geometry into device-local arenas rather than one allocation per primitive.

### Slice 3.2 — GPU scene

- Define contiguous buffers for instances, transforms, bounds, meshes, primitives, materials, and lights.
- Preserve stable object identities and previous transforms.
- Add incremental upload of changed ranges.
- Keep a CPU reference representation for verification.

### Slice 3.3 — versioned shader ABI

- Reserve resource spaces for global resources, frame/view data, pass data, and push data.
- Validate reflection results against the ABI at load time.
- Add content-hashed shader artifacts and pipeline keys.
- Persist `VkPipelineCache` and warm known pipelines outside frame execution.

### Slice 3.4 — descriptor-indexing resource table

- Add global arrays for textures, samplers, uniform buffers, and storage buffers.
- Use explicit fallback resources for unoccupied slots.
- Add non-uniform HLSL indexing and reflection validation.
- Delay slot reuse through queue timelines.
- Retain conventional bind groups as a fallback/reference until cross-vendor verification passes.

### Exit criteria

- A many-material scene uses a constant small number of descriptor-set binds.
- Repeated resource streaming and destruction is clean under GPU-assisted validation.
- The same runtime scene renders through the direct reference path.

## Milestone 4 — first useful renderer

Purpose: deliver a conventional, understandable renderer before advanced geometry or ray paths.

- Implement cameras, transforms, depth, physically based metallic/roughness materials, environment lighting, shadows, transparency, HDR, and tone mapping.
- Implement clustered Forward+ light-list construction and debug visualization.
- Add representative golden-image and benchmark scenes.
- Prototype compact deferred shading only as a measured comparison.

### Exit criteria

- Opaque and transparent PBR scenes render correctly with multiple lights and shadows.
- Pass timings, light-list occupancy, memory use, and pipeline counts are reported.
- Results and timings are recorded on at least two target GPU vendors.

## Milestone 5 — GPU-driven scene submission

Purpose: reduce CPU submission cost for large scenes using mature Vulkan mechanisms.

- Add compute frustum culling and LOD selection over GPU-scene buffers.
- Generate compact visible-instance, indirect-command, and draw-count buffers.
- Render through `vkCmdDrawIndexedIndirectCount`.
- Add conservative previous-frame Hi-Z occlusion only after frustum-culling measurements.
- Preserve CPU/direct drawing as a correctness and small-scene reference.

### Exit criteria

- A high-object-count benchmark shows lower CPU submission cost without visibility errors or GPU regressions.
- Per-stage counters explain rejected, visible, and submitted work.

## Milestone 6 — temporal rendering and scaling

Purpose: establish correct temporal data before integrating an upscaler.

- Add current/previous camera and object transforms, projection jitter, motion vectors, and history resources.
- Define resize, camera-cut, disocclusion, and exposure invalidation.
- Implement a simple TAA reference with debug views.
- Separate render and display resolution.
- Add timestamp-driven dynamic resolution with hysteresis.
- Integrate one cross-vendor temporal upscaler behind a small adapter.

### Exit criteria

- Automated camera and object motion sequences verify vector conventions and history resets.
- Temporal quality and frame-time behavior are measured across representative scenes.

## Milestone 7 — optional measured experiments

These do not become baseline requirements. Each needs a fallback, representative content, and a written success metric.

- Meshlet preprocessing followed by `VK_EXT_mesh_shader` versus indexed indirect drawing.
- Ray-query shadows or ambient occlusion before full ray-tracing pipelines.
- Visibility-buffer shading versus Forward+ or compact deferred shading.
- Descriptor buffers and later `VK_EXT_descriptor_heap` behind the existing resource-table seam.
- `VK_EXT_device_generated_commands` only if state changes limit ordinary indirect rendering.
- Cluster-granular or virtual geometry only after mesh-level streaming and memory budgets are proven insufficient.

## Immediate next slice

After the test seams are confirmed:

1. Add CTest and one failing `Pool<T>` behavior test.
2. Make that test pass with the smallest interface change.
3. Repeat for live count, invalid destruction, slot reuse, and stale generations.
4. Integrate typed RHI handles only after core pool behavior is stable.

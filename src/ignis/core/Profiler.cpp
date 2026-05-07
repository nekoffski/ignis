#include "Profiler.hh"

#include <fmt/std.h>

#include <algorithm>
#include <numeric>

#include "Scope.hh"

namespace ignis {

static constexpr f64 toMS(f64 nanoseconds) {
    return static_cast<f64>(nanoseconds) / 1'000'000.0;
}

RegionTimer Profiler::profileRegion(const std::string& name) {
    return RegionTimer{name, m_threads[std::this_thread::get_id()]};
}

void Profiler::registerThread() {
    const auto id = std::this_thread::get_id();
    std::lock_guard lock{m_mutex};
    m_threads.try_emplace(id);
}

ProfilerSummary Profiler::generateSummary() {
    ON_SCOPE_EXIT { clear(); };
    return ProfilerSummary{m_threads};
}

void Profiler::clear() {
    for (auto& [_, events] : m_threads) events.clear();
}

RegionTimer::RegionTimer(std::string name, ProfilerEvents& events)
    : m_name(std::move(name)), m_events(events) {
    m_events.push_back({
        .type = ProfilerEvent::Type::begin,
        .name = m_name,
        .timestamp = Clock::now(),
    });
}

RegionTimer::~RegionTimer() {
    m_events.push_back({
        .type = ProfilerEvent::Type::end,
        .name = m_name,
        .timestamp = Clock::now(),
    });
}

ProfilerSummary::ProfilerSummary(const ProfilerEventsPerThread& events) {
    generateSummary(events);
}

void ProfilerSummary::print() const {
    log::debug("──── Profiler ────────────────────────────────────");
    for (const auto& [threadId, graph] : m_summaryGraphs) {
        log::debug("  Thread {}", threadId);
        printGraph(graph, "  ");
    }
    log::debug("──────────────────────────────────────────────────");
}

void ProfilerSummary::generateSummary(const ProfilerEventsPerThread& events) {
    for (const auto& [threadId, threadEvents] : events) {
        SummaryGraphNode root{.name = "root"};
        processThread(root, threadEvents);
        m_summaryGraphs[threadId] = std::move(root);
    }
}

void ProfilerSummary::processThread(
    SummaryGraphNode& root, const ProfilerEvents& events
) {
    auto* current = &root;
    for (const auto& [type, name, timestamp] : events) {
        if (type == ProfilerEvent::Type::begin) {
            auto& child = current->children[name];
            child.name = name;
            child.parent = current;
            child.times.push_back(timestamp.time_since_epoch().count());
            current = &child;
        } else {
            auto& times = current->times;
            const auto beginTime = times.back();
            times.back() = timestamp.time_since_epoch().count() - beginTime;
            current = current->parent;
        }
    }
}

void ProfilerSummary::printGraph(
    const SummaryGraphNode& node, const std::string& prefix
) const {
    const auto& children = node.children;
    for (auto it = children.begin(); it != children.end(); ++it) {
        const bool isLast = std::next(it) == children.end();
        const auto& child = it->second;
        const auto stats = computeStats(child, child.name);

        const auto* branch = isLast ? "└─ " : "├─ ";
        const auto* stem = isLast ? "   " : "│  ";

        if (stats.count > 1) {
            log::debug(
                "{}{}{}  x{}  {:.3f}ms  (avg {:.3f}ms)", prefix, branch,
                child.name, stats.count, toMS(stats.totalTime),
                toMS(stats.averageTime)
            );
        } else {
            log::debug(
                "{}{}{}  {:.3f}ms", prefix, branch, child.name,
                toMS(stats.totalTime)
            );
        }
        printGraph(child, prefix + stem);
    }
}

void ProfilerSummary::forEachNode(
    const std::function<void(const CallStatistics&, u64, std::thread::id)>& fn
) const {
    for (const auto& [threadId, graph] : m_summaryGraphs)
        forEachNodeImpl(graph, "", fn, 0, threadId);
}

ProfilerSummary::CallStatistics ProfilerSummary::computeStats(
    const SummaryGraphNode& node, const std::string& fullName
) {
    const u64 total =
        std::accumulate(node.times.begin(), node.times.end(), u64{0});
    const u64 count = node.times.size();
    return {
        .count = count,
        .totalTime = static_cast<f64>(total),
        .averageTime = count > 0 ? static_cast<f64>(total) / count : 0.0,
        .name = node.name,
        .fullName = fullName,
    };
}

void ProfilerSummary::forEachNodeImpl(
    const SummaryGraphNode& node, const std::string& parentName,
    const std::function<void(const CallStatistics&, u64, std::thread::id)>& fn,
    u64 indent, std::thread::id threadId
) const {
    const bool isRoot = node.parent == nullptr;

    if (not isRoot) {
        const auto fullName =
            parentName.empty() ? node.name : parentName + "::" + node.name;
        fn(computeStats(node, fullName), indent, threadId);
        for (const auto& [_, child] : node.children)
            forEachNodeImpl(child, fullName, fn, indent + 1, threadId);
    } else {
        for (const auto& [_, child] : node.children)
            forEachNodeImpl(child, parentName, fn, indent, threadId);
    }
}

}  // namespace ignis

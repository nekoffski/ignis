#include <gtest/gtest.h>

#include <concepts>
#include <utility>

#include "ignis/rhi/ResourceManager.hh"

namespace {

template <typename Handle>
using DestroyResult =
    decltype(std::declval<ignis::rhi::ResourceManager&>().destroy(
        std::declval<Handle>()
    ));

}  // namespace

TEST(ResourceManagerContractTest, DestructionCanReportAnInvalidHandle) {
    constexpr bool destructionReportsErrors = std::same_as<
        DestroyResult<ignis::rhi::BufferHandle>, ignis::Opt<ignis::Error>>;

    EXPECT_TRUE(destructionReportsErrors);
}

TEST(ResourceHandleTest, GenerationParticipatesInIdentityAndHashing) {
    const ignis::rhi::BufferHandle original{7, 1};
    const ignis::rhi::BufferHandle replacement{7, 2};

    EXPECT_NE(original, replacement);
    EXPECT_NE(
        std::hash<ignis::rhi::BufferHandle>{}(original),
        std::hash<ignis::rhi::BufferHandle>{}(replacement)
    );
}

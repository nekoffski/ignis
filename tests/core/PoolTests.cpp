#include <gtest/gtest.h>

#include "ignis/core/Pool.hh"

TEST(PoolTest, NewPoolHasNoLiveValues) {
    const ignis::Pool<int> pool{2};

    EXPECT_EQ(pool.liveCount(), 0);
}

TEST(PoolTest, ReportsItsFixedCapacity) {
    const ignis::Pool<int> pool{2};

    EXPECT_EQ(pool.capacity(), 2);
}

TEST(PoolTest, CreatesAndFindsAValue) {
    ignis::Pool<int> pool{1};

    const auto key = pool.create(42);

    ASSERT_TRUE(key.has_value());
    ASSERT_NE(pool.get(*key), nullptr);
    EXPECT_EQ(*pool.get(*key), 42);
}

TEST(PoolTest, ReportsPoolFullWithoutChangingTheLiveCount) {
    ignis::Pool<int> pool{1};
    ASSERT_TRUE(pool.create(42).has_value());

    const auto result = pool.create(7);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code(), ignis::Error::Code::poolFull);
    EXPECT_EQ(pool.liveCount(), 1);
}

TEST(PoolTest, UpdatesLiveCountWhenAValueIsDestroyed) {
    ignis::Pool<int> pool{1};
    const auto key = pool.create(42);
    ASSERT_TRUE(key.has_value());
    ASSERT_EQ(pool.liveCount(), 1);

    ASSERT_TRUE(pool.destroy(*key));

    EXPECT_EQ(pool.liveCount(), 0);
    EXPECT_EQ(pool.get(*key), nullptr);
}

TEST(PoolTest, RejectsDestroyingTheSameSlotTwice) {
    ignis::Pool<int> pool{1};
    const auto id = pool.create(42);
    ASSERT_TRUE(id.has_value());

    ASSERT_TRUE(pool.destroy(*id));
    EXPECT_FALSE(pool.destroy(*id));
}

TEST(PoolTest, RejectsAKeyAfterItsSlotIsReused) {
    ignis::Pool<int> pool{1};
    const auto original = pool.create(42);
    ASSERT_TRUE(original.has_value());
    ASSERT_TRUE(pool.destroy(*original));

    const auto replacement = pool.create(7);
    ASSERT_TRUE(replacement.has_value());

    EXPECT_EQ(pool.get(*original), nullptr);
}

TEST(PoolTest, ReusesAFreeSlotWithANewGeneration) {
    ignis::Pool<int> pool{1};
    const auto original = pool.create(42);
    ASSERT_TRUE(original.has_value());
    ASSERT_TRUE(pool.destroy(*original));

    const auto replacement = pool.create(7);

    ASSERT_TRUE(replacement.has_value());
    EXPECT_EQ(replacement->id, original->id);
    EXPECT_NE(replacement->generation, original->generation);
    ASSERT_NE(pool.get(*replacement), nullptr);
    EXPECT_EQ(*pool.get(*replacement), 7);
}

TEST(PoolTest, StaleKeyCannotDestroyAReplacement) {
    ignis::Pool<int> pool{1};
    const auto original = pool.create(42);
    ASSERT_TRUE(original.has_value());
    ASSERT_TRUE(pool.destroy(*original));
    const auto replacement = pool.create(7);
    ASSERT_TRUE(replacement.has_value());

    EXPECT_FALSE(pool.destroy(*original));
    EXPECT_EQ(pool.liveCount(), 1);
    ASSERT_NE(pool.get(*replacement), nullptr);
    EXPECT_EQ(*pool.get(*replacement), 7);
}

TEST(PoolTest, RejectsAnOutOfRangeKey) {
    ignis::Pool<int> pool{1};
    const ignis::HandleKey key{1, 0};

    EXPECT_EQ(pool.get(key), nullptr);
    EXPECT_FALSE(pool.destroy(key));
    EXPECT_EQ(pool.liveCount(), 0);
}

TEST(PoolTest, RetiresASlotInsteadOfWrappingItsGeneration) {
    ignis::Pool<int, ignis::u8> pool{1};

    for (ignis::u32 generation = 0;
         generation <= std::numeric_limits<ignis::u8>::max(); ++generation) {
        const auto key = pool.create(42);
        ASSERT_TRUE(key.has_value());
        EXPECT_EQ(key->generation, generation);
        ASSERT_TRUE(pool.destroy(*key));
    }

    const auto result = pool.create(7);

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().code(), ignis::Error::Code::poolFull);
    EXPECT_EQ(pool.liveCount(), 0);
}

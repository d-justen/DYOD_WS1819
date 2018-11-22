#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "../../lib/storage/fitted_attribute_vector.hpp"

namespace opossum {

class StorageFittedAttributeVectorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    fav->set(0, ValueID{3});
    fav->set(1, ValueID{1});
    fav->set(2, ValueID{0});
    fav->set(3, ValueID{2});
  }

  std::shared_ptr<opossum::FittedAttributeVector<uint16_t>> fav =
      std::make_shared<opossum::FittedAttributeVector<std::uint16_t>>(4);
};

TEST_F(StorageFittedAttributeVectorTest, GetSet) {
  EXPECT_EQ(fav->get(0), ValueID{3});
  EXPECT_EQ(fav->get(1), ValueID{1});
  EXPECT_EQ(fav->get(2), ValueID{0});
  EXPECT_EQ(fav->get(3), ValueID{2});
}

TEST_F(StorageFittedAttributeVectorTest, Size) { EXPECT_EQ(fav->size(), 4ul); }

TEST_F(StorageFittedAttributeVectorTest, Width) { EXPECT_EQ(fav->width(), 2u); }

}  // namespace opossum

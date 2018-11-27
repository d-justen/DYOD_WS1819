#include <limits>
#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "../../lib/all_type_variant.hpp"
#include "../../lib/resolve_type.hpp"
#include "../../lib/storage/base_segment.hpp"
#include "../../lib/storage/dictionary_segment.hpp"
#include "../../lib/storage/value_segment.hpp"

class StorageDictionarySegmentTest : public ::testing::Test {
 protected:
  std::shared_ptr<opossum::ValueSegment<int>> vc_int = std::make_shared<opossum::ValueSegment<int>>();
  std::shared_ptr<opossum::ValueSegment<std::string>> vc_str = std::make_shared<opossum::ValueSegment<std::string>>();
};

TEST_F(StorageDictionarySegmentTest, CompressSegmentString) {
  vc_str->append("Bill");
  vc_str->append("Steve");
  vc_str->append("Alexander");
  vc_str->append("Steve");
  vc_str->append("Hasso");
  vc_str->append("Bill");

  auto col = opossum::make_shared_by_data_type<opossum::BaseSegment, opossum::DictionarySegment>("string", vc_str);
  auto dict_col = std::dynamic_pointer_cast<opossum::DictionarySegment<std::string>>(col);

  // Test attribute_vector size
  EXPECT_EQ(dict_col->size(), 6u);

  // Test dictionary size (uniqueness)
  EXPECT_EQ(dict_col->unique_values_count(), 4u);

  // Test sorting
  auto dict = dict_col->dictionary();
  EXPECT_EQ((*dict)[0], "Alexander");
  EXPECT_EQ((*dict)[1], "Bill");
  EXPECT_EQ((*dict)[2], "Hasso");
  EXPECT_EQ((*dict)[3], "Steve");
}

TEST_F(StorageDictionarySegmentTest, LowerUpperBound) {
  for (int i = 0; i <= 10; i += 2) vc_int->append(i);
  auto col = opossum::make_shared_by_data_type<opossum::BaseSegment, opossum::DictionarySegment>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<opossum::DictionarySegment<int>>(col);

  EXPECT_EQ(dict_col->lower_bound(4), (opossum::ValueID)2);
  EXPECT_EQ(dict_col->upper_bound(4), (opossum::ValueID)3);

  EXPECT_EQ(dict_col->lower_bound(5), (opossum::ValueID)3);
  EXPECT_EQ(dict_col->upper_bound(5), (opossum::ValueID)3);

  EXPECT_EQ(dict_col->lower_bound(15), opossum::INVALID_VALUE_ID);
  EXPECT_EQ(dict_col->upper_bound(15), opossum::INVALID_VALUE_ID);
}

TEST_F(StorageDictionarySegmentTest, GetAppend) {
  for (int i = 0; i <= 10; i += 2) vc_int->append(i);
  auto col = opossum::make_shared_by_data_type<opossum::BaseSegment, opossum::DictionarySegment>("int", vc_int);
  auto dict_col = std::dynamic_pointer_cast<opossum::DictionarySegment<int>>(col);

  const opossum::AllTypeVariant test_atp = opossum::AllTypeVariant{0};
  EXPECT_EQ((*dict_col)[0], test_atp);
  EXPECT_EQ(dict_col->get(0), 0);
  EXPECT_THROW(dict_col->append(test_atp), std::exception);
}

TEST_F(StorageDictionarySegmentTest, FittedAttributeVector8) {
  for (u_int8_t i = 0; i < std::numeric_limits<uint8_t>::max(); i++) vc_int->append(std::to_string(i));
  auto col = opossum::make_shared_by_data_type<opossum::BaseSegment, opossum::DictionarySegment>("string", vc_str);
  auto dict_col = std::dynamic_pointer_cast<opossum::DictionarySegment<std::string>>(col);

  EXPECT_EQ(dict_col->attribute_vector()->width(), 1);
}

TEST_F(StorageDictionarySegmentTest, FittedAttributeVector16) {
  for (u_int16_t i = 0; i < static_cast<uint16_t>(std::numeric_limits<uint8_t>::max()) + 1; i++) {
    vc_str->append(std::to_string(i));
  }
  auto col = opossum::make_shared_by_data_type<opossum::BaseSegment, opossum::DictionarySegment>("string", vc_str);
  auto dict_col = std::dynamic_pointer_cast<opossum::DictionarySegment<std::string>>(col);

  EXPECT_EQ(dict_col->attribute_vector()->width(), 2);
}

TEST_F(StorageDictionarySegmentTest, FittedAttributeVectorDuplicates) {
  for (size_t i = 0; i < std::numeric_limits<uint8_t>::max() - 1; i++) {
    vc_str->append(std::to_string(i));
    vc_str->append(std::to_string(i));
  }
  // Expect FittedAttributeVector to still have uint8_t values after deduplication in ctor
  auto col = opossum::make_shared_by_data_type<opossum::BaseSegment, opossum::DictionarySegment>("string", vc_str);
  auto dict_col = std::dynamic_pointer_cast<opossum::DictionarySegment<std::string>>(col);

  EXPECT_EQ(dict_col->attribute_vector()->width(), 1);
}

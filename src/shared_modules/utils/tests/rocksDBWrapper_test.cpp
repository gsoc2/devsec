/*
 * Wazuh - Shared Modules utils tests
 * Copyright (C) 2015, Wazuh Inc.
 * September 9, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "rocksDBWrapper_test.hpp"

/**
 * @brief Tests the put function
 */
TEST_F(RocksDBWrapperTest, TestPut)
{
    EXPECT_NO_THROW(db_wrapper->put("key1", "value1"));
}

/**
 * @brief Tests the put function with an empty key or value
 */
TEST_F(RocksDBWrapperTest, TestPutEmptyKeyOrValue)
{
    EXPECT_THROW(db_wrapper->put("", "value1"), std::invalid_argument);
    EXPECT_THROW(db_wrapper->put("key2", ""), std::invalid_argument);
}

/**
 * @brief Tests the put function with a key that already exists
 */
TEST_F(RocksDBWrapperTest, TestPutExistingKey)
{
    EXPECT_NO_THROW(db_wrapper->put("key3", "value3"));
    EXPECT_NO_THROW(db_wrapper->put("key3", "value3"));
}

/**
 * @brief Tests that the value is updated when the put function is called with an existing key
 */
TEST_F(RocksDBWrapperTest, TestPutExistingKeyUpdateValue)
{
    const std::string value3 {"value3"};
    EXPECT_NO_THROW(db_wrapper->put("key3", value3));
    std::string value {};
    db_wrapper->get("key3", value);
    EXPECT_EQ(value, value3);

    const std::string newValue {"newValue"};
    value = {};
    EXPECT_NO_THROW(db_wrapper->put("key3", newValue)); // The value should be updated
    db_wrapper->get("key3", value);
    EXPECT_EQ(value, newValue);
}

/**
 * @brief Tests the get function
 */
TEST_F(RocksDBWrapperTest, TestGet)
{
    db_wrapper->put("key2", "value2");
    std::string value {};
    EXPECT_NO_THROW(db_wrapper->get("key2", value));
    EXPECT_EQ(value, "value2");
}

/**
 * @brief Tests the get function with a non-existent key
 */
TEST_F(RocksDBWrapperTest, TestGetNonExistentKey)
{
    std::string value {};
    EXPECT_THROW(db_wrapper->get("non_existent_key", value), std::invalid_argument);
}

/**
 * @brief Tests the get function with an empty key
 */
TEST_F(RocksDBWrapperTest, TestGetEmptyKey)
{
    std::string value {};
    EXPECT_THROW(db_wrapper->get("", value), std::invalid_argument);
}

/**
 * @brief Tests the get function with an empty database
 */
TEST_F(RocksDBWrapperTest, TestGetEmptyDB)
{
    Utils::RocksDBWrapper new_db_wrapper("new_test.db");
    std::string value {};
    EXPECT_THROW(new_db_wrapper.get("key1", value), std::invalid_argument);
}

/**
 * @brief Tests the delete_ function
 */
TEST_F(RocksDBWrapperTest, TestDelete)
{
    db_wrapper->put("key3", "value3");
    EXPECT_NO_THROW(db_wrapper->delete_("key3"));
    std::string value {};
    EXPECT_THROW(db_wrapper->get("key3", value), std::invalid_argument); // The key should have been deleted
}

/**
 * @brief Tests the delete_ function with a non-existent key
 */
TEST_F(RocksDBWrapperTest, TestDeleteNonExistentKey)
{
    EXPECT_NO_THROW(db_wrapper->delete_("non_existent_key"));
}

/**
 * @brief Tests the delete_ function with an empty key
 */
TEST_F(RocksDBWrapperTest, TestDeleteEmptyKey)
{
    EXPECT_NO_THROW(db_wrapper->delete_(""));
}

/**
 * @brief Tests the delete_ function with an empty database
 */
TEST_F(RocksDBWrapperTest, TestDeleteEmptyDB)
{
    Utils::RocksDBWrapper new_db_wrapper("new_test.db");
    EXPECT_NO_THROW(new_db_wrapper.delete_("key1"));
}

/**
 * @brief Tests the deleteAll function
 */
TEST_F(RocksDBWrapperTest, TestGetLastKeyValue)
{
    db_wrapper->put("key4", "value4");
    db_wrapper->put("key5", "value5");

    const auto [lastKey, lastValue] = db_wrapper->getLastKeyValue();
    EXPECT_EQ(lastKey, "key5");
    EXPECT_EQ(lastValue, "value5");
}

/**
 * @brief Tests the getLastKeyValue function with an empty database
 */
TEST_F(RocksDBWrapperTest, TestGetLastKeyValueEmptyDB)
{
    Utils::RocksDBWrapper new_db_wrapper("new_test.db");
    const auto [lastKey, lastValue] = new_db_wrapper.getLastKeyValue();
    EXPECT_EQ(lastKey, "");
    EXPECT_EQ(lastValue, "");
}

/**
 * @brief Tests the deleteAll function
 */
TEST_F(RocksDBWrapperTest, TestDeleteAll)
{
    db_wrapper->put("key6", "value6");
    db_wrapper->put("key7", "value7");
    EXPECT_NO_THROW(db_wrapper->deleteAll());
    std::string value {};
    EXPECT_THROW(db_wrapper->get("key6", value), std::invalid_argument); // The key should have been deleted
    EXPECT_THROW(db_wrapper->get("key7", value), std::invalid_argument); // The key should have been deleted
}

/**
 * @brief Tests the deleteAll function with an empty database
 */
TEST_F(RocksDBWrapperTest, TestDeleteAllEmptyDB)
{
    Utils::RocksDBWrapper new_db_wrapper("new_test.db");
    EXPECT_NO_THROW(db_wrapper->deleteAll());
}

/**
 * @brief Tests the seek function
 */
TEST_F(RocksDBWrapperTest, TestSeek)
{
    db_wrapper->put("key9", "value9");
    db_wrapper->put("key10", "value10");
    db_wrapper->put("key11", "value11");

    std::string value {};
    auto it = db_wrapper->seek("key10");
    EXPECT_EQ(it->key().ToString(), "key10");
    EXPECT_EQ(it->value().ToString(), "value10");
}

/**
 * @brief Tests the seek function with an empty database
 */
TEST_F(RocksDBWrapperTest, TestSeekEmptyDB)
{
    Utils::RocksDBWrapper new_db_wrapper("new_test.db");
    EXPECT_THROW(new_db_wrapper.seek("key1"), std::invalid_argument);
}

/**
 * @brief Tests the seek function with a non-existent key
 */
TEST_F(RocksDBWrapperTest, TestSeekNonExistentKey)
{
    db_wrapper->put("key12", "value12");
    EXPECT_THROW(db_wrapper->seek("non_existent_key"), std::invalid_argument);
}

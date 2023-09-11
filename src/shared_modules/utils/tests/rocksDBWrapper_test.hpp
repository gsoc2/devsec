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

#ifndef _ROCKS_DB_WRAPPER_TEST_HPP
#define _ROCKS_DB_WRAPPER_TEST_HPP

#include "rocksDBWrapper.hpp"
#include "gtest/gtest.h"

/**
 * @brief Tests the RocksDBWrapper class
 *
 */
class RocksDBWrapperTest : public ::testing::Test
{
protected:
    RocksDBWrapperTest() = default;
    ~RocksDBWrapperTest() override = default;

    /**
     * @brief RocksDBWrapper object
     *
     */
    std::optional<Utils::RocksDBWrapper> db_wrapper;

    /**
     * @brief Initial conditions for tests
     *
     */
    // cppcheck-suppress unusedFunction
    void SetUp() override
    {
        db_wrapper = Utils::RocksDBWrapper("test.db");
    }

    /**
     * @brief Tear down routine for tests
     *
     */
    // cppcheck-suppress unusedFunction
    void TearDown() override
    {
        db_wrapper->deleteAll();
    }
};

#endif //_ROCKS_DB_WRAPPER_TEST_HPP

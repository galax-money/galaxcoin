// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.


#pragma once 

#include "Chaingen.h"

class CheckBlockPurged : public test_chain_unit_base {
public:
  CheckBlockPurged(size_t invalidBlockIdx) :
    m_invalidBlockIdx(invalidBlockIdx) {

    CryptoNote::CurrencyBuilder currencyBuilder(m_logger);
    currencyBuilder.upgradeHeightV2(blockMajorVersion == CryptoNote::BLOCK_MAJOR_VERSION_1 ? CryptoNote::IUpgradeDetector::UNDEF_HEIGHT : UINT32_C(0));
    m_currency.reset(new Currency(currencyBuilder.currency()));


    REGISTER_CALLBACK("check_block_purged", CheckBlockPurged::check_block_purged);
    REGISTER_CALLBACK("markInvalidBlock", CheckBlockPurged::markInvalidBlock);
  }

  bool check_block_verification_context(std::error_code bve, size_t eventIdx, const CryptoNote::BlockTemplate& /*blk*/) {
    std::cout << "message: " << bve.message() << std::endl;
    return m_invalidBlockIdx != eventIdx ? blockWasAdded(bve) : blockWasNotAdded(bve);
  }

  bool check_block_verification_context(std::error_code bve, size_t eventIdx, const CryptoNote::RawBlock& /*blk*/) {
    return check_block_verification_context(bve, eventIdx, CryptoNote::BlockTemplate{});
  }

  bool check_block_purged(CryptoNote::Core& c, size_t eventIdx, const std::vector<test_event_entry>& events) {
    DEFINE_TESTS_ERROR_CONTEXT("CheckBlockPurged::check_block_purged");

    CHECK_TEST_CONDITION(m_invalidBlockIdx < eventIdx);
    CHECK_EQ(0, c.getPoolTransactionCount());
    CHECK_EQ(m_invalidBlockIdx, c.getTopBlockIndex() + 1);

    return true;
  }

  bool markInvalidBlock(CryptoNote::Core& c, size_t eventIdx, const std::vector<test_event_entry>& events) {
    m_invalidBlockIdx = eventIdx + 1;
    return true;
  }

protected:
  size_t m_invalidBlockIdx;
};


struct CheckBlockAccepted : public test_chain_unit_base {
  CheckBlockAccepted(size_t expectedBlockchainHeight) :
    m_expectedBlockchainHeight(expectedBlockchainHeight) {

    CryptoNote::CurrencyBuilder currencyBuilder(m_logger);
    currencyBuilder.upgradeHeightV2(blockMajorVersion == CryptoNote::BLOCK_MAJOR_VERSION_1 ? CryptoNote::IUpgradeDetector::UNDEF_HEIGHT : UINT32_C(0));
    m_currency.reset(new Currency(currencyBuilder.currency()));


    REGISTER_CALLBACK("check_block_accepted", CheckBlockAccepted::check_block_accepted);
  }

  bool check_block_accepted(CryptoNote::Core& c, size_t /*eventIdx*/, const std::vector<test_event_entry>& /*events*/) {
    DEFINE_TESTS_ERROR_CONTEXT("CheckBlockAccepted::check_block_accepted");

    CHECK_EQ(0, c.getPoolTransactionCount());
    CHECK_EQ(m_expectedBlockchainHeight, c.getTopBlockIndex() + 1);

    return true;
  }

protected:
  size_t m_expectedBlockchainHeight;
};


struct TestBlockMajorVersionAccepted : public CheckBlockAccepted {
  TestBlockMajorVersionAccepted() :
    CheckBlockAccepted(2) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct TestBlockMajorVersionRejected : public CheckBlockPurged {
  TestBlockMajorVersionRejected() :
    CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct TestBlockBigMinorVersion : public CheckBlockAccepted {

  TestBlockBigMinorVersion()
    : CheckBlockAccepted(2) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_ts_not_checked : public CheckBlockAccepted
{
  gen_block_ts_not_checked(uint8_t blockMajorVersion)
    : CheckBlockAccepted(0, blockMajorVersion) {
    // bad bad bad
    const_cast<CryptoNote::BlockTemplate&>(m_currency->genesisBlock()).timestamp = 1338224400;
    m_expectedBlockchainHeight = m_currency->timestampCheckWindow();

  }

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_ts_in_past : public CheckBlockPurged
{
  gen_block_ts_in_past(uint8_t blockMajorVersion)
    : CheckBlockPurged(0, blockMajorVersion) {
    const_cast<CryptoNote::BlockTemplate&>(m_currency->genesisBlock()).timestamp = 1338224400;
    m_invalidBlockIdx = m_currency->timestampCheckWindow();

  }

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_ts_in_future_rejected : public CheckBlockPurged
{
  gen_block_ts_in_future_rejected(uint8_t blockMajorVersion)
    : CheckBlockPurged(1, blockMajorVersion) {
      const_cast<CryptoNote::BlockTemplate&>(m_currency->genesisBlock()).timestamp = 1338224400;
    }


  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_ts_in_future_accepted : public CheckBlockAccepted
{
  gen_block_ts_in_future_accepted()
    : CheckBlockAccepted(2) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_invalid_prev_id : public CheckBlockPurged
{
  gen_block_invalid_prev_id() 
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
  bool check_block_verification_context(std::error_code bve, size_t event_idx, const CryptoNote::BlockTemplate& /*blk*/);
};

struct gen_block_invalid_nonce : public CheckBlockPurged
{
  gen_block_invalid_nonce()
    : CheckBlockPurged(3) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_no_miner_tx : public CheckBlockPurged
{
  gen_block_no_miner_tx()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_unlock_time_is_low : public CheckBlockPurged
{
  gen_block_unlock_time_is_low()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_unlock_time_is_high : public CheckBlockPurged
{
  gen_block_unlock_time_is_high()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_unlock_time_is_timestamp_in_past : public CheckBlockPurged
{
  gen_block_unlock_time_is_timestamp_in_past()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_unlock_time_is_timestamp_in_future : public CheckBlockPurged
{
  gen_block_unlock_time_is_timestamp_in_future()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_height_is_low : public CheckBlockPurged
{
  gen_block_height_is_low()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_height_is_high : public CheckBlockPurged
{
  gen_block_height_is_high()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_miner_tx_has_2_tx_gen_in : public CheckBlockPurged
{
  gen_block_miner_tx_has_2_tx_gen_in(uint8_t blockMajorVersion)
    : CheckBlockPurged(1, blockMajorVersion) {
    m_invalidBlockIdx = 2;
  }


  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_miner_tx_has_2_in : public CheckBlockPurged
{
  gen_block_miner_tx_has_2_in(uint8_t blockMajorVersion)
    : CheckBlockPurged(0, blockMajorVersion) {
    m_invalidBlockIdx = m_currency->minedMoneyUnlockWindow() + 2;

  }

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_miner_tx_with_txin_to_key : public CheckBlockPurged
{
  gen_block_miner_tx_with_txin_to_key(uint8_t blockMajorVersion)
    : CheckBlockPurged(0, blockMajorVersion) {
    m_invalidBlockIdx = m_currency->minedMoneyUnlockWindow() + 2;

  }

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_miner_tx_out_is_small : public CheckBlockPurged
{
  gen_block_miner_tx_out_is_small()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_miner_tx_out_is_big : public CheckBlockPurged
{
  gen_block_miner_tx_out_is_big()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_miner_tx_has_no_out : public CheckBlockPurged
{
  gen_block_miner_tx_has_no_out()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_miner_tx_has_out_to_alice : public CheckBlockAccepted
{
  gen_block_miner_tx_has_out_to_alice()
    : CheckBlockAccepted(2) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_has_invalid_tx : public CheckBlockPurged
{
  gen_block_has_invalid_tx()
    : CheckBlockPurged(1) {}

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_is_too_big : public CheckBlockPurged
{
  gen_block_is_too_big()
      : CheckBlockPurged(1) {
    CryptoNote::CurrencyBuilder currencyBuilder(m_logger);
    currencyBuilder.upgradeHeightV2(blockMajorVersion == CryptoNote::BLOCK_MAJOR_VERSION_1 ? CryptoNote::IUpgradeDetector::UNDEF_HEIGHT : UINT32_C(0));

    currencyBuilder.maxBlockSizeInitial(std::numeric_limits<size_t>::max() / 2);
    m_currency.reset(new Currency(currencyBuilder.currency()));
  }

  bool generate(std::vector<test_event_entry>& events) const;
};

struct TestBlockCumulativeSizeExceedsLimit : public CheckBlockPurged {
  TestBlockCumulativeSizeExceedsLimit()
    : CheckBlockPurged(std::numeric_limits<size_t>::max()) {
  }

  bool generate(std::vector<test_event_entry>& events) const;
};

struct gen_block_invalid_binary_format : public test_chain_unit_base
{
  gen_block_invalid_binary_format();

  bool generate(std::vector<test_event_entry>& events) const;
  bool check_block_verification_context(std::error_code bve, size_t event_idx, const CryptoNote::BlockTemplate& /*blk*/);
  bool check_all_blocks_purged(CryptoNote::Core& c, size_t ev_index, const std::vector<test_event_entry>& events);
  bool corrupt_blocks_boundary(CryptoNote::Core& c, size_t ev_index, const std::vector<test_event_entry>& events);

private:
  size_t m_corrupt_blocks_begin_idx;
};

//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#include "utils.hpp"

#include <cassert>

using std::bitset;
using std::to_integer;

using std::cout;
using std::endl;
using std::min;

auto get_submatrix_as_uint64s(vector<byte>& db, size_t db_row_length_in_bits,
                              size_t subm_top_left_corner_in_bits,
                              const size_t subm_row_length_in_bits,
                              size_t subm_rows) -> vector<uint64_t> {
  vector<uint64_t> subm;
  subm.reserve(subm_rows);
  assert(subm_row_length_in_bits == 18 && "unsupported submatrix size");

  // add 4 padding bytes to the db
  for (size_t i = 0; i < 4; i++) {
    db.push_back(byte(0));
  }

  const auto db_rows = db.size() / (db_row_length_in_bits / 8);
  const auto subm_start_row =
      subm_top_left_corner_in_bits / db_row_length_in_bits;
  const auto subm_end_row = subm_start_row + subm_rows;

  const auto iterable_db_rows = min(db_rows, subm_end_row) - subm_start_row;
  const auto paddinging_rows = subm_rows - iterable_db_rows;

  for (size_t i = 0; i < iterable_db_rows; i++) {
    const size_t db_row_index =
        subm_top_left_corner_in_bits / 8 + i * db_row_length_in_bits / 8;
    const int db_row_offset_in_bits = subm_top_left_corner_in_bits % 8;
    const size_t subm_row_length_in_bytes = subm_row_length_in_bits / 8 + 1;

    const auto db_row = db.begin() + db_row_index;

    bitset<64> subm_row = 0;

    auto first_row = bitset<8>(db_row[0]) << db_row_offset_in_bits;
    first_row >>= db_row_offset_in_bits;

    subm_row |= bitset<64>(first_row.to_ullong());
    for (size_t j = 1; j < subm_row_length_in_bytes; j++) {
      const auto db_byte_as_int = bitset<64>(db_row[j]);
      subm_row <<= 8;
      subm_row |= db_byte_as_int;
    }
    // bits consumed by the first byte = 8 - db_row_offset_in_bits
    // bits consumed by shifting = 8 * (subm_row_length_in_bytes - 1)
    // bits_we_want_left = subm_row_length_in_bits
    // So we shift by (bits_consumed - bits_we_want_left)
    subm_row >>= ((8 - db_row_offset_in_bits) +
                  8 * (subm_row_length_in_bytes - 1) - subm_row_length_in_bits);

    subm.push_back(subm_row.to_ullong());
  }

  for (size_t i = 0; i < paddinging_rows; i++) {
    subm.push_back(0);
  }

  // remove the padding byte
  for (size_t i = 0; i < 4; i++) {
    db.pop_back();
  }

  return subm;
}
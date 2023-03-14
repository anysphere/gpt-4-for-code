//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: MIT
//

#pragma once

#include <algorithm>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

// extract a submatrix from a matrix db, where each row in the submatrix is a
// uint64_t
//
// db is a row-major stored matrix with db_row_length_in_bits bits per row.
// subm_top_left_corner_in_bits represent the index of the top left corner of
// the submatrix, in bits. subm_row_length_in_bits is the number of bits in each
// row of the submatrix. subm_cols is the number of columns in the submatrix.
//
// note: if subm_top_left_corner_in_bits + subm_row_length_in_bits goes past the
// right edge of the matrix, we DONT want to wrap around, but instead pretend
// that the db matrix is padded to the right with 0s.
//
// precondition: db.size() is a multiple of row_length_in_bits/8
//
//

#include <bitset>
#include <cassert>

using byte = unsigned char;

using std::bitset;
using std::vector;

auto get_submatrix_as_uint64s(vector<byte>& db, size_t db_row_length_in_bits,
                              size_t subm_top_left_corner_in_bits,
                              size_t subm_row_length_in_bits, size_t subm_rows)
    -> vector<uint64_t>;

template <int N>
auto concat_N_lsb_bits(const vector<uint64_t>& v) -> vector<byte> {
  bitset<8 * N> bits;
  vector<byte> result;

  for (size_t i = 0; i < v.size(); i += 8) {
    bits = 0;
    for (size_t j = 0; j < 8; j++) {
      if (i + j >= v.size()) {
        break;
      }
      auto extract_N_bits = v[i + j] & ((1 << N) - 1);
      bits <<= N;
      bits |= extract_N_bits;
    }
    vector<byte> semi_result;
    for (size_t j = 0; j < N; j++) {
      // get 8 bits from the j-th bit to the j+3-th bit
      bitset<8 * N> mask = 0;
      mask = (1 << 8) - 1;
      mask <<= (j * 8);
      auto extract_bits = (bits & mask) >> (j * 8);
      byte b = static_cast<byte>(extract_bits.to_ulong());
      semi_result.push_back(b);
    }
    std::reverse(semi_result.begin(), semi_result.end());
    result.insert(result.end(), semi_result.begin(), semi_result.end());
  }

  size_t output_size = v.size() * N / 8;
  assert(output_size <= result.size());
  for (size_t i = 0; i < result.size() - output_size; i++) {
    result.pop_back();
  }

  return result;
}
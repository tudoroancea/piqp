// This file is part of PIQP.
//
// Copyright (c) 2024 EPFL
//
// This source code is licensed under the BSD 2-Clause License found in the
// LICENSE file in the root directory of this source tree.

#ifndef PIQP_SPARSE_KKT_TPP
#define PIQP_SPARSE_KKT_TPP

#include "piqp/common.hpp"
#include "piqp/sparse/kkt.hpp"

namespace piqp
{

namespace sparse
{

extern template struct KKTImpl<KKT<common::Scalar, common::StorageIndex, KKTMode::KKT_FULL, common::sparse::Ordering>, common::Scalar, common::StorageIndex, KKTMode::KKT_FULL>;
extern template struct KKT<common::Scalar, common::StorageIndex>;

} // namespace sparse

} // namespace piqp

#endif // PIQP_SPARSE_KKT_TPP

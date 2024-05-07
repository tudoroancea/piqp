// This file is part of PIQP.
//
// Copyright (c) 2023 EPFL
// Copyright (c) 2022 INRIA
//
// This source code is licensed under the BSD 2-Clause License found in the
// LICENSE file in the root directory of this source tree.

#define PIQP_EIGEN_CHECK_MALLOC

#include "gtest/gtest.h"
#include "piqp/piqp.hpp"
#include "piqp/utils/random_utils.hpp"
#include "utils.hpp"

using namespace piqp;
using namespace piqp::sparse;

using T = double;
using I = int;

using kkt_types = testing::Types<KKT<T, I, KKTMode::KKT_FULL>,
                                 KKT<T, I, KKTMode::KKT_EQ_ELIMINATED>,
                                 KKT<T, I, KKTMode::KKT_INEQ_ELIMINATED>,
                                 KKT<T, I, KKTMode::KKT_ALL_ELIMINATED>>;
template<typename T>
class SparseKKTTest : public ::testing::Test
{};
TYPED_TEST_SUITE(SparseKKTTest, kkt_types);

TYPED_TEST(SparseKKTTest, UpdateScalings)
{
    isize dim = 10;
    isize n_eq = 8;
    isize n_ineq = 9;
    T sparsity_factor = 0.2;

    Model<T, I> qp_model = rand::sparse_strongly_convex_qp<T, I>(dim, n_eq, n_ineq, sparsity_factor);
    Data<T, I> data(qp_model);
    Settings<T> settings;

    // make sure P_utri has not complete diagonal filled
    data.P_utri.coeffRef(1, 1) = 0;
    data.P_utri.prune(0.0);

    T rho = 0.9;
    T delta = 1.2;

    TypeParam kkt(data, settings);
    kkt.init(rho, delta);

    rho = 0.8;
    delta = 0.2;
    Vec<T> s(n_ineq);
    s.setConstant(1);
    Vec<T> s_lb(dim);
    s_lb.setConstant(1);
    Vec<T> s_ub(dim);
    s_ub.setConstant(1);
    Vec<T> z(n_ineq);
    z.setConstant(1);
    Vec<T> z_lb(dim);
    z_lb.setConstant(1);
    Vec<T> z_ub(dim);
    z_ub.setConstant(1);

    PIQP_EIGEN_MALLOC_NOT_ALLOWED();
    kkt.update_scalings(rho, delta, s, s_lb, s_ub, z, z_lb, z_ub);
    PIQP_EIGEN_MALLOC_ALLOWED();

    // assert PKPt matrix is upper triangular
    SparseMat<T, I> PKPt_upper = kkt.PKPt.template triangularView<Eigen::Upper>();
    assert_sparse_matrices_equal(kkt.PKPt, PKPt_upper);

    TypeParam kkt2(data, settings);
    kkt2.init(rho, delta);

    // assert update was correct, i.e. it's the same as a freshly initialized one
    EXPECT_TRUE(kkt.PKPt.isApprox(kkt2.PKPt, 1e-8));
}

TYPED_TEST(SparseKKTTest, UpdateData)
{
    isize dim = 10;
    isize n_eq = 8;
    isize n_ineq = 9;
    T sparsity_factor = 0.2;

    Model<T, I> qp_model = rand::sparse_strongly_convex_qp<T, I>(dim, n_eq, n_ineq, sparsity_factor);
    Data<T, I> data(qp_model);
    Settings<T> settings;

    // make sure P_utri has not complete diagonal filled
    data.P_utri.coeffRef(1, 1) = 0;
    data.P_utri.prune(0.0);

    T rho = 0.9;
    T delta = 1.2;

    TypeParam kkt(data, settings);
    kkt.init(rho, delta);

    // update data
    Eigen::Map<Vec<T>>(data.P_utri.valuePtr(), data.P_utri.nonZeros()) = rand::vector_rand<T>(data.P_utri.nonZeros());
    Eigen::Map<Vec<T>>(data.AT.valuePtr(), data.AT.nonZeros()) = rand::vector_rand<T>(data.AT.nonZeros());
    Eigen::Map<Vec<T>>(data.GT.valuePtr(), data.GT.nonZeros()) = rand::vector_rand<T>(data.GT.nonZeros());
    int update_options = KKTUpdateOptions::KKT_UPDATE_P | KKTUpdateOptions::KKT_UPDATE_A | KKTUpdateOptions::KKT_UPDATE_G;

    PIQP_EIGEN_MALLOC_NOT_ALLOWED();
    kkt.update_data(update_options);
    PIQP_EIGEN_MALLOC_ALLOWED();

    // assert PKPt matrix is upper triangular
    SparseMat<T, I> PKPt_upper = kkt.PKPt.template triangularView<Eigen::Upper>();
    assert_sparse_matrices_equal(kkt.PKPt, PKPt_upper);

    TypeParam kkt2(data, settings);
    kkt2.init(rho, delta);

    // assert update was correct, i.e. it's the same as a freshly initialized one
    EXPECT_TRUE(kkt.PKPt.isApprox(kkt2.PKPt, 1e-8));
}

TYPED_TEST(SparseKKTTest, FactorizeSolve)
{
    isize dim = 20;
    isize n_eq = 8;
    isize n_ineq = 9;
    T sparsity_factor = 0.2;

    Model<T, I> qp_model = rand::sparse_strongly_convex_qp<T, I>(dim, n_eq, n_ineq, sparsity_factor);
    Data<T, I> data(qp_model);
    Settings<T> settings;

    T rho = 0.9;
    T delta = 1.2;

    TypeParam kkt(data, settings);
    kkt.init(rho, delta);

    PIQP_EIGEN_MALLOC_NOT_ALLOWED();
    ASSERT_TRUE(kkt.regularize_and_factorize(false));
    PIQP_EIGEN_MALLOC_ALLOWED();

    Vec<T> rhs_x = rand::vector_rand<T>(dim);
    Vec<T> rhs_y = rand::vector_rand<T>(n_eq);
    Vec<T> rhs_z = rand::vector_rand<T>(n_ineq);
    Vec<T> rhs_z_lb = rand::vector_rand<T>(dim);
    Vec<T> rhs_z_ub = rand::vector_rand<T>(dim);
    Vec<T> rhs_s = rand::vector_rand<T>(n_ineq);
    Vec<T> rhs_s_lb = rand::vector_rand<T>(dim);
    Vec<T> rhs_s_ub = rand::vector_rand<T>(dim);

    Vec<T> delta_x(dim);
    Vec<T> delta_y(n_eq);
    Vec<T> delta_z(n_ineq);
    Vec<T> delta_z_lb(dim);
    Vec<T> delta_z_ub(dim);
    Vec<T> delta_s(n_ineq);
    Vec<T> delta_s_lb(dim);
    Vec<T> delta_s_ub(dim);

    PIQP_EIGEN_MALLOC_NOT_ALLOWED();
    kkt.solve(rhs_x, rhs_y, rhs_z, rhs_z_lb, rhs_z_ub, rhs_s, rhs_s_lb, rhs_s_ub,
              delta_x, delta_y, delta_z, delta_z_lb, delta_z_ub, delta_s, delta_s_lb, delta_s_ub,
              false);
    PIQP_EIGEN_MALLOC_ALLOWED();

    Vec<T> rhs_x_sol(dim);
    Vec<T> rhs_y_sol(n_eq);
    Vec<T> rhs_z_sol(n_ineq);
    Vec<T> rhs_z_lb_sol(dim);
    Vec<T> rhs_z_ub_sol(dim);
    Vec<T> rhs_s_sol(n_ineq);
    Vec<T> rhs_s_lb_sol(dim);
    Vec<T> rhs_s_ub_sol(dim);

    PIQP_EIGEN_MALLOC_NOT_ALLOWED();
    kkt.multiply(delta_x, delta_y, delta_z, delta_z_lb, delta_z_ub, delta_s, delta_s_lb, delta_s_ub,
                 rhs_x_sol, rhs_y_sol, rhs_z_sol, rhs_z_lb_sol, rhs_z_ub_sol, rhs_s_sol, rhs_s_lb_sol, rhs_s_ub_sol);
    PIQP_EIGEN_MALLOC_ALLOWED();

    ASSERT_TRUE(rhs_x.isApprox(rhs_x_sol, 1e-8));
    ASSERT_TRUE(rhs_y.isApprox(rhs_y_sol, 1e-8));
    ASSERT_TRUE(rhs_z.isApprox(rhs_z_sol, 1e-8));
    ASSERT_TRUE(rhs_z_lb.head(data.n_lb).isApprox(rhs_z_lb_sol.head(data.n_lb), 1e-8));
    ASSERT_TRUE(rhs_z_ub.head(data.n_ub).isApprox(rhs_z_ub_sol.head(data.n_ub), 1e-8));
    ASSERT_TRUE(rhs_s.isApprox(rhs_s_sol, 1e-8));
    ASSERT_TRUE(rhs_s_lb.head(data.n_lb).isApprox(rhs_s_lb_sol.head(data.n_lb), 1e-8));
    ASSERT_TRUE(rhs_s_ub.head(data.n_ub).isApprox(rhs_s_ub_sol.head(data.n_ub), 1e-8));
}

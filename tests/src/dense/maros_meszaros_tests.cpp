// This file is part of PIQP.
//
// Copyright (c) 2023 EPFL
// Copyright (c) 2022 INRIA
//
// This source code is licensed under the BSD 2-Clause License found in the
// LICENSE file in the root directory of this source tree.

#include <iostream>

#include "gtest/gtest.h"
#include "piqp/piqp.hpp"
#include "piqp/utils/filesystem.hpp"
#include "piqp/utils/io_utils.hpp"

using T = double;

class DenseMarosMeszarosTest : public testing::TestWithParam<std::string>
{};

TEST_P(DenseMarosMeszarosTest, CanSolveProblemKKTFull)
{
    std::string path = "maros_meszaros_data/" + GetParam();
    piqp::sparse::Model<T, int> sparse_model = piqp::load_sparse_model<T, int>(path);
    piqp::dense::Model<T> model = sparse_model.dense_model();

    piqp::DenseSolver<T> solver;
    solver.settings().verbose = true;
    solver.setup(model.P, model.c,
                 model.A, model.b,
                 model.G, model.h,
                 model.x_lb, model.x_ub);

    piqp::Status status = solver.solve();
    ASSERT_EQ(status, piqp::Status::PIQP_SOLVED);
}

std::vector<std::string> get_maros_meszaros_problems()
{
    std::vector<std::string> problem_names;
    for (const auto& entry : piqp::fs::directory_iterator("maros_meszaros_data"))
    {
        std::string file_name = entry.path().filename().string();
        if (file_name == "README.md" || file_name == "LICENSE") continue;

        std::string path = "maros_meszaros_data/" + file_name;
        piqp::sparse::Model<T, int> model = piqp::load_sparse_model<T, int>(path);

        // only solve small problems
        if (model.P.rows() <= 1000 && model.A.rows() + model.G.rows() <= 1000)
        {
            problem_names.push_back(file_name);
        }
    }
    return problem_names;
}

INSTANTIATE_TEST_SUITE_P(
        FromFolder,
        DenseMarosMeszarosTest,
        ::testing::ValuesIn(get_maros_meszaros_problems()),
        //    ::testing::Values("QBEACONF"),
        [](const ::testing::TestParamInfo<std::string>& info)
        {
            piqp::usize i = info.param.find(".");
            std::string name = info.param.substr(0, i);
            std::replace(name.begin(), name.end(), '-', '_');
            return name;
        });

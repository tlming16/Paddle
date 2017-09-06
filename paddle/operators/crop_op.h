/* Copyright (c) 2016 CropdleCropdle Authors. All Rights Reserve.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#pragma once

#include "paddle/framework/eigen.h"
#include "paddle/framework/op_registry.h"

namespace paddle {
namespace operators {

template <typename T, size_t D, int MajorType = Eigen::RowMajor,
          typename IndexType = Eigen::DenseIndex>
using EigenTensor = framework::EigenTensor<T, D, MajorType, IndexType>;

using Tensor = framework::Tensor;

template <typename Place, typename T, size_t D>
void CropFunction(const framework::ExecutionContext& context) {
  auto* x = context.Input<Tensor>("X");
  auto* out = context.Output<Tensor>("Out");
  out->mutable_data<T>(context.GetPlace());
  auto x_dims = x->dims();
  auto out_dims = out->dims();

  auto offsets = context.op().GetAttr<std::vector<int>>("offsets");
  PADDLE_ENFORCE_EQ(
      x_dims.size(), offsets.size(),
      "Offsets size should be equal to dimension size of input tensor.");

  Eigen::array<std::pair<int, int>, D> paddings;
  for (size_t i = 0; i < D; ++i) {
    paddings[i].first = -(offsets[i]);
    paddings[i].second = -(x_dims[i] - out_dims[i] - offsets[i]);
  }

  auto x_tensor = EigenTensor<T, D>::From(*x);
  auto out_tensor = EigenTensor<T, D>::From(*out);
  auto place = context.GetEigenDevice<Place>();
  out_tensor.device(place) = x_tensor.pad(paddings, 0);
}

template <typename Place, typename T>
class CropKernel : public framework::OpKernel {
 public:
  void Compute(const framework::ExecutionContext& context) const override {
    int dim = context.Input<Tensor>("X")->dims().size();
    switch (dim) {
      case 1:
        CropFunction<Place, T, 1>(context);
        break;
      case 2:
        CropFunction<Place, T, 2>(context);
        break;
      case 3:
        CropFunction<Place, T, 3>(context);
        break;
      case 4:
        CropFunction<Place, T, 4>(context);
        break;
      case 5:
        CropFunction<Place, T, 5>(context);
        break;
      case 6:
        CropFunction<Place, T, 6>(context);
        break;
      default:
        LOG(ERROR) << "Only ranks up to 6 supported.";
    }
  }
};

template <typename Place, typename T, size_t D>
void CropGradFunction(const framework::ExecutionContext& context) {
  auto* d_out = context.Input<Tensor>(framework::GradVarName("Out"));
  auto* d_x = context.Output<Tensor>(framework::GradVarName("X"));
  d_x->mutable_data<T>(context.GetPlace());
  auto d_x_dims = d_x->dims();
  auto d_out_dims = d_out->dims();

  auto offsets = context.op().GetAttr<std::vector<int>>("offsets");

  Eigen::array<std::pair<int, int>, D> paddings;
  for (int i = 0; i < d_out_dims.size(); ++i) {
    paddings[i].first = offsets[i];
    paddings[i].second = d_x_dims[i] - d_out_dims[i] - offsets[i];
  }

  auto d_x_tensor = EigenTensor<T, D>::From(*d_x);
  auto d_out_tensor = EigenTensor<T, D>::From(*d_out);
  auto place = context.GetEigenDevice<Place>();
  d_x_tensor.device(place) = d_out_tensor.pad(paddings, 0);
}

template <typename Place, typename T>
class CropGradKernel : public framework::OpKernel {
 public:
  void Compute(const framework::ExecutionContext& context) const override {
    size_t dim =
        context.Input<Tensor>(framework::GradVarName("Out"))->dims().size();
    switch (dim) {
      case 1:
        CropGradFunction<Place, T, 1>(context);
        break;
      case 2:
        CropGradFunction<Place, T, 2>(context);
        break;
      case 3:
        CropGradFunction<Place, T, 3>(context);
        break;
      case 4:
        CropGradFunction<Place, T, 4>(context);
        break;
      case 5:
        CropGradFunction<Place, T, 5>(context);
        break;
      case 6:
        CropGradFunction<Place, T, 6>(context);
        break;
      default:
        LOG(ERROR) << "Only ranks up to 6 supported.";
    }
  }
};

}  // namespace operators
}  // namespace paddle

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "boost/scoped_ptr.hpp"
#include "gtest/gtest.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/filler.hpp"
#include "caffe/layers/center_loss_layer.hpp"

#include "caffe/test/test_caffe_main.hpp"
#include "caffe/test/test_gradient_check_util.hpp"


namespace caffe{

template <typename TypeParam>
class CenterLossLayerTest : public MultiDeviceTest<TypeParam> {
     typedef typename TypeParam::Dtype Dtype;
     protected:
      CenterLossLayerTest()
                : blob_bottom_data_(new Blob<Dtype>(20,10,1,1)),
                  blob_bottom_label_(new Blob<Dtype>(20,1,1,1)),
                  blob_top_loss_(new Blob<Dtype>()) {
             FillerParameter filler_param;
             filler_param.set_std(1);
             GaussianFiller<Dtype> filler(filler_param);
             filler.Fill(this->blob_bottom_data_);
             blob_bottom_vec_.push_back(blob_bottom_data_);
             for(int i = 0; i < blob_bottom_label_->count(); i++) {
                 blob_bottom_label_->mutable_cpu_data()[i] = caffe_rng_rand() % 2;
             }
             blob_bottom_vec_.push_back(blob_bottom_label_);
             blob_top_vec_.push_back(blob_top_loss_);
      }
      virtual ~CenterLossLayerTest() {
             delete blob_bottom_data_;
             delete blob_bottom_label_;
             delete blob_top_loss_;
      }
      Blob<Dtype>* const blob_bottom_data_;
      Blob<Dtype>* const blob_bottom_label_;
      Blob<Dtype>* const blob_top_loss_;
      vector<Blob<Dtype>*> blob_bottom_vec_;
      vector<Blob<Dtype>*> blob_top_vec_;
};

TYPED_TEST_CASE(CenterLossLayerTest, TestDtypesAndDevices);

TYPED_TEST(CenterLossLayerTest, TestGradient){
   typedef typename TypeParam::Dtype Dtype;
   LayerParameter layer_param;
   layer_param.add_loss_weight(1);
   CenterLossLayer<Dtype> layer(layer_param);
   GradientChecker<Dtype> checker(1e-4, 1e-2);
   checker.CheckGradientExhaustive(&layer, this->blob_bottom_vec_, this->blob_top_vec_, 0);

}

}

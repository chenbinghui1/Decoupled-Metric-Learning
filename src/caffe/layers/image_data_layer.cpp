#ifdef USE_OPENCV
#include <opencv2/core/core.hpp>

#include <fstream>  // NOLINT(readability/streams)
#include <iostream>  // NOLINT(readability/streams)
#include <string>
#include <utility>
#include <vector>

#include "caffe/data_transformer.hpp"
#include "caffe/layers/base_data_layer.hpp"
#include "caffe/layers/image_data_layer.hpp"
#include "caffe/util/benchmark.hpp"
#include "caffe/util/io.hpp"
#include "caffe/util/math_functions.hpp"
#include "caffe/util/rng.hpp"


namespace caffe {

template <typename Dtype>
ImageDataLayer<Dtype>::~ImageDataLayer<Dtype>() {
  this->StopInternalThread();
}

template <typename Dtype>
void ImageDataLayer<Dtype>::DataLayerSetUp(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
  const int new_height = this->layer_param_.image_data_param().new_height();
  const int new_width  = this->layer_param_.image_data_param().new_width();
  const bool is_color  = this->layer_param_.image_data_param().is_color();
  string root_folder = this->layer_param_.image_data_param().root_folder();
  const int img_num = this->layer_param_.image_data_param().img_num();
  const int label_num = this->layer_param_.image_data_param().label_num();


  CHECK((new_height == 0 && new_width == 0) ||
      (new_height > 0 && new_width > 0)) << "Current implementation requires "
      "new_height and new_width to be set at the same time.";
  // Read the file with filenames and labels
  const string& source = this->layer_param_.image_data_param().source();
  LOG(INFO) << "Opening file " << source;
  std::ifstream infile(source.c_str());

  vector<string> filename(img_num);
  vector<int> label(label_num);
 // string filename1,filename2;
 // int label1,label2,label_smi;
  while (infile >> filename[0]) {
    for(int i=1;i<img_num;i++){
        infile >> filename[i];
     }
    for(int i=0;i<label_num;i++){
        infile >> label[i];
     }
    lines_.push_back(std::make_pair(filename,label));
  }

  if (this->layer_param_.image_data_param().shuffle()) {
    // randomly shuffle data
    LOG(INFO) << "Shuffling data";
    const unsigned int prefetch_rng_seed = caffe_rng_rand();
    prefetch_rng_.reset(new Caffe::RNG(prefetch_rng_seed));
    ShuffleImages();
  }
  LOG(INFO) << "A total of " << lines_.size() << " images.";

  lines_id_ = 0;
  // Check if we would need to randomly skip a few data points
  if (this->layer_param_.image_data_param().rand_skip()) {
    unsigned int skip = caffe_rng_rand() %
        this->layer_param_.image_data_param().rand_skip();
    LOG(INFO) << "Skipping first " << skip << " data points.";
    CHECK_GT(lines_.size(), skip) << "Not enough points to skip";
    lines_id_ = skip;
  }
  // Read an image, and use it to initialize the top blob.
  cv::Mat cv_img = ReadImageToCVMat(root_folder + lines_[lines_id_].first[0],
                                    new_height, new_width, is_color);
  const int channels = cv_img.channels()*img_num;
  const int height = cv_img.rows;
  const int width = cv_img.cols;
  CHECK(cv_img.data) << "Could not load " << lines_[lines_id_].first[0];
  // Use data_transformer to infer the expected blob shape from a cv_image.
 vector<int> top_shape = this->data_transformer_->InferBlobShape(cv_img);
//LOG(INFO) << top_shape[0] << top_shape[1] << top_shape[2] << top_shape[3];

  // Reshape prefetch_data and top[0] according to the batch_size.
  const int batch_size = this->layer_param_.image_data_param().batch_size();
  const int crop_size = this->layer_param_.transform_param().crop_size();
  int crop_width = this->layer_param_.transform_param().crop_width();
  int crop_height = this->layer_param_.transform_param().crop_height();
  crop_width = crop_width > 0 ? crop_width : crop_size;
  crop_height = crop_height > 0 ? crop_height : crop_size;
  
  CHECK_GT(batch_size, 0) << "Positive batch size required";
  top_shape[0] = batch_size;
//LOG(INFO)<<this->PREFETCH_COUNT<<std::endl;
  if(crop_height > 0 && crop_width > 0){
    this->transformed_data_.Reshape(1, channels/img_num, crop_height, crop_width);
    for (int i = 0; i < this->PREFETCH_COUNT; ++i) {
       this->prefetch_[i].data_.Reshape(batch_size, channels, crop_height, crop_width);
      }
    top[0]->Reshape(batch_size, channels, crop_height, crop_width);
  }else{
    this->transformed_data_.Reshape(1, channels/img_num, height, width);
    for (int i = 0; i < this->PREFETCH_COUNT; ++i) {
       this->prefetch_[i].data_.Reshape(batch_size, channels, height, width);
      }
    top[0]->Reshape(batch_size, channels, height, width);
   }
  LOG(INFO) << "output data size: " << top[0]->num() << ","
      << top[0]->channels() << "," << top[0]->height() << ","
      << top[0]->width();
  // label
  vector<int> label_shape(1, batch_size);
  top[1]->Reshape(batch_size, label_num, 1, 1);//3 1 1
  for (int i = 0; i < this->PREFETCH_COUNT; ++i) {
    this->prefetch_[i].label_.Reshape(batch_size, label_num, 1, 1);//3 1 1
  }
}

template <typename Dtype>
void ImageDataLayer<Dtype>::ShuffleImages() {
  caffe::rng_t* prefetch_rng =
      static_cast<caffe::rng_t*>(prefetch_rng_->generator());
  shuffle(lines_.begin(), lines_.end(), prefetch_rng);
}

// This function is called on prefetch thread
template <typename Dtype>
void ImageDataLayer<Dtype>::load_batch(Batch<Dtype>* batch) {
  CPUTimer batch_timer;
  batch_timer.Start();
  double read_time = 0;
  double trans_time = 0;
  CPUTimer timer;
  CHECK(batch->data_.count());
  CHECK(this->transformed_data_.count());
  ImageDataParameter image_data_param = this->layer_param_.image_data_param();
  const int batch_size = image_data_param.batch_size();
  const int new_height = image_data_param.new_height();
  const int new_width = image_data_param.new_width();
  const bool is_color = image_data_param.is_color();
  string root_folder = image_data_param.root_folder();
  const int img_num = this->layer_param_.image_data_param().img_num();
  const int label_num = this->layer_param_.image_data_param().label_num();
  const int crop_size = this->layer_param_.transform_param().crop_size();
  int crop_width = this->layer_param_.transform_param().crop_width();
  int crop_height = this->layer_param_.transform_param().crop_height();
  crop_width = crop_width > 0 ? crop_width : crop_size;
  crop_height = crop_height > 0 ? crop_height : crop_size;

  // Reshape according to the first image of each batch
  // on single input batches allows for inputs of varying dimension.
  
  for(int i=0;i<img_num;i++){
    cv::Mat cv_img = ReadImageToCVMat(root_folder + lines_[lines_id_].first[i],
      new_height, new_width, is_color);
    CHECK(cv_img.data) << "Could not load " << lines_[lines_id_].first[i];
  }

  // Use data_transformer to infer the expected blob shape from a cv_img.
  cv::Mat cv_img1 = ReadImageToCVMat(root_folder + lines_[lines_id_].first[0],
  new_height, new_width, is_color);
  vector<int> top_shape = this->data_transformer_->InferBlobShape(cv_img1);
  const int channels1 = cv_img1.channels()*img_num;
  const int height1 = cv_img1.rows;
  const int width1 = cv_img1.cols;

    top_shape[0] = batch_size;
  if(crop_width > 0 && crop_height > 0){
 this->transformed_data_.Reshape(batch_size,channels1/img_num,crop_height,crop_width);
  // Reshape batch according to the batch_size.
    batch->data_.Reshape(batch_size,channels1,crop_height,crop_width);
  }else {
    this->transformed_data_.Reshape(batch_size,channels1/img_num,height1,width1);
  // Reshape batch according to the batch_size.
    batch->data_.Reshape(batch_size,channels1,height1,width1);
  }
  Dtype* prefetch_data = batch->data_.mutable_cpu_data();
  Dtype* prefetch_label = batch->label_.mutable_cpu_data();

  // datum scales
  const int lines_size = lines_.size();
  for (int item_id = 0; item_id < batch_size; ++item_id) {
    // get a blob
    timer.Start();
    CHECK_GT(lines_size, lines_id_);

  vector<cv::Mat> cv_img(img_num);
  for(int i=0;i<img_num;i++){
    cv_img[i] = ReadImageToCVMat(root_folder + lines_[lines_id_].first[i],
      new_height, new_width, is_color);
    CHECK(cv_img[i].data) << "Could not load " << lines_[lines_id_].first[i];
  }
    read_time += timer.MicroSeconds();
    timer.Start();
    // Apply transformations (mirror, crop...) to the image

    int offset = batch->data_.offset(item_id);
    for(int i=0;i<img_num;i++){
        this->transformed_data_.set_cpu_data(prefetch_data + offset);
        this->data_transformer_->Transform(cv_img[i], &(this->transformed_data_));
        if(i<img_num-1)offset = batch->data_.offset(item_id,is_color?3*(i+1):(i+1));
    }
/*
    this->transformed_data_.set_cpu_data(prefetch_data + offset);
    this->data_transformer_->Transform(cv_img1, &(this->transformed_data_));
         offset = batch->data_.offset(item_id,3);
    this->transformed_data_.set_cpu_data(prefetch_data + offset);
    this->data_transformer_->Transform(cv_img2, &(this->transformed_data_));
  */
    trans_time += timer.MicroSeconds();


    for(int i = 0;i < label_num; i++) {
        prefetch_label[item_id*label_num+i] = lines_[lines_id_].second[i];
    }

    // go to the next iter
    lines_id_++;
    if (lines_id_ >= lines_size) {
      // We have reached the end. Restart from the first.
      DLOG(INFO) << "Restarting data prefetching from start.";
      lines_id_ = 0;
      if (this->layer_param_.image_data_param().shuffle()) {
        ShuffleImages();
      }
    }
  }
  batch_timer.Stop();
  DLOG(INFO) << "Prefetch batch: " << batch_timer.MilliSeconds() << " ms.";
  DLOG(INFO) << "     Read time: " << read_time / 1000 << " ms.";
  DLOG(INFO) << "Transform time: " << trans_time / 1000 << " ms.";
}

INSTANTIATE_CLASS(ImageDataLayer);
REGISTER_LAYER_CLASS(ImageData);

} // namespace caffe
#endif  // USE_OPENCV


#include <algorithm>

#include "stdio.h"
#include "caffe/layers/act_loss.hpp"
#include "caffe/util/io.hpp"
#include "caffe/util/math_functions.hpp"
#include <cmath>
namespace caffe {

template <typename Dtype>
void ActLossLayer<Dtype>::LayerSetUp(
  const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  LossLayer<Dtype>::LayerSetUp(bottom, top);
  CHECK_EQ(bottom[0]->height(), 1);
  CHECK_EQ(bottom[0]->width(), 1);
  F_iter_size_ = 0;
  B_iter_size_ = 0;
  for(int i = 0; i < bottom.size(); i++){
        for(int j = i+1; j < bottom.size(); j++)
        temp_.push_back(vector<Dtype>(bottom[i]->channels()*bottom[j]->channels(),Dtype(0.)));//存储临时差量
  }
}
template <typename Dtype>
void ActLossLayer<Dtype>::Reshape(
  const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
  CHECK_EQ(bottom[0]->height(), 1);
  CHECK_EQ(bottom[0]->width(), 1);
  top[0]->Reshape(1,1,1,1);
}
template <typename Dtype>
void ActLossLayer<Dtype>::Forward_cpu(
    const vector<Blob<Dtype>*>& bottom,
    const vector<Blob<Dtype>*>& top) {
    int num = bottom[0]->num();//
    int M = bottom.size();//number of learners
    top[0]->mutable_cpu_data()[0]=Dtype(0.);
    //static int iter_size = 0;
    if(!this->layer_param_.act_loss_param().perturbation())
    {
        //正常
        for(int i = 0; i < M; i++)
        {
                caffe_set(bottom[i]->count(),Dtype(0.),bottom[i]->mutable_cpu_diff());
        }
        Dtype loss = 0.;
        for(int n = 0; n < num; n++)
        {
                Dtype loss_temp=1.0;
                
                for(int i = 0; i < M; i++)
                {
                        loss_temp = loss_temp * caffe_cpu_dot(bottom[i]->channels(), bottom[i]->cpu_data() + n * bottom[i]->channels(), bottom[i]->cpu_data() + n * bottom[i]->channels())/bottom[i]->channels();
                        
                }
                loss= loss + loss_temp;
        }
        top[0]->mutable_cpu_data()[0] = loss/num;
    
        }
    else {
        //配合itersize 第一次正常第二次不计算
        if(F_iter_size_ == 0)
        {
                for(int i = 0; i < M; i++)
        {
                caffe_set(bottom[i]->count(),Dtype(0.),bottom[i]->mutable_cpu_diff());
        }
        Dtype loss = 0.;
        for(int n = 0; n < num; n++)
        {
                Dtype loss_temp=1.0;
                
                for(int i = 0; i < M; i++)
                {
                        loss_temp = loss_temp * caffe_cpu_dot(bottom[i]->channels(), bottom[i]->cpu_data() + n * bottom[i]->channels(), bottom[i]->cpu_data() + n * bottom[i]->channels())/bottom[i]->channels();
                        
                }
                loss= loss + loss_temp;
        }
        top[0]->mutable_cpu_data()[0] = loss/num;
    
                F_iter_size_ = 1;
        }else{
                top[0]->mutable_cpu_data()[0] = Dtype(0);
                F_iter_size_ = 0;
        }
    }
}

     

template <typename Dtype>
void ActLossLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
	const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) 
{
    //update gradients
    vector<Dtype> sum_temp(bottom.size(),Dtype(0.));
    //static int iter_size = 0;
    if(!this->layer_param_.act_loss_param().perturbation()){
        for(int n=0; n<bottom[0]->num(); n++)
        {
        //compute each learner's norm sum
                for(int i=0; i<bottom.size(); i++)
                {
                        sum_temp[i] = caffe_cpu_dot(bottom[i]->channels(), bottom[i]->cpu_data()+n*bottom[i]->channels(), bottom[i]->cpu_data()+n*bottom[i]->channels())/bottom[i]->channels();
                
                }
                for(int i=0; i<bottom.size(); i++)
                {
                        Dtype temp = 1.0;
                        for(int j = 0; j < bottom.size(); j++)
                        {
                                if(i!=j)
                                        temp = temp * sum_temp[j];
                        }
                        temp = temp * top[0]->cpu_diff()[0] * Dtype(2)/bottom[0]->num()/bottom[i]->channels();
                        caffe_cpu_axpby(bottom[i]->channels(), temp, bottom[i]->cpu_data() + n * bottom[i]->channels(), Dtype(1), bottom[i]->mutable_cpu_diff() + n * bottom[i]->channels());
                
                }
        
        }
    }else
    {
        if(B_iter_size_ == 0)
        {
        for(int n=0; n<bottom[0]->num(); n++)
        {
        //compute each learner's norm sum
                for(int i=0; i<bottom.size(); i++)
                {
                        sum_temp[i] = caffe_cpu_dot(bottom[i]->channels(), bottom[i]->cpu_data()+n*bottom[i]->channels(), bottom[i]->cpu_data()+n*bottom[i]->channels())/bottom[i]->channels();
                
                }
                for(int i=0; i<bottom.size(); i++)
                {
                        Dtype temp = 1.0;
                        for(int j = 0; j < bottom.size(); j++)
                        {
                                if(i!=j)
                                        temp = temp * sum_temp[j];
                        }
                        temp = temp * top[0]->cpu_diff()[0] * Dtype(2)/bottom[0]->num()/bottom[i]->channels();
                        caffe_cpu_axpby(bottom[i]->channels(), temp, bottom[i]->cpu_data() + n * bottom[i]->channels(), Dtype(1), bottom[i]->mutable_cpu_diff() + n * bottom[i]->channels());
                
                }
        
        }
        B_iter_size_ = 1;
        }else
        {
                for(int i = 0; i < bottom.size(); i++)
                {
                caffe_set(bottom[i]->count(),Dtype(0.),bottom[i]->mutable_cpu_diff());
                }
                B_iter_size_ = 0;
        }
    }
    
}




#ifdef CPU_ONLY
STUB_GPU(ActLossLayer);
#endif

INSTANTIATE_CLASS(ActLossLayer);
REGISTER_LAYER_CLASS(ActLoss);

}  // namespace caffe


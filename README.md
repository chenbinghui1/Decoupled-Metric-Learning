## Code for CVPR 2019 Paper "[Hybrid-Attention based Decoupled Metric Learning for Zero-Shot Image Retrieval](http://bhchen.cn)"

This code is developed based on [Caffe](https://github.com/BVLC/caffe/).

* [Updates](#updates)
* [Files](#files)
* [Prerequisites](#prerequisites)
* [Train_Model](#train_model)
* [Extract_DeepFeature](#extract_deepfeature)
* [Evaluation](#evaluation)
* [Contact](#contact)
* [Citation](#citation)
* [LICENSE](#license)

### Updates
- Apr 02, 2019
  * The code and training prototxt for our [CVPR19](http://bhchen.cn) paper are released.
  * For simplication, we use Hinge_Loss-like adversarial constraint instead of the original adversary network (This will reduce the final performance a little but ease the training. We also provide the "gradients_reverse_layer" for the implementation of the orginal adversary network, you can try it by yourself.).
  * If you train our Network on **CUB-200**, the expected retrieval performance of **DeML(I=3,J=3)** will be ~**(R@1=64.7, R@2=75.1, R@4=83.2, R@8=89.4)** at ~10k iterations.

### Files
- Original Caffe library
- BinomialLoss
  * src/caffe/proto/caffe.proto
  * include/caffe/layers/Binomial_loss_layer.hpp
  * src/caffe/layers/Binomial_loss_layer.cpp
- ActLoss
  * src/caffe/proto/caffe.proto
  * include/caffe/layers/act_loss.hpp
  * src/caffe/layers/act_loss.cpp
- OAM
  * src/caffe/proto/caffe.proto
  * include/caffe/layers/proposal_crop_layer.hpp (For single convolution input)
  * src/caffe/layers/proposal_crop_layer.cpp (For single convolution input)
  * include/caffe/layers/proposal_crop_layer.hpp (For multi convolution input)
  * src/caffe/layers/proposal_crop_layer.cpp (For multi convolution input)
- gradients_reverse
  * src/caffe/proto/caffe.proto
  * include/caffe/layers/gradients_reverse_layer.hpp
  * src/caffe/layers/gradients_reverse_layer.cpp
  * src/caffe/layers/gradients_reverse_layer.cu
- CUB
  * examples/CUB/U512  (This is for the implementation of baseline method U512)
  * examples/CUB/DeML3-3_512  (This is for the implementation of our method DeML(I=3,J=3))
  * examples/CUB/pre-trained-model
### Prerequisites
* Caffe
* Matlab (for evaluation)
* 2GPUs, each > 11G
### Train_Model
1. The Installation is completely the same as [Caffe](http://caffe.berkeleyvision.org/). Please follow the [installation instructions](http://caffe.berkeleyvision.org/installation.html). Make sure you have correctly installed before using our code. 
2. Download the training images **CUB** {[google drive](https://drive.google.com/open?id=1V_5tS4YgyMRxUM7QHINn7aRYizwjxwmC), [baidu](https://pan.baidu.com/s/1X4W1xucDBxZafITvPF8SXQ)(psw:w3vh)} and move it to $(your_path). The images are preprossed the same as [Lifted Loss](https://github.com/rksltnl/Deep-Metric-Learning-CVPR16/), i.e. with zero paddings.
3. Download the **training list**(400M) {[google drive](https://drive.google.com/open?id=1P2lUicV-nMchMU_aP6JbgzOibOG1o-F6), [baidu](https://pan.baidu.com/s/1-NH4rpkYwbLjR0tIkr30nA) (psw:xbct)}, and move it to folder "~/Hybrid-Attention-based-Decoupled-Metric-Learning-master/examples/CUB/". (Or you can create your own list by randomly selecting 65 classes with 2 samples each class.)
4. Download [googlenetV1](http://dl.caffe.berkeleyvision.org/bvlc_googlenet.caffemodel) (used for U512) and **3nets** {[google drive](https://drive.google.com/open?id=1boQISUyXaV77qCS0u5Nmlv6dckuN25mM), [baidu](https://pan.baidu.com/s/10Q1wPeHMYtXEJ5cqY9GgPg)(psw:dmev)} (used for DeML3-3_512) to folder "~/Hybrid-Attention-based-Decoupled-Metric-Learning-master/examples/CUB/pre-trained-model/". (each stream of 3nets model is intialized by the same googlenetV1 model)
5. Modify the images path by changing "root_folder" into $(your_path) in all *.prototxt .
6. Then you can train our baseline method **U512** and the proposed **DeML(I=3,J=3)** by running
```
        cd ~/Hybrid-Attention-based-Decoupled-Metric-Learning-master/examples/CUB/U512
        ./finetune_U512.sh
```     
and
```
        cd ~/Hybrid-Attention-based-Decoupled-Metric-Learning-master/examples/CUB/DeML3-3_512
        ./finetune_DeML3-3_512.sh
```     
the trained models are stored in folder "run_U512/" and "run_DeML3-3_512/" respectively.
### Extract_DeepFeature
1. run the following code for **U512** and **DeML(I=3,J=3)** respectively.
```
        cd ~/Hybrid-Attention-based-Decoupled-Metric-Learning-master/examples/CUB/U512
        ./extractfeatures.sh
```

```
        cd ~/Hybrid-Attention-based-Decoupled-Metric-Learning-master/examples/CUB/DeML3-3_512
        ./extractfeatures.sh
```
the feature files are stored at folder "features/"
### Evaluation
1. Run code in folder "~/Hybrid-Attention-based-Decoupled-Metric-Learning-master/evaluation/"
### Contact 
- [Binghui Chen](http://bhchen.cn)

### Citation
You are encouraged to cite the following papers if this work helps your research. 

    @inproceedings{chen2019hybrid,
      title={Hybrid-Attention based Decoupled metric Learning for Zero-Shot Image Retrieval},
      author={Chen, Binghui and Deng, Weihong},
      booktitle={Proceedings of the IEEE Conference on Computer Vision and Pattern Recognition (CVPR)},
      year={2019},
    }
    @InProceedings{chen2019energy,
    author = {Chen, Binghui and Deng, Weihong},
    title = {Energy Confused Adversarial Metric Learning for Zero-Shot Image Retrieval and Clustering},
    booktitle = {AAAI Conference on Artificial Intelligence},
    year = {2019}
    }
    @inproceedings{songCVPR16,
    Author = {Hyun Oh Song and Yu Xiang and Stefanie Jegelka and Silvio Savarese},
    Title = {Deep Metric Learning via Lifted Structured Feature Embedding},
    Booktitle = {Computer Vision and Pattern Recognition (CVPR)},
    Year = {2016}
    }
### License
Copyright (c) Binghui Chen

All rights reserved.

MIT License

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

***

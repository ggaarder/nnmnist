# nnmnist

A Neural Network based single-digit OCR, using the [MNIST dataset][mnist],
according to the tutorial Neural Network And Deep Learning's [Chapter 1][tut1] and [Chapter 2][tut2] written by [Michael Nielsen][mn] in Dec 2017.

[mnist]: http://yann.lecun.com/exdb/mnist/index.html
[tut1]:  http://neuralnetworksanddeeplearning.com/chap1.html
[tut2]:  http://neuralnetworksanddeeplearning.com/chap2.html
[mn]:    http://michaelnielsen.org/

## Network Storage Format

```
uint32 L; // number of layers, at least 3, that is one input layer, one hidden layer and one output layer

struct layer {
    uint32 ncnt; // number of neurons. 0 for the input layer
                 // number of weights is the number of the neurons of the last layer
    struct neuron {
        float32 w[layers[l-1].ncnt]; // weights
        float32 bias;
    } neurons[ncnt];
} layers[L];
```

#ifndef __L_NNP_H_INCLUDED_
#define __L_NNP_H_INCLUDED_
//
// nnp.obj - neural network perceptron (библиотека с нейросетью)
//

const unsigned long NNP_FF_BIN = 0x6e6962; //формат файла 'bin'
const unsigned long NNP_FF_JSON = 0x6e6f736a; //формат файла 'json'

struct Layer {
    unsigned long c_size; // число нейронов в текущем слое
	unsigned long n_size; // число нейронов на следующем слое
    double* neurons;
    double* biases;
    double** weights;
};

struct NeuralNetwork {
    double learningRate; // скорость обучения
    Layer* layers;       // слои
	long layers_length;  // число слоев
	double(_stdcall*activation)(double v); // указатель на функцию активации
	double(_stdcall*derivative)(double v); // указатель на функцию ???
	double* errors; // массив для вычислений
	double* errorsNext;
	double* gradients;
	double** deltas;
};

//
// nnp - import table
//
void (__stdcall* import_nnp)() = (void (__stdcall*)())&"lib_init";
void (_stdcall* NNP_Create)(NeuralNetwork* o, double learningRate, void* activation, void* derivative, unsigned long* sizes, long sizes_length) = (void (_stdcall*)(NeuralNetwork*, double, void*, void*, unsigned long*, long))&"NNP_Create";
double* (_stdcall* NNP_FeedForward)(NeuralNetwork* o, double* inputs) = (double* (_stdcall*)(NeuralNetwork*, double*))&"NNP_FeedForward";
void (_stdcall* NNP_BackPropagation)(NeuralNetwork* o, double* targets) = (void (_stdcall*)(NeuralNetwork*, double*))&"NNP_BackPropagation";
void (_stdcall* NNP_GetMemData)(NeuralNetwork* o, unsigned long fmt, void* m_data) = (void (_stdcall*)(NeuralNetwork*, unsigned long, void*))&"NNP_GetMemData";
long (_stdcall* NNP_SetMemData)(NeuralNetwork* o, unsigned long fmt, void* m_data) = (long (_stdcall*)(NeuralNetwork*, unsigned long, void*))&"NNP_SetMemData";
void (_stdcall* NNP_Destroy)(NeuralNetwork* o) = (void (_stdcall*)(NeuralNetwork*))&"NNP_Destroy";

asm{
	dd 0,0
}

#endif
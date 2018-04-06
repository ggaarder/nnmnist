all: nnmnist.c
	tcc $^ -lm -DNTWKFN='"ntwkarg"'

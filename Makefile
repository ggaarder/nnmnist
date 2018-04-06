# network arguments storage filename
NTWKFN=ntwkarg

all: nnmnist.c
	tcc $^ -lm -DNTWKFN='"$(NTWKFN)"'

debug: nnmnist.c
	tcc $^ -lm -g -DNTWKFN='"$(NTWKFN)"'

rmdat:
	rm -f $(NTWKFN)

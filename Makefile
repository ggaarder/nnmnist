# network arguments storage filename
NTWKFN=ntwkarg

all: nnmnist.c
	tcc $^ -lm -DNTWKFN='"$(NTWKFN)"'

rmdat:
	rm -f $(NTWKFN)

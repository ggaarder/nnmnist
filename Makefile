# network arguments storage filename
NTWKFN=ntwkarg

all: nnmnist.c
	tcc $^ -lm -DNTWKFN='"$(NTWKFN)"'

debug: nnmnist.c
	gcc $^ -lm -g -DNTWKFN='"$(NTWKFN)"' -O0

rmdat:
	rm -f $(NTWKFN)

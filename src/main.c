#include "gc.h"

#include <stdio.h>


void __attribute__ ((noinline)) allocate_small(){
	int* test = gc_malloc(sizeof(int));
	*test = 42;
}


void __attribute__ ((noinline)) allocate_large(){
	int* test = gc_malloc(sizeof(int) * 1000);
	*test = 42;
}



void test_small_allocations(int num_allocations){
	for(int i = 0; i < num_allocations; i++){
		allocate_small();
	}
}

void test_large_allocations(int num_allocations){
	for(int i = 0; i < num_allocations; i++){
		allocate_large();
	}
}

void test_small_and_large_allocations(int num_allocations){
	for(int i = 0; i < num_allocations; i++){
		if(i % 2  == 0){
			allocate_large();
		}
		else{
			allocate_small();
		}
	}
}



void bench_largeallo_smallarr(){
	if(gc_init() < 0){
		return;
	}

	int NUMBER = 300000;

	int* testarr[100];
	for(int i = 0; i < 100; i++){
		int* test = gc_malloc(sizeof(int));
		*test = 25;
		testarr[i] = test;
	}

	test_large_allocations(NUMBER);
	gc_collect();

	test_large_allocations(NUMBER);
	gc_collect();

	*testarr[0] = 25;
}

void bench_smallallo_smallarr(){
	if(gc_init() < 0){
		return;
	}

	int NUMBER = 300000;

	int* testarr[100];
	for(int i = 0; i < 100; i++){
		int* test = gc_malloc(sizeof(int));
		*test = 25;
		testarr[i] = test;
	}

	test_small_allocations(NUMBER);
	gc_collect();

	test_small_allocations(NUMBER);
	gc_collect();

	*testarr[0] = 25;
}

void bench_mixallo_smallarr(){
	if(gc_init() < 0){
		return;
	}

	int NUMBER = 300000;

	int* testarr[100];
	for(int i = 0; i < 100; i++){
		int* test = gc_malloc(sizeof(int));
		*test = 25;
		testarr[i] = test;
	}

	test_small_and_large_allocations(NUMBER);
	gc_collect();

	test_small_and_large_allocations(NUMBER);
	gc_collect();

	*testarr[0] = 25;
}



void bench_largeallo_noarr(){
	if(gc_init() < 0){
		return;
	}

	int NUMBER = 300000;

	test_large_allocations(NUMBER);
	gc_collect();

	test_large_allocations(NUMBER);
	gc_collect();
}

void bench_smallallo_noarr(){
	if(gc_init() < 0){
		return;
	}

	int NUMBER = 300000;

	test_small_allocations(NUMBER);
	gc_collect();

	test_small_allocations(NUMBER);
	gc_collect();
}

void bench_mixallo_noarr(){
	if(gc_init() < 0){
		return;
	}

	int NUMBER = 300000;

	test_small_and_large_allocations(NUMBER);
	gc_collect();

	test_small_and_large_allocations(NUMBER);
	gc_collect();
}



void bench_largeallo_largearr(){
	if(gc_init() < 0){
		return;
	}

	int* testarr[5000];
	for(int i = 0; i < 5000; i++){
		int* test = gc_malloc(sizeof(int));
		*test = 25;
		testarr[i] = test;
	}

	int NUMBER = 300000;

	test_large_allocations(NUMBER);
	gc_collect();

	test_large_allocations(NUMBER);
	gc_collect();

	*testarr[0] = 25;
}

void bench_smallallo_largearr(){
	if(gc_init() < 0){
		return;
	}

	int* testarr[5000];
	for(int i = 0; i < 5000; i++){
		int* test = gc_malloc(sizeof(int));
		*test = 25;
		testarr[i] = test;
	}

	int NUMBER = 300000;

	test_small_allocations(NUMBER);
	gc_collect();

	test_small_allocations(NUMBER);
	gc_collect();

	*testarr[0] = 25;
}

void bench_mixallo_largearr(){
	if(gc_init() < 0){
		return;
	}

	int* testarr[5000];
	for(int i = 0; i < 5000; i++){
		int* test = gc_malloc(sizeof(int));
		*test = 25;
		testarr[i] = test;
	}

	int NUMBER = 300000;

	test_small_and_large_allocations(NUMBER);
	gc_collect();

	test_small_and_large_allocations(NUMBER);
	gc_collect();

	*testarr[0] = 25;
}


int main() {
	bench_mixallo_largearr();

    return 0;
}
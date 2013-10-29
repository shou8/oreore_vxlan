#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#ifdef DEBUG
#include "test.h"
#endif /* DEBUG */

int main(int argc, char *argv[]){

#ifdef DEBUG
	test();
#endif /* DEUBG */

    return 0;
}


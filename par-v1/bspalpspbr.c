#include "bspedupack.h"


#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include <string.h>
#define N 10000


void main(){
 
 int* gen_graph(int n, double p); 
 int nloc(int p, int s, int n);
 void mainloop();


bsp_init(mainloop, 0, NULL );
mainloop();

}



//Description: generate random graph with integer weights of size n*n, with probability p
int* gen_graph(int n, double p)
{
	int i,j;
	int* l = calloc(n*n, sizeof(int));
	for ( j = 0; j < n; ++j) {
		for (i = 0; i < n; ++i)
		  if(((double) rand() / (RAND_MAX)) > p){
			l[j*n+i] = rand() % 1000;
		  }
			l[j*n+j] = 0;
		}	
	return l;
}



int nloc(int p, int s, int n){
    /* Compute number of rows of processor s for vector
       of length n block distributed over p processors. */
 	int t = ceil(((double)n/(double)p));
	if((s+1)*t > n ){
		if(s*t < n){
			 return (n - s*t); 
		}else{
			return 0;
		}
	}else{
		return  (ceil(t));
	}
} /* end nloc */



void mainloop(){

//int init[N*N] = {0,3,8,1000,-4, 1000,0,1000,1,7,1000,4,0,1000,1000,
//2,1000,-5,0,1000,1000,1000,1000,6,0};

int i,j,k,l,li,lj,lk,g,lsize, *linit, *linter;
int startrow, endrow;

int* init = gen_graph(N, 0.05);  


bsp_begin(bsp_nprocs());


/**********Initialization SuperStep 0***************/
lsize = nloc(bsp_nprocs(),bsp_pid(), N);	//Get the number of rows of processor s
linit = vecalloci(N*N); 				 	// Initialize the initial vector 
linter = vecalloci(N*N);					//Intermidiate matrix used from the repeated squaring algorithm 

bsp_push_reg(linter,N*N*SZINT);				//linter elements will be communicated



memcpy(linit,init,N*N*SZINT);
vecfreei(init);								//not needed anymore





/****Get the first and last global row of processor s***/
if(bsp_pid() == (bsp_nprocs() - 1)){
  startrow = (N - lsize);
  endrow = N;
}else{
  startrow = bsp_pid()*lsize;
  endrow = bsp_pid()*lsize + lsize;
}
/********************************************/

/**********End Initialization SuperStep***************/
bsp_sync();


double time0= bsp_time();
/*********Repeated Squaring loop start*************/
j=1;
while ((N-1) > j) {
 

       /****Comp. Superstep j0****/
	   //local update takes place here// 
	   for ( li = startrow; li < endrow; li++){
        	for ( lj=0; lj < N; lj++) {
				linter[N*li+lj] = 1000;
				for ( lk=0; lk < N; lk++) {
                        linter[N*li+lj] = fmin(linter[N*li+lj], linit[N*li+lk]+linit[N*lk+lj]); 
                }
        	}
    	}
 
        /****End Comp. Superstep j0****/ 
		bsp_sync();
		
		/****Comm. Superstep j1****/
		//processors give their values to all//
	 	for(g=0; g<bsp_nprocs();g++){
	 		bsp_put(g,&linter[startrow*N],&linter[0],startrow*N*SZINT,lsize*N*SZINT);
	 	}
		/****End Comm. Superstep j1****/
  		bsp_sync();

		/****Comp. Superstep j2****/ 
		memcpy(linit,linter,N*N*SZINT);

  		j=2*j;
		/****End Comp. Superstep j2****/ 
		bsp_sync();
}
/*********Repeated Squaring loop end*************/
double time1= bsp_time();
bsp_pop_reg(linter);
bsp_sync();
/*********display matrices and time*********/
if(bsp_pid()==0){
	printf( " \n Block Row Distr calculation of APSP took: %f seconds \n", time1-time0 ); 
}
/*for(g = 0; g < bsp_nprocs(); g++){
if(bsp_pid()==g){
 printf("\n i am proc %d and i have APSP Mat \n",bsp_pid());
  for(k=0;k<N;k++)
     {
	  printf("\n");
		 for(l=0;l<N;l++){
		    printf("\t %d",linit[N*k+l]);
			  }
			printf("\n \n ");
		}
	}
	bsp_sync();
}*/
//clean up
vecfreei(linit);
vecfreei(linter);

bsp_end();   
}


 

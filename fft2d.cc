// Distributed two-dimensional Discrete FFT transform
// YOUR NAME HERE
// ECE8893 Project 1

//steps send entire array to next cpu
//correct the rows your cpu is in charge of
//final cpu should transpose write to a file
//every cpu will read and prefore 1dtranspose
//steps 1 and 2 again
//write to output file
//i[c+*r#*width]


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <signal.h>
#include <math.h>
#include <mpi.h>

#include "Complex.h"
#include "InputImage.h"

using namespace std;
//void Transform1D(Complex* h, int w, Complex* H);

void Transform1D(Complex* h, int w, Complex* H)//code will only edit certain parts array that current rank needs to edit
{
  int numTask,rank;
  MPI_Comm_size(MPI_COMM_WORLD, &numTask);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);//get current rank

  int rowsPerRank=w/numTask;
  Complex newValue; 
  int startRow=rank*rowsPerRank*w;//start index based on rank.

  for(int k=0;k < rowsPerRank; k++ ){ 
    for(int i = 0; i < w ; i++ ){
      newValue=0; 
        for(int j = 0; j < w; j++ ){//Fourier transform formulae
          Complex temp(cos(2*M_PI*i*j/w),-sin(2*M_PI*i*j/w));
          temp = temp*h[startRow+j+k*w ]; 
          newValue=newValue+temp;
         }	  
      H[startRow+i+k*w]=newValue; 
     }
  }
       
}
void transpose(int w ,Complex* H)
{
  Complex* temp =new Complex[w*w];
  for(int i=0; i<w; i++){
   for(int j=0; j<w; j++){
     temp[i+w*j]=H[j+w*i];
    }
  }
  for(int k=0; k<(w*w); k++){
    H[k]=temp[k];
  }
  delete [] temp;
}


void push(Complex* result, int numTask, int w, int rank, InputImage i1){
//  Complex* incoming1=new Complex[w*w];
  int rc; 
  if (rank==0){
    int rowsPerRank=w/numTask; 
      for(int r=1; r<numTask; r++){
        Complex* incoming1=new Complex[w*w];
        MPI_Status status;
        rc = MPI_Recv(incoming1,w*w*sizeof(Complex), MPI_CHAR, r,0, MPI_COMM_WORLD, &status);    
        int startRow= w*r*rowsPerRank;    
        for(int i = 0; i < w*rowsPerRank ; i++ ){
          result[startRow+i]=incoming1[startRow+i];
       }
          delete [] incoming1;

     //attach over
      }
//send
    transpose(w,result);
    string fn1("MyAfter2D.txt");
    i1.SaveImageData(fn1.c_str(),result, w, w);

    	for(int r=1; r<numTask;r++){
          rc = MPI_Send(result,w*w*sizeof(Complex),MPI_CHAR,r,0,MPI_COMM_WORLD );
	 }
  }//end if rank=0

 else{
    rc=MPI_Send(result,w*w*sizeof(Complex),MPI_CHAR,0,0,MPI_COMM_WORLD);
    MPI_Status status;
    rc = MPI_Recv(result,w*w*sizeof(Complex), MPI_CHAR,0 ,0, MPI_COMM_WORLD, &status); 
 
  } 

}  
void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  // 1) Use the InputImage object to read in the Tower.txt file and
  InputImage i1 (inputFN);  
//    find the width/height of the input image.
  int h = i1.GetHeight(); int w = i1.GetWidth();
  // 2) Use MPI to find how many CPUs in total, and which one
  //    this process is
  int rank,numTask;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &numTask);//tasks = how many ranks there are
  // 3) Allocate an array of Complex object of sufficient size to
  //    hold the 2d DFT results (size is width * height)
  Complex* result= new Complex[w*h];
  Complex* result2= new Complex[w*h];
  // 4) Obtain a pointer to the Complex 1d array of input data
  Complex* C = i1.GetImageData();
  //1D
  Transform1D(C,w,result);    
  push(result,numTask,w,rank,i1);
  //2D
  Transform1D(result,w,result2);    
  push(result2,numTask,w,rank,i1);
  //deleting arrays
  delete [] result;delete [] result2;
  InputImage image(inputFN);  // Create the helper object for reading the image


}

int main(int argc, char** argv)
{
  int rc=MPI_Init(&argc, &argv);
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  Transform2D(fn.c_str()); // Perform the transform.
  MPI_Finalize();
}  

  

  

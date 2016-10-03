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

void Transform1D(Complex* h, int w, Complex* H){
  // Implement a simple 1-d DFT using the double summation equation
  // given in the assignment handout.  h is the time-domain input
  // data, w is the width (N), and H is the output array
  int numTask,rank;
  MPI_Comm_size(MPI_COMM_WORLD, &numTask);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int rowsPerRank=w/numTask;
  Complex newValue; 
  int startRow=rank*rowsPerRank*w;

 for(int k=0;k < rowsPerRank; k++ ){ 
  for(int i = 0; i < w ; i++ ){
    for(int j = 0; j < w; j++ ){
      Complex temp(cos(2*M_PI*i*j/w),-sin(2*M_PI*i*j/w));
      temp = temp*h[j];
      newValue=newValue+temp;
	}	  
   H[startRow+i+k*w]=newValue; 
  }
 }
       
}
void transpose(int w ,Complex* H){
  Complex* temp =new Complex[w*w];
  for(int i=0; i<w; i++)
   {
   for(int j=0; j<w; j++){
     temp[i+w*j]=H[j+w*i];
    }
  }
   for(int k=0; k<(w*w); k++){
     H[k]=temp[k];

   }
   delete [] temp;
}
void attach(Complex *input,int w, Complex *output){
  //take the row and attach it to the final answer
  //for example take all the rows rank x is responsible then modify the corresponding 
  int rank,numTask;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &numTask);//tasks = how many ranks there are
  
  int rowsPerRank=w/numTask;
  int startRow=rank*rowsPerRank*w;
  for(int k=0;k < rowsPerRank; k++ ){ 
    for(int i = 0; i < w ; i++ ){
       output[startRow+i+k*w]=input[startRow+i+k*w];
       
  }
 } 
}
void push(Complex* result,int numTask,int w,int rank,InputImage i1 ){
  Complex* incoming1=new Complex[w*w];   
  //
  int rc;
 if(rank != (numTask-1) )//everything except task 15 needs to recieve.Then attach result
    {  
     MPI_Status status;
     rc = MPI_Recv(incoming1, sizeof(incoming1), MPI_CHAR, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status) ; 
     attach(result,w,incoming1);
     cout<<" in Rank "<< rank << " input 1 was recieved"; 
    }  
 if(rank != 0 && rank != (numTask-1) ) //everything expect rank 0 and 15 push a recieved&edited message.
   {
    cout<<"\n"<<" in Rank "<< rank << " input 1 was pushed\n\n\n\n"; 
 
    rc= MPI_Send(incoming1,sizeof(incoming1),MPI_CHAR,rank-1,0,MPI_COMM_WORLD);
   }  
 if(rank == (numTask-1) )// rank 15 pushes the result message
  {
    rc= MPI_Send(result,sizeof(result),MPI_CHAR,rank-1,0,MPI_COMM_WORLD);
  }
 if (rank == (0))//rank 0 writes to file
 {
  
    transpose(w,incoming1);
    const char* fn1("output.txt");
    i1.SaveImageData(fn1,incoming1, w, w);
   
 }
  delete [] incoming1;
}
    

void Transform2D(const char* inputFN) 
{ // Do the 2D transform here.
  // 1) Use the InputImage object to read in the Tower.txt file and
  InputImage i1 (inputFN);  
//    find the width/height of the input image.
  int h = i1.GetHeight(); int w = i1.GetWidth();
  // 2) Use MPI to find how many CPUs in total, and which one
  //    this process is
  
  int rank,numTask,rc;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);  
  MPI_Comm_size(MPI_COMM_WORLD, &numTask);//tasks = how many ranks there are
  // 3) Allocate an array of Complex object of sufficient size to
  //    hold the 2d DFT results (size is width * height)
  Complex* result= new Complex[w*h];
  // 4) Obtain a pointer to the Complex 1d array of input data
  Complex* C = i1.GetImageData();
  // 5) Do the individual 1D transforms on the rows assigned to your CPU
  //the idea here is have each Rank have a certain number of rows, and send each row in that rank into the 1DTransformulae
  Transform1D(C,w,result);     
  // 6) Send the resultant transformed values to the appropriate
  //     other processors for the next phase.
  push(result,numTask,w,rank,i1);
  // 7) Receive messages from other processes to collect your column
  InputImage i2 ("output.txt");
  Complex* C2 = i2.GetImageData();
  Complex* result2= new Complex[w*h];
  Transform1D(C2,w,result2);
  push(result2,numTask,w,rank,i1);  
  // 8) When all columns received, do the 1D transforms on the columns
   // 9) Send final answers to CPU 0 (unless you are CPU 0)
  // 9a) If you are CPU 0, collect all values from other processors
  // and print out with SaveImageData().
     delete [] result; delete C; delete C2; delete result2;
  InputImage image(inputFN);  // Create the helper object for reading the image
  // Step (1) in the comments is the line above.
  // Your code here, steps 2-9
}
/*
void Transform1D(Complex* h, int w, Complex* H)
{
  // Implement a simple 1-d DFT using the double summation equation
  // given in the assignment handout.  h is the time-domain input
  // data, w is the width (N), and H is the output array
  int numTask,rank;
  MPI_Comm_size(MPI_COMM_WORLD, &numTask);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  int rowsPerRank=w/numTask;
  Complex newValue; 
  int startRow=rank*rowsPerRank*w;

 for(int k=0;k < rowsPerRank; k++ ){ 
  for(int i = 0; i < w ; i++ ){
    for(int j = 0; j < w; j++ ){
      Complex temp(cos(2*M_PI*i*j/w),-sin(2*M_PI*i*j/w));
      temp = temp*h[j];
      newValue=newValue+temp;
	}	  
   H[startRow+i+k*w]=newValue; 
  }
 }
       
}*/
int main(int argc, char** argv)
{
  int rc=MPI_Init(&argc, &argv);
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  Transform2D(fn.c_str()); // Perform the transform.
  MPI_Finalize();
}  

  

  

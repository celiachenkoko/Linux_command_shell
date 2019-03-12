#include <stdio.h>
#include <stdlib.h>

#include <iostream>
int main(int argc,char ** argv){
  for(int i = 1 ; i < argc; i++){
    std::cout<<argv[i]<<"\n";
  }
  return EXIT_SUCCESS;
}
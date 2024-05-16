#include "mpi.h"
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

typedef void* MPI_VECTOR;



std::vector<int> histogram(int* v, int chunk_size){
    std::vector<int> results;
    results.resize(256);
    for (int i = 0; i < 256; ++i){
        results[i] = 0;
    }
    for(int x = 0; x<chunk_size;x++){
        results[v[x]] ++;
    }
    return results;
}

std::vector<int> addVect(std::vector<int> a, std::vector<int> b){
    std::vector<int> results;
    results.resize(256);
    for (int i = 0; i < 256; ++i){
        results[i] = a[i]+b[i];
    }
    return results;
}


int main(int argc, char *argv[])
{
    int processId;
    int numberOfProcesses;
 
    // Setup MPI
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &processId);
    MPI_Comm_size( MPI_COMM_WORLD, &numberOfProcesses);

    int image_height;
    int image_width;
    int image_maxShades;
   
    int* chunk_inputImage;
    int* chunk_adj_matrix;
    int* inputImage; 
    int* adj_matrix;



        std::ifstream file(argv[1]);
        if(!file.is_open())
        {
            std::cout << "ERROR: Could not open file " << argv[1] << std::endl;
            MPI_Finalize();
            return 0;
        }
        std::string workString;
        /* Remove comments '#' and check image format */ 
        while(std::getline(file,workString))
        {
            if( workString.at(0) != '#' ){
                if( workString.at(1) != '2' ){
                    std::cout << "Input image is not a valid PGM image" << std::endl;
                    MPI_Finalize();
                    return 0;
                } else {
                    break;
                }       
            } else {
                continue;
            }
        }
        /* Check image size */ 
        while(std::getline(file,workString))
        {
            if( workString.at(0) != '#' ){
                std::stringstream stream(workString);
                int n;
                stream >> n;
                image_width = n;
                stream >> n;
                image_height = n;
                break;
            } else {
                continue;
            }
        }

        inputImage = new int[image_height * image_width];
        adj_matrix = new int[numberOfProcesses*numberOfProcesses];

        /* Check image max shades */ 
        while(std::getline(file,workString))
        {
            if( workString.at(0) != '#' ){
                std::stringstream stream(workString);
                stream >> image_maxShades;
                break;
            } else {
                continue;
            }
        }
    if(processId == 1){
        /* Fill input image matrix */ 
        int pixel_val;
        for( int i = 0; i < image_height; i++ )
        {
            if( std::getline(file,workString) && workString.at(0) != '#' ){
                std::stringstream stream(workString);
                for( int j = 0; j < image_width; j++ ){
                    if( !stream )
                        break;
                    stream >> pixel_val;
                    inputImage[i*image_width+j] = pixel_val;
                }
            } else {
                continue;
            }
        }
        std::ifstream file2(argv[2]);
        if(!file2.is_open())
        {
            std::cout << "ERROR: Could not open file " << argv[2] << std::endl;
            MPI_Finalize();
            return 0;
        }
        //fill adj matrix
        int adj_val;
        for( int i = 0; i < numberOfProcesses; i++ )
        {
            if(std::getline(file2,workString)){
                std::stringstream stream(workString);
                for( int j = 0; j < numberOfProcesses; j++ ){
                    if( !stream )
                        break;
                    stream >> adj_val;
                    adj_matrix[i*numberOfProcesses+j] = adj_val;
                }
            } else {
                continue;
            }
        }
         //     for(int i = 0; i < numberOfProcesses; i++ ){
    //     for(int j = 0; j < numberOfProcesses; j++ ){
    //         // std::cout<<adj_matrix[i*numberOfProcesses+j]<<" ";
    //     }
    //     std::cout<<"\n";
    // }
    }
    
    int ChunkSize = (image_height*image_width)/numberOfProcesses;
    chunk_inputImage = new int[ChunkSize];
    chunk_adj_matrix = new int[numberOfProcesses];

    MPI_Scatter(inputImage,ChunkSize,MPI_INT,chunk_inputImage,ChunkSize,MPI_INT,1,MPI_COMM_WORLD);
    MPI_Scatter(adj_matrix,numberOfProcesses,MPI_INT,chunk_adj_matrix,numberOfProcesses,MPI_INT,1,MPI_COMM_WORLD);

    std::vector<int> results = histogram(chunk_inputImage,ChunkSize);


    int added = 0;

    if(processId == 1){
        for(int i = 0; i < numberOfProcesses; i++ ){
            if(chunk_adj_matrix[i]==1){
                MPI_Send(results.data(),256,MPI_INT,i,0,MPI_COMM_WORLD);
                // std::cout<<"from "<<processId<<" sent to "<<i<<"\n";
                chunk_adj_matrix[i] = -1;
                break;
            }
        }
    }
    int whilebreak =1;
    int first = 1;
    if(processId == 1) first = 0;   
    int parent = 0;
    std::vector<int> total_results = results;
    while(whilebreak){
        std::vector<int> temp_results;
        temp_results.resize(256);
        MPI_Status status;
        MPI_Recv(temp_results.data(),256,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&status);
        // std::cout<<processId<<" receive from: "<<status.MPI_SOURCE<<"\n";
        chunk_adj_matrix[status.MPI_SOURCE] = -1;
        if(first){
            parent = status.MPI_SOURCE;
            first = 0;
            total_results = addVect(total_results,temp_results);
            std::cout<<processId<<"'s parent is "<<parent<<"\n";
        }
        else{
            total_results = temp_results;
        }

        for(int i = 0; i < numberOfProcesses; i++ ){
            if(chunk_adj_matrix[i]==1){
                MPI_Send(total_results.data(),256,MPI_INT,i,0,MPI_COMM_WORLD);
                // std::cout<<"from "<<processId<<"sent to "<<i<<"\n";
                chunk_adj_matrix[i] = -1;
                break;
            }
            if(i == (numberOfProcesses-1)){
                whilebreak=0;
                break;
            }
        }
    }

    MPI_Send(total_results.data(),256,MPI_INT,parent,0,MPI_COMM_WORLD);
    std::cout<<"from "<<processId<<"sent to parent: "<<parent<<"\n";




    if(processId == 1){
        std::ofstream ofile(argv[3]);
        if( ofile.is_open() )
        {
            for( int i = 0; i < 256; i++ )
            {
                ofile << total_results[i];
                ofile << "\n";
            }
        } else {
            std::cout << "ERROR: Could not open output file " << argv[2] << std::endl;
            return 0;
        }
    }
    
    // if(processId == 0){
    //     for(int i = 0; i < numberOfProcesses; i++ ){
    //         std::cout<< chunk_adj_matrix[i]<<", ";
    //     }
    // }


    // free(inputImage);
    // free(adj_matrix);
    // free(chunk_adj_matrix);
    // free(chunk_inputImage);
    MPI_Finalize();
    return 0;
}

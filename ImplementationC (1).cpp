#include <omp.h>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>

int partition(std::vector<int>& arr, int low, int high)
{
    int pivot = arr[high];
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    return (i + 1);
}

void quickSort_threaded(std::vector<int>& arr, int low, int high)
{
            
    if (low < high) {


        int section = partition(arr, low, high);

    
                #pragma omp task shared(arr)
                quickSort_threaded(arr, low, section - 1);
            
                #pragma omp task shared(arr)
                quickSort_threaded(arr, section + 1, high);
           
    }

}

void quickSort(std::vector<int>& arr, int low, int high)
{

            if (low < high) {
        
                int section = partition(arr, low, high);
        
                quickSort_threaded(arr, low, section - 1);

                quickSort_threaded(arr, section + 1, high);
            }


}


int main(){
    std::vector<int> vect(1000000);
    

    std::generate(vect.begin(), vect.end(), std::rand);

    std::vector<int> vect2 =vect;
    std::vector<int> vect3 =vect;
    std::vector<int> vect4 =vect;
    std::vector<int> vect5 =vect;
    std::vector<int> vect6 =vect;

    int N = vect.size();
 
    // for (int i = 0; i < N; i++)
    //     std::cout << vect[i] << " ";
    // Function call

    std::cout<<"\n";

    double dtime_static = omp_get_wtime();
    omp_set_num_threads(1);
    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout<<"num of threads: "<<omp_get_num_threads()<<"\n";
            quickSort_threaded(vect, 0, N - 1);
        }
    }   
    dtime_static = omp_get_wtime() - dtime_static;

    std::cout<<"time: "<<dtime_static<<"\n";

    dtime_static = omp_get_wtime();
    omp_set_num_threads(4);
    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout<<"num of threads: "<<omp_get_num_threads()<<"\n";
            quickSort_threaded(vect6, 0, N - 1);
        }
    }   
    dtime_static = omp_get_wtime() - dtime_static;

    std::cout<<"time: "<<dtime_static<<"\n";


    dtime_static = omp_get_wtime();
    omp_set_num_threads(8);
    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout<<"num of threads: "<<omp_get_num_threads()<<"\n";
            quickSort_threaded(vect3, 0, N - 1);
        }
    }   
    dtime_static = omp_get_wtime() - dtime_static;

    std::cout<<"time: "<<dtime_static<<"\n";


    dtime_static = omp_get_wtime();
    omp_set_num_threads(16);
    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout<<"num of threads: "<<omp_get_num_threads()<<"\n";
            quickSort_threaded(vect4, 0, N - 1);
        }
    }   
    dtime_static = omp_get_wtime() - dtime_static;

    std::cout<<"time: "<<dtime_static<<"\n";

    dtime_static = omp_get_wtime();
    omp_set_num_threads(24);
    #pragma omp parallel
    {
        #pragma omp single
        {
            std::cout<<"num of threads: "<<omp_get_num_threads()<<"\n";
            quickSort_threaded(vect5, 0, N - 1);
        }
    }   
    dtime_static = omp_get_wtime() - dtime_static;

    std::cout<<"time: "<<dtime_static<<"\n";


    double time_static = omp_get_wtime();
    quickSort(vect2, 0, N - 1);
    time_static = omp_get_wtime() - time_static;

    std::cout<<"no thread time: "<<time_static<<"\n";

    // std::cout << "Sorted array: " << std::endl;
    

    std::cout << std::is_sorted(std::begin(vect), std::end(vect)) << "\n";
    std::cout << std::is_sorted(std::begin(vect2), std::end(vect2)) << "\n";

    return 0;
}

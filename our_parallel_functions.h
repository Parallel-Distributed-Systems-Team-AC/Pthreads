

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>



#include "mmio.h"


#ifndef COLORING_AND_FIND_SCC
#define COLORING_AND_FIND_SCC   

typedef unsigned int uint;

// Structure declaration
struct vector_pair {   
  uint vector_size;         // Member (uint variable)
  uint* vector1_start;       // Member (uint* variable)
  uint* vector2_start;       // Member (uint* variable)
};

// Structure declaration
struct vector_part {
    struct vector_pair v ; // Struct
    uint start_of_the_part ;
    uint end_of_the_part ;
};

struct color_propagation_data{
    
    bool* has_changed_colors_forward ;
    bool* has_changed_colors_backward ;
    uint  E ;
    bool* is_in_SCC ;
    uint* v; 
    uint* v_back; 
    uint* I;
    uint* J;
};

struct color_propagation_data_part{
    struct color_propagation_data data ;
    uint start_for ;
    uint end_for ;  // less than E
};

struct my_bool_arrays_struct {
    
    bool* is_in_SCC;
    uint* SCC_colors;
    bool* is_it_root;
};

struct are_in_scc_data_part{
    struct vector_pair v ;
    struct my_bool_arrays_struct bool_arrays ;
    uint start_for;
    uint end_for ;
    bool* is_G_not_empty; // pointer not array ! 
};

struct trim_data{
    bool* is_in_I;
    bool* is_in_J;
    bool* is_in_SCC;
    bool* is_it_root;
    uint* I ; 
    uint* J ; 

};

struct trim_data_part{
    struct trim_data same_data_for_all_threads ;
    uint start_for_over_E ;
    uint end_for_over_E ;
    uint start_for_over_nodes ;
    uint end_for_over_nodes ;

};

union parfor_inputs {
    struct vector_part  coloring_initialization ;
    struct color_propagation_data_part color_propagation ; 
    struct are_in_scc_data_part find_scc_data ;
    struct trim_data_part trim_input ;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Forward Declarations  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int parfor( pthread_t* threads , int total_number_of_threads, void* (some_parallel_funtion)(void*) , union parfor_inputs* thread_input);
//void* parfor( pthread_t* threads , int total_number_of_threads, void* (some_parallel_funtion)(void*) , void* thread_input , int thread_input_element_size);

void* parfor_test( pthread_t* threads , int total_number_of_threads, void* (some_parallel_funtion)(void*) , void* thread_input , int thread_input_element_size);

void* initialize_colors_parallel( void* v_part);

void* forward_color_propagation( void* input );

void* backward_color_propagation( void* input );
void* parallel_forward_and_backward_color_propagation( void* input );

void* find_SCC ( void* input);
void* count_SCC ( void* input);

int parallel_trim(pthread_t* threads , int number_of_threads , void* input  ) ;
void* check_if_IorJ (void* input ) ;
void* actual_parallel_trim ( void* input);
void* initialize_to_false_is_in_IandJ ( void* input);

void trim_serial( uint* I , uint* J ,bool* is_in_I , bool* is_in_J , bool* is_in_SCC , bool* is_it_root, uint E , uint total_nodes, int* total_nodes_in_SCC);
// That is the old serial one 
//void initialize_colors(uint *v, uint *v_back, uint size_of_V);

void print_array(uint *v, uint size_of_V);

void print_edges(uint *I, uint *J, uint E);


// bool is_it_in_this_vector(uint *vector, uint size_of_vector,
//                          uint element_we_search, bool is_the_vector_sorted);

void load_file(const char *file_name, uint *total_nodes,
               uint *total_number_of_edges, uint **I, uint **J, int number_of_columns) ;


void load_example(uint *total_nodes, uint *total_number_of_edges, uint **I,
                  uint **J);


const char* select_file(int number_of_file);
int number_columns(int number_of_file);


#endif


#include <sys/time.h>


#include <pthread.h>
#include <time.h>



#include "our_parallel_functions.h"


int main() {

  // gcc our_parallel_functions.c  main_pthreadsV1.8.c mmio.c  -o "executable_name" -O3 -lpthread  

  printf(" ~~~~~~~~~~~~~~~~~~  Code Version 1.8 pthreads  ~~~~~~~~~~~~~~~~~~ \n\n");
  //~~~~~~~~~~~~~~~~~~~~~  Opening file ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  uint *I, *J;
  uint total_nodes;
  uint total_number_of_edges;
  bool load_from_file = true;
  int number_of_file;

  
  //struct timeval stop, start;
  struct timeval start_load, end_load;
  struct timeval start_trim, end_trim;
  struct timeval start_SCC , end_SCC;

  printf("0) For 'celegansneural.mtx'\n");
  printf("1) For 'foldoc.mtx' \n");
  printf("2) For 'language.mtx' \n");
  printf("3) For 'eu-2005.mtx'\n");
  printf("4) For 'wiki-topcats.mtx'\n");
  printf("5) For 'sx-stackoverflow.mtx'\n");
  printf("6) For 'wikipedia-20060925.mtx'\n");
  printf("7) For 'wikipedia-20061104.mtx'\n");
  printf("8) For 'wikipedia-20070206.mtx'\n");
  printf("9) For 'wb-edu.mtx'\n");
  printf("10) For 'indochina-2004.mtx' \n");
  printf("11) For 'uk-2002.mtx' \n");
  printf("12) For 'arabic-2005.mtx' \n");
  printf("13) For 'uk-2005.mtx'\n");
  printf("14) For 'twitter7.mtx' \n");
  
  printf("Enter the file you want to test: ");
  scanf("  %d", &number_of_file);

  

  gettimeofday(&start_load, NULL);

  const char* file_name = select_file(number_of_file);
  int number_of_columns=number_columns(number_of_file);

  // Load data from a file
  if (load_from_file) {
    //const char *file_name = "files/indochina-2004.mtx";
    printf("\n\n                     File chosen: %s\n", file_name);
    
    printf("Start loading the file... ");
    load_file(file_name, &total_nodes, &total_number_of_edges, &I, &J,number_of_columns);
  } 
  else {
    printf("Load a small custom example graph  \n");
    load_example(&total_nodes, &total_number_of_edges, &I, &J);
  }

  
  printf("\rGraph stats                                          \n");
  printf("Number of edges = %d\n", total_number_of_edges);
  printf("Number of nodes = %d\n", total_nodes);
  
  int number_of_threads = 8;
  printf("Number of threads that will be used for trimming and finding the SCC %d \n" , number_of_threads);
  
  
  
  gettimeofday(&end_load, NULL);
  printf("\nTime to load file: \t\t %lu us\n", (end_load.tv_sec - start_load.tv_sec) * 1000000 + end_load.tv_usec - start_load.tv_usec);


  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Threads ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  pthread_t threads[number_of_threads];  
  
  bool is_G_not_empty = true;
  uint *SCC_colors = (uint *)malloc(total_nodes * sizeof(uint));
  bool *is_it_root = (bool *)calloc(sizeof(bool), total_nodes);
  bool *is_in_SCC = (bool *)calloc(sizeof(bool), total_nodes);

    
  struct my_bool_arrays_struct  bool_arrays_in_main;

  bool_arrays_in_main.is_in_SCC = is_in_SCC ;
  bool_arrays_in_main.is_it_root = is_it_root ;
  bool_arrays_in_main.SCC_colors = SCC_colors ;



  // Make the v and v_back
  struct vector_pair v_colors ;  
  
  v_colors.vector_size = total_nodes ;
  v_colors.vector1_start = (uint *) malloc(v_colors.vector_size*sizeof(uint)) ;
  v_colors.vector2_start = (uint *) malloc(v_colors.vector_size*sizeof(uint)) ;

  int step_v = v_colors.vector_size / number_of_threads;

  union parfor_inputs   find_scc_parallel_input[number_of_threads];
  
  //struct vector_part thread_part_to_initialize[number_of_threads] ;
  union parfor_inputs color_init_data[number_of_threads]; // New union 
  
  // Find what part of v and v_back  / v_colors each thread will initialize
  //printf("Parts of the coloring thing\n");
  for ( int i = 0 ; i < number_of_threads ; i++){
    color_init_data[i].coloring_initialization.v = v_colors ;
    color_init_data[i].coloring_initialization.start_of_the_part =  i   * step_v ;
    color_init_data[i].coloring_initialization.end_of_the_part =  (i+1) * step_v ;

    find_scc_parallel_input[i].find_scc_data.v = v_colors ;
    find_scc_parallel_input[i].find_scc_data.bool_arrays = bool_arrays_in_main ;
    find_scc_parallel_input[i].find_scc_data.start_for =  i   * step_v ;
    find_scc_parallel_input[i].find_scc_data.end_for =  (i+1) * step_v ;
    find_scc_parallel_input[i].find_scc_data.is_G_not_empty = &is_G_not_empty;
    
        
    //printf("Start of id %d is  = %u\n", i , color_init_data[i].coloring_initialization.start_of_the_part   );
    //printf("End   of id %d is  = %u\n", i , color_init_data[i].coloring_initialization.end_of_the_part   );
        
  }           
  
  find_scc_parallel_input[number_of_threads-1].find_scc_data.end_for +=  v_colors.vector_size % number_of_threads ;
  // In case it is not divisible by the number_of_threads we add to last thread the extra work
  color_init_data[number_of_threads-1].coloring_initialization.end_of_the_part += v_colors.vector_size % number_of_threads ;
  //printf("The remeinder is %u \n" ,v_colors.vector_size % number_of_threads );
  // printf("End   of id %d is  = %u\n\n", number_of_threads-1 , color_init_data[number_of_threads-1].coloring_initialization.end_of_the_part   );
    

  // Define those to not change the whole code again 
  uint* v       = v_colors.vector1_start ;
  uint* v_back  = v_colors.vector2_start ; 
  uint total_different_colors_in_SCC = 0;

  bool has_changed_colors_forward, has_changed_colors_backward;
  uint total_nodes_in_SCC = 0;
  uint E = total_number_of_edges; // This is for the start it will get shorter

  //print_edges(I, J, E);   // Debug_print
  // bool has_changed_colors ;

  uint times_coloration_for; // Just to count how many times we

  struct color_propagation_data propagation_data ; 

  propagation_data.has_changed_colors_forward = &has_changed_colors_forward ;
  propagation_data.has_changed_colors_backward = &has_changed_colors_backward ;
  propagation_data.is_in_SCC = is_in_SCC ;
  propagation_data.E = E ;
  propagation_data.v = v ;
  propagation_data.v_back = v_back ;
  propagation_data.I = I ;
  propagation_data.J = J ;

  uint step_E = E / number_of_threads;

  // To tell each thread what part it has to propagate 
  struct color_propagation_data_part thread_part_to_color_propagate[number_of_threads];  
  union parfor_inputs part_to_color_propagate[number_of_threads]; // New union 

  // Find what part of I and J each thread will propagate
  for ( int i = 0 ; i < number_of_threads ; i++){
    part_to_color_propagate[i].color_propagation.data = propagation_data  ;           // =============
    part_to_color_propagate[i].color_propagation.start_for =  i   * step_E ;          // =============
    part_to_color_propagate[i].color_propagation.end_for =  (i+1) * step_E ;          // =============
      
    //printf("Start of id %d is  = %u\n", i ,part_to_color_propagate[i].color_propagation.start_for  );
    //printf("End   of id %d is  = %u\n", i , part_to_color_propagate[i].color_propagation.end_for   );
     
  }        
  
  part_to_color_propagate[number_of_threads-1].color_propagation.end_for += E % number_of_threads ;
  //printf("End of id %d is  = %u\n\n", number_of_threads-1 , part_to_color_propagate[number_of_threads-1].color_propagation.end_for   );
       
  // In case it is not divisible by the number_of_threads we add to last thread the extra work

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  End of Set-up ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Trim stuff ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  // Inputs is_in_I and is_in_J must have false everywhere !   
  bool* is_in_I = (bool*) calloc(sizeof(bool) , total_nodes);
  bool* is_in_J = (bool*) calloc(sizeof(bool) , total_nodes);

  
  // Input in trim , same for all threads used in the union struct  trim_data_part  'trim_input'
  struct trim_data same_trim_data ;

  same_trim_data.is_in_I = is_in_I ;
  same_trim_data.is_in_J = is_in_J ;
  same_trim_data.is_in_SCC = is_in_SCC ;
  same_trim_data.is_it_root = is_it_root ;
  same_trim_data.I = I ;
  same_trim_data.J = J ;
  

  union parfor_inputs trim_input_from_main[number_of_threads];

  for ( int i = 0 ; i < number_of_threads ; i++ ){
    trim_input_from_main[i].trim_input.same_data_for_all_threads  = same_trim_data ;
    trim_input_from_main[i].trim_input.start_for_over_E           = part_to_color_propagate[i].color_propagation.start_for  ; // Those are over E , are calculated above
    trim_input_from_main[i].trim_input.end_for_over_E             = part_to_color_propagate[i].color_propagation.end_for    ;
    trim_input_from_main[i].trim_input.start_for_over_nodes       = find_scc_parallel_input[i].find_scc_data.start_for      ; // Those are over the nodes , are calculated above
    trim_input_from_main[i].trim_input.end_for_over_nodes         = find_scc_parallel_input[i].find_scc_data.end_for        ;
  }

  printf("Start triming...");

  gettimeofday(&start_trim, NULL);
  total_nodes_in_SCC +=  parallel_trim(threads , number_of_threads , (void*) trim_input_from_main  ) ; 
  // trim_serial( I , J , is_in_I , is_in_J , is_in_SCC , is_it_root, E ,  total_nodes ,&total_nodes_in_SCC)   ;
  
  gettimeofday(&end_trim, NULL);
  printf("\rTime to trim: \t\t\t %lu us\n", (end_trim.tv_sec - start_trim.tv_sec) * 1000000 + end_trim.tv_usec - start_trim.tv_usec);
  
  //printf("After trim\n");

  gettimeofday(&start_SCC, NULL);

  printf("Start finding the SCCs ...");
  while (is_G_not_empty) {

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ initialize_colors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    

    //printf("About to color v and v_back \n"  ) ;                              // Debug_print
    parfor(threads , number_of_threads ,initialize_colors_parallel ,  color_init_data  );
    //printf("Ended coloring v and v_back \n"  ) ;                              // Debug_print
    
   
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  FORWWARD & BACKWARD  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    //printf("About to go in FORWWARD & BACKWARD parallel while \n"  ) ;        // Debug_print
    has_changed_colors_forward = true;
    has_changed_colors_backward = true;
    
    while (has_changed_colors_forward && has_changed_colors_backward) {
        has_changed_colors_forward = false;
        has_changed_colors_backward = false;
        parfor(threads , number_of_threads ,parallel_forward_and_backward_color_propagation , part_to_color_propagate );
    }    
    //printf("About to go in FORWWARD & BACKWARD parallel while \n"  ) ;        // Debug_print
    

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  FORWWARD  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    
    //printf("About to go in FORWWARD parallel while \n"  ) ;                   // Debug_print
    while (has_changed_colors_forward) {
      has_changed_colors_forward = false;
      parfor(threads , number_of_threads ,forward_color_propagation , part_to_color_propagate  );
    }    
    //printf("Ended the FORWWARD parallel while \n\n"  ) ;                      // Debug_print
    
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  BACKWARD  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //printf("About to go in BACKWARD parallel while \n"  ) ;                   // Debug_print
    while (has_changed_colors_backward) {
      has_changed_colors_backward = false;
      parfor(threads , number_of_threads ,backward_color_propagation  , part_to_color_propagate );
    }
    //printf("Ended the BACKWARD parallel while \n"  ) ;                         // Debug_print
    

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Find SCC  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // printf(" \nPrint who are in SCCs: \n");     // Debug_print
    
     
    int return_value = parfor(threads , number_of_threads , find_SCC , find_scc_parallel_input);
    total_nodes_in_SCC += return_value ;
     
    
    //printf(" After find scc total_nodes_in_scc are is:%d",total_nodes_in_SCC);

    // Check if there are nodes left in the graph 
    if (total_nodes_in_SCC == total_nodes) {
      is_G_not_empty = false;
      break;
    }


    // print_edges(I, J, E);        // Debug_print

    // trim ???? // Will this make it faster in any way ?
  }

  uint total_number_SCC = -1; // because 0 is included

  uint return_value = parfor(threads , number_of_threads , count_SCC , find_scc_parallel_input);
  total_number_SCC += return_value;
  


  gettimeofday(&end_SCC, NULL);
  printf("\rTime to find SCC: \t\t %lu us\n", (end_SCC.tv_sec - start_SCC.tv_sec) * 1000000 + end_SCC.tv_usec - start_SCC.tv_usec);
  printf("Time to trim and find SCC: \t %lu us\n", (end_SCC.tv_sec - start_trim.tv_sec) * 1000000 + end_SCC.tv_usec - start_trim.tv_usec);

  

  printf("\nTotal total_number_SCC = %d \n\n", total_number_SCC);

  free(I);
  free(J);
 
  free(is_it_root);
  free(is_in_SCC);
  free(SCC_colors);

  //free(v);
  //free(v_back);
  free(v_colors.vector1_start);
  free(v_colors.vector2_start);

  
  free(is_in_I);
  free(is_in_J);

}
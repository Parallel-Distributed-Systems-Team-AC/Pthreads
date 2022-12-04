
#include "our_parallel_functions.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  Function Implementations   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void print_array(uint *v, uint size_of_V) {
  // printf("Inside print_array \n");
  for (uint i = 0; i < size_of_V; i++) {
    printf("Node: %4d  v = %4d\n", i, v[i]);
  }
  // printf("End of print_array \n");
}

void print_edges(uint *I, uint *J, uint E) {
  printf("Inside print_edges \n");

  printf(" \n");
  for (uint i = 0; i < E; i++) {
    printf("Edge form: %4d --> %4d \n", I[i], J[i]);
  }

  printf(" \n");
  printf("End of print_edges \n");
}



void load_example(uint *total_nodes, uint *total_number_of_edges, uint **I,
                  uint **J) {

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ OUR TEST ARRAY
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  printf("Get in main and Define variable \n");

  /* SCC are :
        3                          // from  trim
        6                          // from  trim
        2                          // from Re - trim
        5                          // from Re - trim
        26                         // from Re - trim
        27                         // from Re - trim
        20                         // points to itself
        [13,12]                    // max 12
        [7,4,1]                    // max 7
        [11,10,9,8]                // max 11
        [17,16,15,14]              // max 17
        [25,23,24,21,22,19,18]     // max 25
    */

  *total_nodes =
      28; // plus one for the node ZERO , for the numbers to be the  same

  printf("Define test array\n");
  // Temp array to test and load values
  uint array[] = {5,  1,  7,  1,  26, 2,  1,  4,  2,  5,  4,  7,  11,
                  8,  8,  9,  9,  10, 10, 11, 13, 12, 12, 13, 17, 14,
                  27, 14, 14, 15, 16, 15, 15, 16, 16, 17, 22, 18, 18,
                  19, 20, 20, 18, 21, 19, 22, 25, 22, 21, 23, 22, 23,
                  19, 24, 23, 25, 24, 25, 10, 26, 26, 27 };

  *total_number_of_edges = sizeof(array) / 8; //  = len(I) = len(J)
  printf("number_of_edges = %d\n", *total_number_of_edges);
  printf("number_of_nodes = %d\n", *total_nodes);

  printf("Start malloc \n");

  *I = (uint *)malloc(*total_number_of_edges * sizeof(int));
  *J = (uint *)malloc(*total_number_of_edges * sizeof(int));

  printf("Fill I and J from the example array\n");

  for (uint i = 0; i < *total_number_of_edges; i++) {
    // printf("Passed i= %d\n" ,i);
    (*I)[i] = array[2 * i];
    (*J)[i] = array[2 * i + 1];
  }

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ End of OUR ARRAY
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}


int parfor( pthread_t* threads , int total_number_of_threads, void* (some_parallel_funtion)(void*) , union parfor_inputs* thread_input){
  
  // Create threads      
  // printf("About to create %d threads \n" , total_number_of_threads ) ;
  for ( int i = 0 ; i < total_number_of_threads ; i++){
      if ( pthread_create(&threads[i], NULL, some_parallel_funtion, (void *)  &( thread_input[i])) != 0)   {perror("Failed to created thread"); }
      //if ( pthread_create(&threads[i], NULL, initialize_colors_parallel, (void*) &thread_part_to_initialize[i]) != 0)   {perror("Failed to created thread"); }
    
  }
  // Join Threads 
  //printf("About to join %d threads \n" , total_number_of_threads ) ;
  int total_sum = 0 ;
  int* partial_sum ;
  for ( int i = 0 ; i < total_number_of_threads ; i++){
    //printf("Try to join thread %d  \n" , i  ) ;
    if ( pthread_join( threads[i], (void **) &partial_sum) != 0) { perror("Failed to join thread"); } 
    if(partial_sum != NULL){
      total_sum += *partial_sum;
      free(partial_sum);
      
    }
  } 
  //printf("Threads joined \n\n"  ) ;
  return total_sum ; 

}

//without_unions_and_cumsum
void* parfor_old( pthread_t* threads , int total_number_of_threads, void* (some_parallel_funtion)(void*) , void* thread_input , int thread_input_element_size){
  /*
    // Create threads      
    // printf("About to create %d threads \n" , total_number_of_threads ) ;
  */
  printf("\nEach_input of each thread: \n");
  for ( int i = 0 ; i < total_number_of_threads ; i++){
    if ( pthread_create(&threads[i], NULL, some_parallel_funtion,   thread_input + i * thread_input_element_size ) != 0)   {perror("Failed to created thread"); }
    
  }
  // Join Threads 
  //printf("About to join %d threads \n" , total_number_of_threads ) ;
  for ( int i = 0 ; i < total_number_of_threads ; i++){
    //printf("Try to join thread %d  \n" , i  ) ;
    if ( pthread_join( threads[i], NULL) != 0) { perror("Failed to join thread"); } 
  }    
  //printf("Threads joined \n\n"  ) ;
  return NULL ;

}

void* initialize_colors_parallel( void* v_part){
    
    //printf("Start of a thread initialize_colors \n" ) ;

    union parfor_inputs  temp_uninon =  *( (union parfor_inputs*) v_part );
    struct vector_part v_part_fun = temp_uninon.coloring_initialization ;
    int start_from = v_part_fun.start_of_the_part ;
    int end_to = v_part_fun.end_of_the_part ;

    int* v1 = v_part_fun.v.vector1_start ;
    int* v2 = v_part_fun.v.vector2_start ;
    int total_size = v_part_fun.v.vector_size;
    
    //printf("The initializze colors start %d  end_to %d \n" , start_from , end_to);
    for( int i = start_from ; i < end_to ; i++){
        v1[i] = i ;
        v2[i] = i ;
    }
    
    //printf("End of a thread  initialize_colors \n" ) ;

    return NULL ;

}


void trim_serial( uint* I , uint* J ,bool* is_in_I , bool* is_in_J , bool* is_in_SCC , bool* is_it_root, uint E , uint total_nodes  , int* total_nodes_in_SCC){

  
  // is_in_I , is_in_J  must have false everywere 

  bool trimed_once = false ;
  int count = 0 ; 

  for( int i = 0 ; i < E ; i++){
    if( (! is_in_SCC[ I[i] ] ) && (!  is_in_SCC[ J[i] ]) ){

      is_in_I[ I[i] ] = true;
    }
    if( (! is_in_SCC[ I[i] ] ) && (!  is_in_SCC[ J[i] ])  ){

      is_in_J[ J[i] ] = true;
    }
  }

  for( int i = 0 ; i < total_nodes ; i++){
    if ( is_in_I[i] != is_in_J[i] ){
      // Then it is not in I and is in J
      // or it is in I and not in J 
      // This is a XOR
      is_in_SCC[i] = true ; 
      is_it_root[i] = true ; 
      (*total_nodes_in_SCC)++;
      count++ ;

      trimed_once = true ;
    }
  }

  // We can call this again to trim new nodes

  printf("Trimed count %d ", count);

  if( trimed_once ){

    for( int i = 0 ; i < total_nodes ; i++){
      
      is_in_I[i] = false;
      is_in_J[i] = false;
    }
    
    
    trim_serial( I , J , is_in_I , is_in_J , is_in_SCC , is_it_root, E ,  total_nodes  ,total_nodes_in_SCC);
  }

  //return NULL ;
}


int parallel_trim(pthread_t* threads , int number_of_threads , void* input  ){
  // is_in_I , is_in_J  must have false everywere 

  int total_trimed_inside_trim = 0 ; 
  int temp_nodes_trimed = 0 ; 


  parfor( threads , number_of_threads , check_if_IorJ , input );



  
  temp_nodes_trimed = parfor( threads , number_of_threads , actual_parallel_trim , input );
  total_trimed_inside_trim += temp_nodes_trimed ;
  
  // We can call this again to trim new nodes
  if ( temp_nodes_trimed != 0 ){
    // is_in_I , is_in_J  must have false everywere before we call trim again 
    parfor( threads , number_of_threads , initialize_to_false_is_in_IandJ , input );
    
    temp_nodes_trimed = parallel_trim( threads ,  number_of_threads ,  input  );
    total_trimed_inside_trim += temp_nodes_trimed ;
    //trim_serial( I , J , is_in_I , is_in_J , is_in_SCC , is_it_root, E ,  total_nodes  ,total_nodes_in_SCC);
  }

  return  total_trimed_inside_trim; 

}


void* actual_parallel_trim ( void* input){

  // Expected input is a "union parfor_inputs*"     
  union parfor_inputs  temp_uninon =  * (union parfor_inputs*) input ;
  // That has stored inside a struct of type :
  struct trim_data_part temp = temp_uninon.trim_input ; 

  uint start_for_over_nodes = temp.start_for_over_nodes ;
  uint end_for_over_nodes = temp.end_for_over_nodes ;

  // Same data for all threads:
  uint* I = temp.same_data_for_all_threads.I ; 
  uint* J = temp.same_data_for_all_threads.J ; 
  bool* is_in_J = temp.same_data_for_all_threads.is_in_J ; 
  bool* is_in_I = temp.same_data_for_all_threads.is_in_I ;
  bool* is_in_SCC = temp.same_data_for_all_threads.is_in_SCC ;
  bool* is_it_root = temp.same_data_for_all_threads.is_it_root ;

  int parctial_counter_of_total_nodes_in_SCC = 0 ;

  for( uint i = start_for_over_nodes ; i < end_for_over_nodes ; i++){
    if ( is_in_I[i] != is_in_J[i] ){
      // Then it is not in I and is in J
      // or it is in I and not in J 
      // This is a XOR
      is_in_SCC[i] = true ; 
      is_it_root[i] = true ; 
      parctial_counter_of_total_nodes_in_SCC++;
      //(*total_nodes_in_SCC)++;
      

      //trimed_once = true ; // We will test this by checking if we added any new nodes to the scc
    }
  }
  

  int* result = malloc(sizeof(int*));
  *result = parctial_counter_of_total_nodes_in_SCC ;
  return (void*) result ; 


}


void* initialize_to_false_is_in_IandJ ( void* input){
  // Expected input is a "union parfor_inputs*"     
  union parfor_inputs  temp_uninon =  * (union parfor_inputs*) input ;
  // That has stored inside a struct of type :
  struct trim_data_part temp = temp_uninon.trim_input ; 

  uint start_for_over_nodes = temp.start_for_over_nodes ;
  uint end_for_over_nodes = temp.end_for_over_nodes ;

  // Same data for all threads:
  //uint* I = temp.same_data_for_all_threads.I ; 
  //uint* J = temp.same_data_for_all_threads.J ; 
  bool* is_in_J = temp.same_data_for_all_threads.is_in_J ; 
  bool* is_in_I = temp.same_data_for_all_threads.is_in_I ;
  //bool* is_in_SCC = temp.same_data_for_all_threads.is_in_SCC ;

  for( uint i = start_for_over_nodes ; i < end_for_over_nodes ; i++){
      
      is_in_I[i] = false;
      is_in_J[i] = false;
    }

  return NULL;

}


void* check_if_IorJ (void* input ){

  // Expected input is a "union parfor_inputs*"     
  union parfor_inputs  temp_uninon =  * (union parfor_inputs*) input ;
  // That has stored inside a struct of type :
  struct trim_data_part temp = temp_uninon.trim_input ; 

  uint start_for_over_E = temp.start_for_over_E ;
  uint end_for_over_E = temp.end_for_over_E ;

  // Same data for all threads:
  uint* I = temp.same_data_for_all_threads.I ; 
  uint* J = temp.same_data_for_all_threads.J ; 
  bool* is_in_J = temp.same_data_for_all_threads.is_in_J ; 
  bool* is_in_I = temp.same_data_for_all_threads.is_in_I ;
  bool* is_in_SCC = temp.same_data_for_all_threads.is_in_SCC ;
  
  for( uint i = start_for_over_E ; i < end_for_over_E ; i++){
    if( (! is_in_SCC[ I[i] ] ) && (!  is_in_SCC[ J[i] ]) ){

      is_in_I[ I[i] ] = true;
    }
    if( (! is_in_SCC[ I[i] ] ) && (!  is_in_SCC[ J[i] ])  ){

      is_in_J[ J[i] ] = true;
    }
  }

  return NULL ;

}



void* forward_color_propagation( void* input ){
    
    //printf("A thread was created pthread ID - %d \n" , pthread_self()  ) ;
    // Here we should break the inside loop to par for 

    
    union parfor_inputs  temp_uninon =  *( (union parfor_inputs*) input );
    struct color_propagation_data_part temp = temp_uninon.color_propagation ;

    uint temp1 = temp.start_for ;
    uint temp2 = temp.end_for ;

    bool* has_changed_colors = temp.data.has_changed_colors_forward; // one way
    bool* is_in_SCC = temp.data.is_in_SCC;
    uint* I = temp.data.I ;
    uint* J = temp.data.J ;
    uint* v = temp.data.v ;

    //bool is_there_a_change = false ;
    
    for (uint k = temp.start_for; k < temp.end_for; k++) {
        if (is_in_SCC[I[k]] || is_in_SCC[J[k]]) {
          continue;
        }
        if (v[I[k]] > v[J[k]]) {
          v[J[k]] = v[I[k]];
          *has_changed_colors = true;
          //is_there_a_change = true ;
        }
      }

    return NULL ;
}


void* backward_color_propagation( void* input ){
        
    union parfor_inputs  temp_uninon =  *( (union parfor_inputs*) input );
    struct color_propagation_data_part temp = temp_uninon.color_propagation ;

    uint temp1 = temp.start_for ;
    uint temp2 = temp.end_for ;
  
    bool* has_changed_colors = temp.data.has_changed_colors_backward; // one way
    bool* is_in_SCC = temp.data.is_in_SCC;
    uint* I = temp.data.J ;         // This is different because it is backwards 
    uint* J = temp.data.I ;         // This is different because it is backwards 
    uint* v = temp.data.v_back ;    // This is different because it is backwards 

    //bool is_there_a_change = false ;
    
    //for (int k =( ( temp.end_for)-1 ); k >= temp.start_for ; k--) {
    for (uint k = temp.start_for; k < temp.end_for; k++) {
        
        if (is_in_SCC[I[k]] || is_in_SCC[J[k]]) {
          continue;
        }
        

        
        if (v[I[k]] > v[J[k]]) {
          v[J[k]] = v[I[k]];
          *has_changed_colors = true;
        }
        
        
    }
    

    return NULL ;
}



void* parallel_forward_and_backward_color_propagation( void* input ){

    union parfor_inputs  temp_uninon =  *( (union parfor_inputs*) input );
    struct color_propagation_data_part temp = temp_uninon.color_propagation ;

    uint temp1 = temp.start_for ;
    uint temp2 = temp.end_for ;
  


    bool* has_changed_colors_forward = temp.data.has_changed_colors_forward; //  Forward    
    bool* has_changed_colors_backward = temp.data.has_changed_colors_backward; // Backward
    bool* is_in_SCC = temp.data.is_in_SCC;
    uint* I = temp.data.I ;
    uint* J = temp.data.J ;
    uint* v = temp.data.v ;    
    uint* v_back = temp.data.v_back ;    

    //bool is_there_a_change = false ;
    
    
    for (uint k = temp.start_for; k < temp.end_for; k++) {

      if (is_in_SCC[I[k]] || is_in_SCC[J[k]]) {
        continue;
      }

      if (v[I[k]] > v[J[k]]) {
        v[J[k]] = v[I[k]];
        *has_changed_colors_forward = true;
      }
      if (v_back[J[k]] > v_back[I[k]]) {
        v_back[I[k]] = v_back[J[k]];
        *has_changed_colors_backward = true;
      }
      
      }



    return NULL ;
}






void* find_SCC ( void* input){

  union parfor_inputs  temp_uninon =  *( (union parfor_inputs*) input );
  struct are_in_scc_data_part temp_right_struct = temp_uninon.find_scc_data ;
  int pactial_sum  = 0 ; 


   uint* v = temp_right_struct.v.vector1_start ;
   uint* v_back = temp_right_struct.v.vector2_start ;
    
    bool* is_in_SCC = temp_right_struct.bool_arrays.is_in_SCC;
    uint* SCC_colors = temp_right_struct.bool_arrays.SCC_colors;
    bool* is_it_root = temp_right_struct.bool_arrays.is_it_root;
    
    uint start_for = temp_right_struct.start_for;
    uint end_for = temp_right_struct.end_for;
    //bool*  is_G_not_empty = temp_right_struct.is_G_not_empty;


    
    for (uint node = start_for; node < end_for; node++) {
      if (is_in_SCC[node]) {
        continue;
      }
      if (v[node] == v_back[node]) {
        // Then the node 'node' is part of an SCC with color v[node]
        is_in_SCC[node] = true;
        SCC_colors[node] = v[node];
        pactial_sum++;
        // printf("The node %4d is part of the SCC with color %4d \n", node,v[node]);        // Debug_print

        if (v[node] == node) {
          // then 'node' is the root of an SCC
          is_it_root[node] = true;
        }
      }
    }

    int* result = malloc(sizeof(int*));
    *result = pactial_sum ;
    return (void*) result ; 

}


void* count_SCC ( void* input){


  union parfor_inputs  temp_uninon =  *( (union parfor_inputs*) input );
  struct are_in_scc_data_part temp_right_struct = temp_uninon.find_scc_data ;
  int pactial_sum  = 0 ; 

  bool* is_in_SCC = temp_right_struct.bool_arrays.is_in_SCC;
  //uint* SCC_colors = temp_right_struct.bool_arrays.SCC_colors;
  bool* is_it_root = temp_right_struct.bool_arrays.is_it_root;
  
  uint start_for = temp_right_struct.start_for;
  uint end_for = temp_right_struct.end_for;
  //bool*  is_G_not_empty = temp_right_struct.is_G_not_empty;




  for (uint i = start_for; i < end_for; i++) {

    if (!is_in_SCC[i]) {
      // This means i is not yet in an SCC and does not have any arrows / edges
      // so it is a trivial SCC
      is_it_root[i] = true;
      pactial_sum++;
      // continue;
      // Store i in an SCC with color i
    } 
    else if (is_it_root[i]) {
      pactial_sum++;
    }
  }


    int* result = malloc(sizeof(int*));
    *result = pactial_sum ;
    return (void*) result ; 

}




 


void load_file(const char *file_name, uint *total_nodes,
               uint *total_number_of_edges, uint **I, uint **J, int number_of_columns) {

  uint ret_code;
  MM_typecode matcode;
  FILE *f;

  uint M, N, nz;
  // uint i, *I, *J;
  // uint total_nodes;
  // uint total_number_of_edges;

  //printf("Inside load file \n");

  if ((f = fopen(file_name, "r")) == NULL) {
    printf("Could not open file it returned a NULL pointer.\n");
    exit(1);
  }

  if (mm_read_banner(f, &matcode) != 0) {
    printf("Could not process Matrix Market banner.\n");
    exit(1);
  } 

  /*  This is how one can screen matrix types if their application */
  /*  only supports a subset of the Matrix Market data types.      */

  if (mm_is_complex(matcode) && mm_is_matrix(matcode) &&
      mm_is_sparse(matcode)) {
    printf("Sorry, this application does not support ");
    printf("Market Market type: [%s]\n", mm_typecode_to_str(matcode));
    exit(1);
  }

  /* find out size of sparse matrix .... */

  if ((ret_code = mm_read_mtx_crd_size(f, &M, &N, &nz)) != 0){

    printf("There was an error reading the file .\n");
    exit(1);
  }

  /* reseve memory for matrices */

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Start of malloc
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // nz; //
  // ==========================================================================================

  *I = (uint *)malloc(nz * sizeof(uint));
  *J = (uint *)malloc(nz * sizeof(uint));
  // val = (double *)malloc(nz * sizeof(double));
  double val;
  *total_number_of_edges = nz;

  /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
  /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
  /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */

  bool succefully_malloc = true;

  if ( *I == NULL ){
    succefully_malloc = false;     
    printf("\nFailed to malloc I");
  }
  if (*J == NULL){
    succefully_malloc = false;     
    printf("\nFailed to malloc J");    
  }

  if ( !succefully_malloc ) {      
    printf("\nSo the program is exiting \n");  
    exit(1) ;
  }
  

  //printf("Start reading file ... \n");
  if (number_of_columns==2){
    
    for (uint i = 0; i < nz; i++) {
      
        //printf("Read line zu 2 : %5u \n" , i );
        
        fscanf(f, "%zu %zu \n", &(*I)[i], &(*J)[i]);
        
        //printf("Read line : %5u \n" , i );
      }
    
  }
  else {
      for (uint i = 0; i < nz; i++) {
        
        //printf("Read line : %5u \n" , i );
        fscanf(f, "%zu %zu %lg\n", &(*I)[i], &(*J)[i], &val);

      }
  
  }
    
    //printf("Filled I and J \n");

  if (f != stdin) {
    fclose(f);
  }

  /************************/
  /* now write out matrix */
  /************************/

  // mm_write_banner(stdout, matcode);
  
  //printf("Dimentions of \n");
  //mm_write_mtx_crd_size(stdout, M, N, nz);

  *total_nodes = M + 1;

  // for (i = 0; i < nz; i++)
  //  fprintf(stdout, "%d %d %20.19g\n", I[i], J[i], val[i]);
}





const char* select_file(int number_of_file){
  const char *file_name;

  switch (number_of_file){
    case 0:
      file_name = "files/celegansneural.mtx";
      break;
    case 1:
      file_name = "files/foldoc.mtx";
      break;
    case 2:
      file_name = "files/language.mtx";
      break;
    case 3:
      file_name = "files/eu-2005.mtx";
      break;
    case 4:
      file_name = "files/wiki-topcats.mtx";
      break;
    case 5:
      file_name = "files/sx-stackoverflow.mtx";
      break;
    case 6:
      file_name = "files/wikipedia-20060925.mtx";
      break;
    case 7:
      file_name = "files/wikipedia-20061104.mtx";
      break;
    case 8:
      file_name = "files/wikipedia-20070206.mtx";
      break;
    case 9:
      file_name = "files/wb-edu.mtx";
      break;
    case 10:
      file_name = "files/indochina-2004.mtx";
      break;
    case 11:
      file_name = "files/uk-2002.mtx";
      break;
    case 12:
      file_name = "files/arabic-2005.mtx";
      break;
    case 13:
      file_name = "files/uk-2005.mtx";
      break;
    case 14:
      file_name = "files/twitter7.mtx";
      break;
  }

  return file_name;
}

int number_columns(int number_of_file){
  int number_of_columns;
  switch (number_of_file){
    case 0:
      number_of_columns=3;
      break;
    case 1:
      number_of_columns=3;
      break;
    case 2:
      number_of_columns=3;
      break;
    case 3:
      number_of_columns=2;
      break;
    case 4:
      number_of_columns=2;
      break;
    case 5:
      number_of_columns=3;
      break;
    case 6:
      number_of_columns=2;
      break;
    case 7:
      number_of_columns=2;
      break;
    case 8:
      number_of_columns=2;
      break;
    case 9:
      number_of_columns=2;
      break;
    case 10:
      number_of_columns=2;
      break;
    case 11:
      number_of_columns=2;
      break;
    case 12:
      number_of_columns=2;
      break;
    case 13:
      number_of_columns=2;
      break;
    case 14:
      number_of_columns=2;
      break;
  }

  return number_of_columns;
}









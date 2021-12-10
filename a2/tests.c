#include "util.h"
#include "io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>

//returns number of query points, where the given NNs are the k closest points
//files "points", "queries", and "indexes_bf" must exist
int nn_check(){
    int n_point;
    int d_point;

    FILE *f_p = fopen("points","r");
    double *point_data = read_points(f_p, &n_point, &d_point);
    fclose(f_p);

    int n_query;
    int d_query;

    FILE *f_q = fopen("queries","r");
    double *query_data = read_points(f_q, &n_query, &d_query);
    fclose(f_q);

    int n_indexes;
    int k_indexes;

    FILE *f_i = fopen("indexes_bf","r");
    int *index_data = read_indexes(f_i, &n_indexes, &k_indexes);
    fclose(f_i);

    //for each query point do check
    int correct_query_counter = 0;
    for(int i = 0; i < n_query; i++){
        //Find distance to farthest NN
        int *farthest_index_NN = index_data + (i*k_indexes+(k_indexes-1));
        double farthest_dist_NN = distance(d_point, query_data+(i*d_point), point_data + *(farthest_index_NN)*d_point); 
        //counter of points with dist smaller or equal to farthest NN 
        int nn_hit_counter = 0; //
        
        for(int j = 0; j < n_point; j++){
            //distance to point in question
            double dist_j = distance(d_point, query_data+(i*d_point), point_data+(j*d_point));
            //count hit - may hit closest outside if there is a tie
            if(dist_j <= farthest_dist_NN){
                nn_hit_counter += 1;
            }
        }
        if(nn_hit_counter >= k_indexes){ //accept overcount due to potential ties for farthest NN
            correct_query_counter += 1;
            if(nn_hit_counter > k_indexes){ //notify is tie occurs
                printf("Query index %d: Number of ties out of closest with farthest NN: %d",i,nn_hit_counter-k_indexes);
            }
        }
    }
    free(point_data);
    free(query_data);
    free(index_data);

    return correct_query_counter;
}

//produce data files and run single test for given sizes
//requires executables: knn-genpoints knn-bruteforce knn-kdtree
//returns number of correct queries
int run_check(int n_point, int n_queries, int d, int k){
    char gen_points[50];
    char gen_queries[50];
    char run_bfknn[50];
    char run_kdknn[50];
    //prepare bash commands
    assert(sprintf(gen_points, "./knn-genpoints %d %d > points",n_point, d));
    assert(sprintf(gen_queries, "./knn-genpoints %d %d > queries",n_queries, d));
    assert(sprintf(run_bfknn, "./knn-bruteforce points queries %d indexes_bf",k));
    assert(sprintf(run_kdknn, "./knn-kdtree points queries %d indexes_kd",k));
    //execute commands
    assert(system(gen_points)!=-1);
    assert(system(gen_queries)!=-1);
    assert(system(run_bfknn)!=-1);
    assert(system(run_kdknn)!=-1);
    //now that all files are written, run test on bruteforce
    return nn_check();
}

//compare output index-files of bruteforce and kd-tree programs using unix "cmp"
//index files indexes_bf and indexes_kd must exist
void kd_bf_cmp(){
    int ret = system("cmp -s indexes_bf indexes_kd");
    if(WEXITSTATUS(ret) == 0){
        printf("knn-bruteforce and knn-kdtree output comparison: SUCCES\n\n");
    }else{
        printf("knn-bruteforce and knn-kdtree output comparison: FAIL\n\n");
    }
}

void perform_test(size_t test_n0, int p, int q, int d, int k){
    int hits = run_check(p,q,d,k);
    printf("Test %zu: n_points=%d, n_queries=%d, d=%d, k=%d\n",test_n0,p,q,d,k);
    printf("Number of queries handled correctly by knn-bruteforce: %d/%d\n",hits,q);
    kd_bf_cmp();
}


int main(){
    //test 1: few points, few queries, low dimensions, few neighbours
    perform_test(1,10,4,2,2);

    //test 2: many points, few queries, high dimensions, few neighbours
    perform_test(2,15000,4,500,2);

    //test 3: medium points, medium queries, high dimensions, medium neighbours
    perform_test(3,500,200,500,10);

    //test 4: few points, many queries, high dimensions, few neighbours
    perform_test(4,10,1000,500,10);    

    //test 5: medium points, medium queries, 1 dimension, 1 neighbour
    perform_test(5,500,200,1,1);    
    
   return 0;
}
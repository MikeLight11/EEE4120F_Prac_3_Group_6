// =========================================================================
// Practical 3: Minimum Energy Consumption Freight Route Optimization
// =========================================================================
//
// GROUP NUMBER:
//
// MEMBERS:
//   - Glen Jones, JNSGLE007
//   - Michael Lighton, LGHMIC003

// ========================================================================
//  PART 2: Minimum Energy Consumption Freight Route Optimization using OpenMPI
// =========================================================================


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <limits.h> // REq for INT_MAX
#include "mpi.h"


#define MAX_N 10 // Max 10 citiees

// ============================================================================
// Global variables
// ============================================================================

int n; // If this is -1, it signals an error/exit
int adj[MAX_N][MAX_N]; // adjacency matrix size allocation
int lcl_best_cost = 2147483647; // Each process' current best cost 
int lcl_best_path[MAX_N]; // Each process' curr best path


// Task struct (ACT AS starting point for processes)
typedef struct {
    int path[MAX_N];
    int visited[MAX_N];
    int depth;
    int cost;
} Task; 

Task tasks[MAX_N * MAX_N];
int num_tasks = 0;


// ============================================================================
// Timer: returns time in seconds
// ============================================================================

double gettime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// ============================================================================
// Usage function
// ============================================================================

void Usage(char *program) {
  printf("Usage: mpirun -np <num> %s [options]\n", program);
  printf("-i <file>\tInput file name\n");
  printf("-o <file>\tOutput file name\n");
  printf("-h \t\tDisplay this help\n");
}

// recursive depth-first search
void branchnbound(int depth, int current_cost, int *path, int *visited, int *local_best, int *local_best_path) {
    if (depth == n) {   // base case (all cities visited)
        if (current_cost < *local_best) {
            *local_best = current_cost;
            for (int i = 0; i < n; i++) {
                 local_best_path[i] = path[i];
            }
        }
        return;
    }

    int last = path[depth - 1]; // curr city
    for (int city = 0; city < n; city++) {
        if (visited[city]) continue; //skip if visited
        int new_cost = current_cost + adj[last][city]; //add cost

        if (new_cost >= *local_best) continue; // If now worse than local best

        visited[city] = 1;
        path[depth] = city;

        // recursive step
        branchnbound(depth + 1, new_cost, path, visited, local_best, local_best_path);
        visited[city] = 0; // backtracking 'un visits so others can visit'
    }
}

int main(int argc, char **argv)
{
    // Every process runs main but action depends on rank (mainly 0 for admin)

    int rank, nprocs;
    int opt;
    int i, j;
    char *input_file = NULL;
    char *output_file = NULL;
    FILE *infile = NULL;
    FILE *outfile = NULL;
    int success_flag = 1; // 1 = good, 0 = error/help encountered

    // Initialize MPI
    MPI_Init(&argc, &argv);     // Starts the MPI environment
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Gets the ID of cur prc (0,1,2))
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs); // Gives TOTAL number of processes

    MPI_Barrier(MPI_COMM_WORLD); 
    double start_init = MPI_Wtime();

    if (rank == 0) { // Parsing argument
        n = -1; 

        while ((opt = getopt(argc, argv, "i:o:h")) != -1)
        {
            switch (opt)
            {
                case 'i':
                    input_file = optarg;
                    break;

                case 'o':
                    output_file = optarg;
                    break;

                case 'h':
                    Usage(argv[0]);
                    success_flag = 0; 
                    break;

                default:
                    Usage(argv[0]);
                    success_flag = 0;
            }
        }

        
    
        if (success_flag) {
            infile = fopen(input_file, "r");
            if (infile == NULL) {
                fprintf(stderr, "Error: Cannot open input file '%s'\n", input_file);
                perror("");
                success_flag = 0;
            } else {
                
                fscanf(infile, "%d", &n);

                for (i = 1; i < n; i++)
                {
                    for (j = 0; j < i; j++)
                    {
                        fscanf(infile, "%d", &adj[i][j]);
                        adj[j][i] = adj[i][j];
                    }
                }
                fclose(infile);
            }
        }
        if (success_flag) {
            outfile = fopen(output_file, "w");
            if (outfile == NULL) {
                fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
                perror("");
                success_flag = 0;
            }
        }

    }


    // Share to all priv memories (1-all)
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD); // Master broadcasts info to everyone
    // &n data to send , sending 1 item, data type, sender (root), group of all processes)
    
    if (n == -1) {
        MPI_Finalize();
        return 0; 
    }

    // Sends entire adjacency matrix treated as 1D strip of mem. sent to every worker
    MPI_Bcast(&adj[0][0], MAX_N * MAX_N, MPI_INT, 0, MPI_COMM_WORLD);

    
    //printf("Process %d received adjacency matrix:\n", rank); // Printing out received adj
    //for (i = 0; i < n; i++) {
        //for (j = 0; j < n; j++) {
            //printf("%d ", adj[i][j]);
        //}
        //printf("\n");
    //}
    //printf("\n");

    double end_init = MPI_Wtime();
    double t_init = end_init - start_init;
    double t_comp_start = MPI_Wtime();
        
    // TODO: compute solution to minimum energy consumption problem here and write to output file
    // Be careful on which process rank writes to the output file to avoid conflicts!
    
    // Task Generation (DEpth of 2 )
    // Generate all depth-2 partial paths starting from City 1 (index of 0)
    // Generate every possible par of first 2 stops
    /// work units divided by proc
    for (int a = 1; a < n; a++) {          // 1st city after City1
        for (int b = 1; b < n; b++) {      // 2nd city
            if (b == a) continue;          // skips - cant visist same twice

            // Clear visited
            for (int k = 0; k < n; k++) {
                tasks[num_tasks].visited[k] = 0;
            }

            // Mark as  as visited
            tasks[num_tasks].visited[0] = 1;   // City1 
            tasks[num_tasks].visited[a] = 1;   // 1st
            tasks[num_tasks].visited[b] = 1;    // 2nd

            //partial path
            tasks[num_tasks].path[0] = 0;      
            tasks[num_tasks].path[1] = a;      
            tasks[num_tasks].path[2] = b;      

            // depth and cosst
            tasks[num_tasks].depth = 3;
            tasks[num_tasks].cost  = adj[0][a] + adj[a][b];

            // Next task
            num_tasks++;
        }
    }
    

 // Parallel comp
    MPI_Barrier(MPI_COMM_WORLD);  // synchronisation wall for fair timing
    double start_comp = MPI_Wtime();

    //local result
    int local_best = INT_MAX;
    int local_best_path[MAX_N];
    int current_path[MAX_N];
    int current_visited[MAX_N];
    
    

    // work distribution FOR RANKS
    for (int t = rank; t < num_tasks; t += nprocs) {
        for (int k = 0; k < n; k++) {
            current_path[k] = tasks[t].path[k];
            current_visited[k] = tasks[t].visited[k];
        }
        branchnbound(tasks[t].depth, tasks[t].cost, current_path, current_visited, &local_best, local_best_path);
    }


    // sendBuf structure: [Cost, City0, City1,.,CityN-1]
    int sendBuf[MAX_N + 1];
    sendBuf[0] = local_best;
    for(int i = 0; i < n; i++) {
        sendBuf[i+1] = local_best_path[i];
    }

    int *recvBuf = NULL; // allocating buffer for rank 0 received data
    if (rank == 0) {
        recvBuf = (int *)malloc(nprocs * (MAX_N + 1) * sizeof(int));
    }

    // Use MPI_Gather to bring all data to Rank 0
    MPI_Gather(sendBuf, MAX_N + 1, MPI_INT, recvBuf, MAX_N + 1, MPI_INT, 0, MPI_COMM_WORLD);
    //  what wach proc sends, num elements, data type, where rank0 stores, num elemepnrs, datatype rec, proc grp

    MPI_Barrier(MPI_COMM_WORLD);
    double end_comp = MPI_Wtime();
    double t_comp = end_comp - start_comp;
    

    if (rank == 0) {
        int final_best_cost = INT_MAX;
        int final_best_path[MAX_N];

        // Step 1: Search the gathered results for the absolute minimum
        for (int p = 0; p < nprocs; p++) {
            int current_rank_cost = recvBuf[p * (MAX_N + 1)];
            if (current_rank_cost < final_best_cost) {
                final_best_cost = current_rank_cost;
                // Extract the path sitting right after this cost
                for (int k = 0; k < n; k++) {
                    final_best_path[k] = recvBuf[p * (MAX_N + 1) + (k + 1)];
                }
            }
        }
        printf("\n==============================================\n");
        printf("Distributed Branch & Bound Results\n");
        printf("==============================================\n");
        printf("Processors used:    %d\n", nprocs);
        printf("Minimum Cost:       %d\n", final_best_cost);
        printf("Optimal Path:       ");
        for (int k = 0; k < n; k++) printf("%d ", final_best_path[k] + 1);
        printf("\nInit Time (Tinit):  %.6f seconds\n", t_init);
        printf("Comp Time (Tcomp):  %.6f seconds\n", t_comp);
        printf("Total Wall Time:    %.6f seconds\n", t_init + t_comp);
        printf("==============================================\n");

        // Write to file - only rank 0
        if (outfile != NULL) {
            fprintf(outfile, "Nodes: %d\n", nprocs);
            fprintf(outfile, "Best Cost: %d\n", final_best_cost);
            fprintf(outfile, "Best Path: ");
            for (int k = 0; k < n; k++) fprintf(outfile, "%d ", final_best_path[k] + 1);
            fprintf(outfile, "\nTinit: %.6f\n", t_init);
            fprintf(outfile, "Tcomp: %.6f\n", t_comp);
            fclose(outfile);
            printf("Results successfully saved to: %s\n", output_file);
        }

        free(recvBuf); // cleanup mem
    }
    

    MPI_Finalize();
    return 0;
}

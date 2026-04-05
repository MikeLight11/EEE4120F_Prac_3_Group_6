// =========================================================================
// Practical 3: Minimum Energy Consumption Freight Route Optimization
// =========================================================================
//
// GROUP NUMBER:
//
// MEMBERS:
//   - Member 1 Name, Student Number
//   - Member 2 Name, Student Number

// ========================================================================
//  PART 1: Minimum Energy Consumption Freight Route Optimization using OpenMP
// =========================================================================


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <omp.h>

#define MAX_N 20

// ============================================================================
// Global variables
// ============================================================================

int procs = 1;

int n;
int adj[MAX_N][MAX_N];

int best_cost;
int best_path[MAX_N];
omp_lock_t my_lock;

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
  printf("Usage: %s [options]\n", program);
  printf("-p <num>\tNumber of processors/threads to use\n");
  printf("-i <file>\tInput file name\n");
  printf("-o <file>\tOutput file name\n");
  printf("-h \t\tDisplay this help\n");
}

//Branch and Bound function
void branchnbound(int depth, int current_cost, int *path, int *visited, int *local_best, int *local_path)
{
    // checks if all cities have been placed in the path
    if (depth == n) {

        // Checks if this complete route better than our best so far
        if (current_cost < *local_best) {

            // Updates this thread's local best cost
            *local_best = current_cost;

            // Save the actual route that achieved this cost
            for (int i = 0; i < n; i++) {
                local_path[i] = path[i];
            }

            // Now update the GLOBAL best so other threads can prune better
            // We need a lock here because multiple threads share best_cost
            omp_set_lock(&my_lock);
            if (current_cost < best_cost) {
                best_cost = current_cost;
                for (int i = 0; i < n; i++) {
                    best_path[i] = path[i];
                }
            }
            omp_unset_lock(&my_lock);
        }
        return;  // stop recursing as path is complete
    }

    // Since the array is indexed from 0 and City 1 is at index 0 therefore 1-0=0 so depth-1
    int last = path[depth - 1];

    // Try every city as the next stop
    for (int city = 0; city < n; city++) {

        // Checks if this city has been visited
        if (visited[city]) continue;

        // Calculates the new cost from this city to the next
        int new_cost = current_cost + adj[last][city];

        // Checks if the partial cost already matches or exceeds the best COMPLETE route
        if (new_cost >= *local_best) continue;  //skip

        visited[city] = 1;       // mark this city as visited
        path[depth] = city;      // add this city to current path

        branchnbound(depth + 1, new_cost, path, visited, local_best, local_path);

        visited[city] = 0;       // unmark this city to make available again for other branches
    }
}

// Define the Task struct
typedef struct {
    int path[MAX_N];
    int visited[MAX_N];
    int depth;
    int cost;
} Task; 

// Declare the task array directly — no malloc needed
Task tasks[MAX_N * MAX_N];
int num_tasks = 0;


int main(int argc, char **argv)
{

    double t_init_start = gettime(); // Initialisation timing start
    
    int opt;
    int i, j;
    char *input_file = NULL;
    char *output_file = NULL;
    FILE *infile = NULL;
    FILE *outfile = NULL;
    int success_flag = 1; // 1 = good, 0 = error/help encountered
    
    

    while ((opt = getopt(argc, argv, "p:i:o:h")) != -1)
    {
        switch (opt)
        {
            case 'p':
            {
                procs = atoi(optarg);
                break;
            }

            case 'i':
            {
                input_file = optarg;
                break;
            }

            case 'o':
            {
                output_file = optarg;
                break;
            }

            case 'h':
            {
                Usage(argv[0]);
                success_flag = 0; 
                break;
            }

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
                    adj[j][i] = adj[i][j]; //i = starting city and j = destination city. But City 1 = index 0, City 2 = 1, and so on
                }
            }
        }
    }

    if (success_flag && output_file != NULL) {
        outfile = fopen(output_file, "w");
        if (outfile == NULL) {
            fprintf(stderr, "Error: Cannot open output file '%s'\n", output_file);
            perror("");
            success_flag = 0;
        }
    }

    if (!success_flag) return 1;

    double t_init_end = gettime(); // Initialisation timing end

    printf("Running with %d processes/threads on a graph with %d nodes\n", procs, n);

    double t_comp_start = gettime(); // Computation timing start
    
    // TODO: compute solution to minimum energy consumption problem here and write to outfile

    best_cost = INT_MAX;
    omp_init_lock(&my_lock);


    // Generate all depth-2 partial paths starting from City 1 (index 0)
    for (int a = 1; a < n; a++) {          // first city to visit after City1
        for (int b = 1; b < n; b++) {      // second city to visit after that
            if (b == a) continue;          // skips since you can't visit the same city twice

            // Clear visited array manually
            for (int k = 0; k < n; k++) {
                tasks[num_tasks].visited[k] = 0;
            }

            // Mark the three cities in this partial path as visited
            tasks[num_tasks].visited[0] = 1;   // City1 always visited first
            tasks[num_tasks].visited[a] = 1;   // first stop
            tasks[num_tasks].visited[b] = 1;   // second stop

            // Build the partial path
            tasks[num_tasks].path[0] = 0;      // City1
            tasks[num_tasks].path[1] = a;      // first stop
            tasks[num_tasks].path[2] = b;      // second stop

            // Record how deep we are and the cost so far
            tasks[num_tasks].depth = 3;
            tasks[num_tasks].cost  = adj[0][a] + adj[a][b];

            // Move to the next task slot
            num_tasks++;
        }
    }

    omp_set_num_threads(procs);

    #pragma omp parallel
    {
        // each thread gets its own copy of these variables
        int local_path[MAX_N];       // working path for this thread
        int local_best_path[MAX_N];  // best path found by this thread
        int local_visited[MAX_N];    // working visited array for this thread
        int local_best = INT_MAX;    // best cost found by this thread INT_MAX means "nothing found yet"

        #pragma omp for schedule(dynamic, 1)
        for (int t = 0; t < num_tasks; t++) {

            // Fetch our current best global cost
            omp_set_lock(&my_lock); //needs a lock as this is a shared varible among threads
            int current_global_best = best_cost;
            omp_unset_lock(&my_lock);

            if (tasks[t].cost >= current_global_best) continue; // We skip if our partial cost is already worse than the best

            if (current_global_best < local_best) {
                local_best = current_global_best;
            }

            // Copy task state into thread-local working arrays
            for (int i = 0; i < n; i++) {
                local_path[i]    = tasks[t].path[i];
                local_visited[i] = tasks[t].visited[i];
            }

            // Run Branch & Bound function from this task's starting point
            branchnbound(tasks[t].depth, tasks[t].cost, local_path, local_visited, &local_best, local_best_path);
        }

        // After all tasks done we now merge this thread's best into global best
        omp_set_lock(&my_lock);
        if (local_best < best_cost) {
            best_cost = local_best;
            for (int i = 0; i < n; i++) {
                best_path[i] = local_best_path[i];
            }
        }
        omp_unset_lock(&my_lock);
    }

    omp_destroy_lock(&my_lock); // Clean up the lock

    double t_comp_end = gettime(); // Computation timing end
 
    double t_init = t_init_end - t_init_start;
    double t_comp = t_comp_end - t_comp_start;
    double t_total = t_init + t_comp;
 
    printf("Tinit  = %.6f s\n", t_init);
    printf("Tcomp  = %.6f s\n", t_comp);
    printf("Ttotal = %.6f s\n", t_total);

    printf("Best cost: %d\n", best_cost);
    printf("Best path: ");
    for (int k = 0; k < n; k++) {
        printf("%d ", best_path[k] + 1);
    }
    printf("\n");

    if (outfile != NULL) {
        fprintf(outfile, "Best cost: %d\n", best_cost);
        fprintf(outfile, "Best path: ");
        for (int k = 0; k < n; k++) {
            fprintf(outfile, "%d ", best_path[k] + 1);
        }
        fprintf(outfile, "\n");
        fclose(outfile);
    }

    return 0;
}

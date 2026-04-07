// =========================================================================
// Practical 3: Minimum Energy Consumption Freight Route Optimization
// =========================================================================
//
// GROUP NUMBER: 6
//
// MEMBERS:
//   - Member 1 Michael Lighton, LGHMIC003
//   - Member 2 Glen Jones, JNSGLE007

// ========================================================================
//  PART 1: Minimum Energy Consumption Freight Route Optimization using OpenMP
// =========================================================================


#include <stdio.h>
#include <stdlib.h>
#include <limits.h> //required for INT_MAX
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <omp.h>

#define MAX_N 10

// ============================================================================
// Global variables
// ============================================================================

int procs = 1;

int n;
int adj[MAX_N][MAX_N];

int global_best_cost;
int global_best_path[MAX_N];
omp_lock_t lock;

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
            // We need a lock here because multiple threads share global_best_cost
            omp_set_lock(&lock);
            if (current_cost < global_best_cost) {
                global_best_cost = current_cost;
                for (int i = 0; i < n; i++) {
                    global_best_path[i] = path[i];
                }
            }
            omp_unset_lock(&lock);
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


Task tasks[MAX_N * MAX_N];


int main(int argc, char **argv)
{

    double time_initialization_start = gettime(); // Initialisation timing start
    
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

    // TODO: compute solution to minimum energy consumption problem here and write to outfile

    omp_set_num_threads(procs); // Set the number of threads for OpenMP

    double time_initialization_end = gettime(); // Initialisation timing end

    printf("Running with %d processes/threads on a graph with %d nodes\n", procs, n);

    double time_computation_start = gettime(); // Computation timing start

    global_best_cost = INT_MAX;  // Initialize global best cost to infinity
    omp_init_lock(&lock);  // Initialize the lock for synchronizing access to global_best_cost and global_best_path

    // Generate all depth-2 partial paths starting from City 1 (index 0)
    // City1 -> CityA -> CityB, where A and B are different cities from 2 to n
    int index = 0; // Index for tasks array
    for (int a = 1; a < n; a++) {          // first city to visit after City1
        for (int b = 1; b < n; b++) {      // second city to visit after that
            if (b == a) continue;          // skips since you can't visit the same city twice

            // Clear visited array manually
            for (int k = 0; k < n; k++) {
                tasks[index].visited[k] = 0;
            }

            // Mark the three cities in this partial path as visited
            tasks[index].visited[0] = 1;   // City1 always visited first
            tasks[index].visited[a] = 1;   // first stop
            tasks[index].visited[b] = 1;   // second stop

            // Build the partial path
            tasks[index].path[0] = 0;      // City1 is always the starting point (index 0)
            tasks[index].path[1] = a;      // first stop
            tasks[index].path[2] = b;      // second stop

            tasks[index].depth = 3; // We have placed 3 cities in the path so far (City1, CityA, CityB)
            tasks[index].cost  = adj[0][a] + adj[a][b]; // Cost of this partial path so far (City1 to CityA, then CityA to CityB)

            // Move to the next task slot
            index++;
        }
    }

    #pragma omp parallel // Start of parallel region
    {
        // each thread gets its own copy of these variables
        int local_path[MAX_N];       // working path for this thread
        int local_best_path[MAX_N];  // best path found by this thread
        int local_visited[MAX_N];    // working visited array for this thread
        int local_best_cost = INT_MAX;    // best cost found by this thread, initialized to infinity

        // Distribute the tasks among threads with dynamic scheduling for better load balancing
        #pragma omp for schedule(dynamic, 2) // dynamic scheduling with chunk size of 2 for better load balancing
        for (int t = 0; t < index; t++) {

            // Fetch our current best global cost
            omp_set_lock(&lock); //needs a lock as this is a shared varible among threads
            int current_global_best = global_best_cost; // read the global best cost into a local variable for this thread to use while processing this task
            omp_unset_lock(&lock); 

            if (tasks[t].cost >= current_global_best) continue; // We skip if our partial cost is already worse than the best

            if (current_global_best < local_best_cost) { // If the global best is better (less) than our local best, we update our local best to it.
                local_best_cost = current_global_best;
            }

            // Copy task state into thread-local working arrays
            for (int i = 0; i < n; i++) {
                local_path[i]    = tasks[t].path[i]; // copy the partial path for this task into our local path
                local_visited[i] = tasks[t].visited[i]; // copy the visited array for this task into our local visited array
            }

            // Run Branch & Bound function from this task's starting point
            branchnbound(tasks[t].depth, tasks[t].cost, local_path, local_visited, &local_best_cost, local_best_path);
        }

        // We now merge this thread's best into global best
        omp_set_lock(&lock);
        if (local_best_cost < global_best_cost) { 
            global_best_cost = local_best_cost; // Update global best cost if this thread found a better solution
            for (int i = 0; i < n; i++) { // Update the global best path if this thread found a better solution
                global_best_path[i] = local_best_path[i];
            }
        }
        omp_unset_lock(&lock);
    }

    omp_destroy_lock(&lock); // Clean up the lock

    double time_computation_end = gettime(); // Computation timing end
 
    double time_initialization = time_initialization_end - time_initialization_start;
    double time_computation = time_computation_end - time_computation_start;
    double time_total = time_initialization + time_computation;
    
    // Print results to console
    printf("Initialization time = %.6f s\n", time_initialization);
    printf("Computation time = %.6f s\n", time_computation);
    printf("Total time = %.6f s\n", time_total);

    printf("Best cost: %d\n", global_best_cost);
    printf("Best path: ");
    for (int k = 0; k < n; k++) {
        printf("%d ", global_best_path[k] + 1);
    }
    printf("\n");

    // Write results to output file if specified
    if (outfile != NULL) {
        fprintf(outfile, "Best cost: %d\n", global_best_cost);
        fprintf(outfile, "Best path: ");
        for (int k = 0; k < n; k++) {
            fprintf(outfile, "%d ", global_best_path[k] + 1);
        }
        fprintf(outfile, "\n");
        fclose(outfile);
    }

    return 0;
}

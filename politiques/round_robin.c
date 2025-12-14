#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "gui.h"
#include "console_display.h"


static SimulationResult simulate_round_robin(Process procs[], int n, int QUANTUM) {
    SimulationResult result;
    
    
    static int timeline[1000];
    static int start[100];
    static int end[100];
    static int remaining[100];
    static int in_queue[100];
    static int queue[100];
    
    result.procs = procs;
    result.n = n;
    result.timeline = timeline;
    result.start = start;
    result.end = end;
    result.init_prio = NULL;
    result.levels = NULL;
    
    
    int time = 0;
    int timeline_len = 0;
    int done = 0;
    int q_size = 0;
    
    for (int i = 0; i < n; i++) {
        remaining[i] = procs[i].duration;
        start[i] = -1;
        end[i] = -1;
        in_queue[i] = 0;
    }
    
   
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival == 0) {
            queue[q_size++] = i;
            in_queue[i] = 1;
        }
    }
    
    
    while (done < n) {
        
        if (q_size == 0) {
            timeline[timeline_len++] = -1;
            time++;
            
            
            for (int i = 0; i < n; i++) {
                if (procs[i].arrival == time && !in_queue[i] && remaining[i] > 0) {
                    queue[q_size++] = i;
                    in_queue[i] = 1;
                }
            }
            continue;
        }
        
        
        int pid = queue[0];
        for (int i = 0; i < q_size - 1; i++)
            queue[i] = queue[i + 1];
        q_size--;
        in_queue[pid] = 0;
        
        
        if (start[pid] == -1)
            start[pid] = time;
        
        
        int quantum_used = 0;
        while (quantum_used < QUANTUM && remaining[pid] > 0) {
            timeline[timeline_len++] = pid;
            remaining[pid]--;
            quantum_used++;
            time++;
            
           
            for (int i = 0; i < n; i++) {
                if (procs[i].arrival == time && !in_queue[i] && remaining[i] > 0) {
                    queue[q_size++] = i;
                    in_queue[i] = 1;
                }
            }
        }
        
        
        if (remaining[pid] > 0) {
            queue[q_size++] = pid;
            in_queue[pid] = 1;
        } else {
            end[pid] = time;
            done++;
        }
    }
    
    result.timeline_len = timeline_len;
    result.total_time = time;
    
    return result;
}


static void copy_to_gui_result(SimulationResult *result, int QUANTUM) {
    if (!current_result) return;
    
    sprintf(current_result->algo_name, "Round Robin (q=%d)", QUANTUM);
    current_result->quantum = QUANTUM;
    current_result->process_count = result->n;
    
    
    current_result->processes = malloc(sizeof(Process) * result->n);
    if (!current_result->processes) {
        fprintf(stderr, "Erreur: malloc échoué pour processes\n");
        return;
    }
    
    for (int i = 0; i < result->n; i++) {
        current_result->processes[i] = result->procs[i];
    }
    
    
    current_result->timeline_len = result->timeline_len;
    for (int i = 0; i < result->timeline_len; i++) {
        current_result->timeline[i] = result->timeline[i];
    }
    
    
    for (int i = 0; i < result->n; i++) {
        current_result->start[i] = result->start[i];
        current_result->end[i] = result->end[i];
    }
    
    
    float sumT = 0.0f, sumW = 0.0f;
    for (int i = 0; i < result->n; i++) {
        current_result->turnaround[i] = result->end[i] - result->procs[i].arrival;
        current_result->wait[i] = current_result->turnaround[i] - result->procs[i].duration;
        sumT += current_result->turnaround[i];
        sumW += current_result->wait[i];
    }
    
    current_result->avg_turnaround = sumT / result->n;
    current_result->avg_wait = sumW / result->n;
}

void round_robin(Process procs[], int n, int QUANTUM) {
    
    SimulationResult result = simulate_round_robin(procs, n, QUANTUM);
    
    
    if (capture_mode && current_result) {
        
        copy_to_gui_result(&result, QUANTUM);
    } else {
       
        display_console_results(
            "ROUND ROBIN",
            QUANTUM,
            &result,
            NULL
        );
    }
}

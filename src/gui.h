#ifndef GUI_H
#define GUI_H

#include "process.h"


typedef struct {
    int timeline[2000];      
    int timeline_len;
    int start[MAXP];
    int end[MAXP];
    int turnaround[MAXP];
    int wait[MAXP];
    float avg_turnaround;
    float avg_wait;
    int process_count;
    Process *processes;      
    int quantum;             
    char algo_name[50];      
} SchedulingResult;

extern SchedulingResult *current_result;
extern int capture_mode;

void lancer_interface_gtk(Process procs[], int count);


void fifo(Process procs[], int n);
void round_robin(Process procs[], int n, int quantum);
void priorite(Process procs[], int n);
void multi_level(Process procs[], int n);
void multi_level_static(Process procs[], int n);


#endif

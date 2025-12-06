#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "gui.h"

void round_robin(Process procs[], int n, int QUANTUM) {
    
    // ═══════════════════════════════════════
    //     MODE CAPTURE POUR GUI
    // ═══════════════════════════════════════
    if (capture_mode && current_result) {
        sprintf(current_result->algo_name, "Round Robin (q=%d)", QUANTUM);
        current_result->quantum = QUANTUM;
        current_result->processes = malloc(sizeof(Process) * n);
        if (!current_result->processes) return;
        
        // Copier les processus
        for (int i = 0; i < n; i++) {
            current_result->processes[i] = procs[i];
        }
        
        int time = 0;
        current_result->timeline_len = 0;
        int remaining[n], start[n], end[n];
        int in_queue[n];
        int done = 0;
        
        for (int i = 0; i < n; i++) {
            remaining[i] = procs[i].duration;
            start[i] = -1;
            end[i] = -1;
            in_queue[i] = 0;
        }
        
        int queue[100];
        int q_size = 0;
        
        // Ajouter les processus arrivés à temps 0
        for (int i = 0; i < n; i++) {
            if (procs[i].arrival == 0) {
                queue[q_size++] = i;
                in_queue[i] = 1;
            }
        }
        
        while (done < n) {
            if (q_size == 0) {
                current_result->timeline[current_result->timeline_len++] = -1;
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
                queue[i] = queue[i+1];
            q_size--;
            in_queue[pid] = 0;
            
            if (start[pid] == -1)
                start[pid] = time;
            
            int quantum_used = 0;
            while (quantum_used < QUANTUM && remaining[pid] > 0) {
                current_result->timeline[current_result->timeline_len++] = pid;
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
        
        // Copier les résultats
        for (int i = 0; i < n; i++) {
            current_result->start[i] = start[i];
            current_result->end[i] = end[i];
        }
        
        // Calculer les statistiques
        float sumT = 0, sumW = 0;
        for (int i = 0; i < n; i++) {
            current_result->turnaround[i] = end[i] - procs[i].arrival;
            current_result->wait[i] = current_result->turnaround[i] - procs[i].duration;
            sumT += current_result->turnaround[i];
            sumW += current_result->wait[i];
        }
        
        current_result->avg_turnaround = sumT / n;
        current_result->avg_wait = sumW / n;
        current_result->process_count = n;
        
        return; // Ne pas afficher en console
    }
    
    // ═══════════════════════════════════════
    //     MODE CONSOLE (ORIGINAL)
    // ═══════════════════════════════════════
    printf("\n═══════════════════════════════════════════════════\n");
    printf("                 ROUND ROBIN (q=%d)\n", QUANTUM);
    printf("═══════════════════════════════════════════════════\n\n");
    
    int time = 0;
    int timeline[1000];
    int timeline_len = 0;
    int remaining[n], start[n], end[n];
    int in_queue[n];
    int done = 0;
    
    for (int i = 0; i < n; i++) {
        remaining[i] = procs[i].duration;
        start[i] = -1;
        end[i] = -1;
        in_queue[i] = 0;
    }
    
    int queue[100];
    int q_size = 0;
    
    // Add initial arrivals at time 0
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
            queue[i] = queue[i+1];
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
    
    printf("CHRONOLOGIE D'EXÉCUTION:\n");
    printf("─────────────────────────\n");
    for (int t = 0; t < timeline_len; t++) {
        if (timeline[t] == -1)
            printf("[IDLE:%d→%d] ", t, t+1);
        else
            printf("[%s:%d→%d] ", procs[timeline[t]].name, t, t+1);
        if ((t+1)%8==0) printf("\n");
    }
    
    printf("\n\nDIAGRAMME DE GANTT:\n");
    printf("───────────────────\n");
    printf("Time ");
    for (int t = 0; t <= timeline_len && t <= 50; t++)
        printf("%2d ", t);
    printf("\n");
    
    for (int i = 0; i < n; i++) {
        printf("%-4s ", procs[i].name);
        for (int t = 0; t < timeline_len && t <= 50; t++)
            printf("%s", timeline[t] == i ? "## " : "   ");
        printf("\n");
    }
    
    printf("\nSTATISTIQUES DES PROCESSUS:\n");
    printf("┌──────┬─────────┬───────┬─────────┬───────┬────────────┬─────────┐\n");
    printf("│ Proc │ Arrivée │ Durée │ Début   │ Fin   │ Turnaround │ Attente │\n");
    printf("├──────┼─────────┼───────┼─────────┼───────┼────────────┼─────────┤\n");
    
    float sumT=0, sumW=0;
    for (int i = 0; i < n; i++) {
        int turn = end[i] - procs[i].arrival;
        int wait = turn - procs[i].duration;
        sumT+=turn; 
        sumW+=wait;
        printf("│ %-4s │ %7d │ %5d │ %7d │ %5d │ %10d │ %7d │\n",
               procs[i].name, procs[i].arrival, procs[i].duration,
               start[i], end[i], turn, wait);
    }
    printf("└──────┴─────────┴───────┴─────────┴───────┴────────────┴─────────┘\n\n");
    printf("Temps de rotation moyen: %.2f\n", sumT / n);
    printf("Temps d'attente moyen:   %.2f\n\n", sumW / n);
}

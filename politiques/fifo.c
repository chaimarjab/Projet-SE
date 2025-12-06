#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "gui.h"

// FIFO = First Come First Served (pas de préemption)

void fifo(Process procs[], int n) {
    
    // ═══════════════════════════════════════
    //     MODE CAPTURE POUR GUI
    // ═══════════════════════════════════════
    if (capture_mode && current_result) {
        strcpy(current_result->algo_name, "FIFO");
        current_result->quantum = 0;
        current_result->processes = malloc(sizeof(Process) * n);
        if (!current_result->processes) return;

        // Copier et trier les processus selon l'arrivée
        for (int i = 0; i < n; i++) {
            current_result->processes[i] = procs[i];
        }
        
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                if (current_result->processes[j].arrival < current_result->processes[i].arrival) {
                    Process tmp = current_result->processes[i];
                    current_result->processes[i] = current_result->processes[j];
                    current_result->processes[j] = tmp;
                }
            }
        }

        // Simuler l'ordonnancement
        int time = 0;
        current_result->timeline_len = 0;
        
        for (int i = 0; i < n; i++) {
            Process *p = &current_result->processes[i];
            
            // CPU Idle si nécessaire
            while (time < p->arrival) {
                current_result->timeline[current_result->timeline_len++] = -1;
                time++;
            }
            
            // Exécuter le processus
            current_result->start[i] = time;
            for (int d = 0; d < p->duration; d++) {
                current_result->timeline[current_result->timeline_len++] = i;
                time++;
            }
            current_result->end[i] = time;
        }
        
        // Calculer les statistiques
        float sumT = 0, sumW = 0;
        for (int i = 0; i < n; i++) {
            current_result->turnaround[i] = current_result->end[i] - current_result->processes[i].arrival;
            current_result->wait[i] = current_result->start[i] - current_result->processes[i].arrival;
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
    printf("                 ORDONNANCEMENT FIFO\n");
    printf("═══════════════════════════════════════════════════\n\n");

    // Trier selon l'arrivée
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (procs[j].arrival < procs[i].arrival) {
                Process tmp = procs[i];
                procs[i] = procs[j];
                procs[j] = tmp;
            }

    int time = 0;
    int timeline[1000];
    int timeline_len = 0;
    int start[n], end[n];

    for (int i = 0; i < n; i++) {
        if (time < procs[i].arrival) {
            // Idle CPU
            while (time < procs[i].arrival) {
                timeline[timeline_len++] = -1;
                time++;
            }
        }

        start[i] = time;
        for (int d = 0; d < procs[i].duration; d++) {
            timeline[timeline_len++] = i;
            time++;
        }
        end[i] = time;
    }

    // CHRONOLOGIE
    printf("CHRONOLOGIE D'EXÉCUTION:\n");
    printf("─────────────────────────\n");

    for (int t = 0; t < timeline_len; t++) {
        if (timeline[t] == -1)
            printf("[IDLE:%d→%d] ", t, t+1);
        else
            printf("[%s:%d→%d] ", procs[timeline[t]].name, t, t+1);
        if ((t + 1) % 8 == 0) printf("\n");
    }

    // GANTT
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

    // STATISTIQUES
    printf("\nSTATISTIQUES DES PROCESSUS:\n");
    printf("┌──────┬─────────┬───────┬─────────┬───────┬─────┬────────────┬─────────┐\n");
    printf("│ Proc │ Arrivée │ Durée │ Début   │ Fin   │ TR  │ Turnaround │ Attente │\n");
    printf("├──────┼─────────┼───────┼─────────┼───────┼─────┼────────────┼─────────┤\n");

    float sumT = 0, sumW = 0;

    for (int i = 0; i < n; i++) {
        int turnaround = end[i] - procs[i].arrival;
        int wait = start[i] - procs[i].arrival;

        sumT += turnaround;
        sumW += wait;

        printf("│ %-4s │ %7d │ %5d │ %7d │ %5d │ %3d │ %10d │ %7d │\n",
               procs[i].name, procs[i].arrival, procs[i].duration,
               start[i], end[i], end[i] - start[i],
               turnaround, wait);
    }

    printf("└──────┴─────────┴───────┴─────────┴───────┴─────┴────────────┴─────────┘\n\n");

    printf("Temps de rotation moyen: %.2f\n", sumT / n);
    printf("Temps d'attente moyen:   %.2f\n\n", sumW / n);
}

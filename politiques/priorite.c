#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "gui.h"
#include "console_display.h"

/* ═══════════════════════════════════════════════════════════
   CORE: Simuler Priorité Préemptive et retourner les résultats
   ═══════════════════════════════════════════════════════════ */
static SimulationResult simulate_priority(Process procs[], int n) {
    SimulationResult result;
    
    // Allouer mémoire pour les tableaux locaux
    static int timeline[1000];
    static int start[100];
    static int end[100];
    static int remaining[100];
    
    result.procs = procs;
    result.n = n;
    result.timeline = timeline;
    result.start = start;
    result.end = end;
    result.init_prio = NULL;
    result.levels = NULL;
    
    // Initialisation
    int time = 0;
    int timeline_len = 0;
    int done = 0;
    
    for (int i = 0; i < n; i++) {
        remaining[i] = procs[i].duration;
        start[i] = -1;
        end[i] = -1;
    }
    
    // Simulation
    while (done < n) {
        // Trouver le processus avec la plus haute priorité
        int best = -1;
        for (int i = 0; i < n; i++) {
            if (procs[i].arrival <= time && remaining[i] > 0) {
                if (best == -1 ||
                    procs[i].priority > procs[best].priority ||
                    (procs[i].priority == procs[best].priority &&
                     procs[i].arrival < procs[best].arrival)) {
                    best = i;
                }
            }
        }
        
        // Si aucun processus n'est prêt
        if (best == -1) {
            timeline[timeline_len++] = -1;
            time++;
            continue;
        }
        
        // Marquer le début si c'est la première exécution
        if (start[best] == -1)
            start[best] = time;
        
        // Exécuter 1 unité de temps
        timeline[timeline_len++] = best;
        remaining[best]--;
        time++;
        
        // Marquer la fin si terminé
        if (remaining[best] == 0) {
            end[best] = time;
            done++;
        }
    }
    
    result.timeline_len = timeline_len;
    result.total_time = time;
    
    return result;
}

/* ═══════════════════════════════════════════════════════════
   HELPER: Copier les résultats vers la structure GUI
   ═══════════════════════════════════════════════════════════ */
static void copy_to_gui_result(SimulationResult *result) {
    if (!current_result) return;
    
    strcpy(current_result->algo_name, "Priorité Préemptive");
    current_result->quantum = 0;
    current_result->process_count = result->n;
    
    // Copier les processus
    current_result->processes = malloc(sizeof(Process) * result->n);
    if (!current_result->processes) {
        fprintf(stderr, "Erreur: malloc échoué pour processes\n");
        return;
    }
    
    for (int i = 0; i < result->n; i++) {
        current_result->processes[i] = result->procs[i];
    }
    
    // Copier la timeline
    current_result->timeline_len = result->timeline_len;
    for (int i = 0; i < result->timeline_len; i++) {
        current_result->timeline[i] = result->timeline[i];
    }
    
    // Copier start/end
    for (int i = 0; i < result->n; i++) {
        current_result->start[i] = result->start[i];
        current_result->end[i] = result->end[i];
    }
    
    // Calculer moyennes
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

/* ═══════════════════════════════════════════════════════════
   MAIN: Point d'entrée Priorité (GUI ou Console)
   ═══════════════════════════════════════════════════════════ */
void priorite(Process procs[], int n) {
    // ★★★ UNE SEULE SIMULATION ★★★
    SimulationResult result = simulate_priority(procs, n);
    
    // Router selon le mode
    if (capture_mode && current_result) {
        // Mode GUI: copier vers current_result
        copy_to_gui_result(&result);
    } else {
        // Mode Console: afficher directement
        display_console_results(
            "Ordonnancement Priorité Préemptive",
            0,
            &result,
            NULL
        );
    }
}

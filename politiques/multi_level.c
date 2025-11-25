#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#define MAX_PRIO 6
#define FAMINE_THRESHOLD 6
#define FATIGUE_THRESHOLD 3

/*
 * RÈGLES DE L'ÉNONCÉ:
 * 1. Quantum: Prio 5,4 → 1 | Prio 3,2 → 2 | Prio 1,0 → 4
 * 2. Priorité -1 SEULEMENT après avoir terminé un quantum complet (min 0)
 * 3. Si préemption avant fin du quantum → priorité INCHANGÉE
 * 4. Fatigue CPU: >3 quanta consécutifs du MÊME processus → repos 1 unité
 * 5. Anti-famine: attente >6 unités consécutives → priorité +1 (max 5)
 * 6. Préemption: si processus de priorité supérieure arrive
 */

// File de priorité
typedef struct {
    int items[MAXP];
    int size;
} PrioQueue;

// État global du scheduler
typedef struct {
    PrioQueue queues[MAX_PRIO];
    int time;
    int *wait_time;      // Attente consécutive par processus
    int *first_run;      // Premier temps d'exécution
    int *end_time;       // Temps de fin
    int *timeline;       // Historique d'exécution
    int timeline_len;
    int running;         // Processus en cours (-1 si aucun)
    int quanta_count;    // Quanta consécutifs du MÊME processus
    int quantum_used;    // Unités utilisées dans le quantum actuel
    int quantum_size;    // Taille du quantum actuel
    int cpu_resting;     // CPU en repos (fatigue)
    int last_proc;       // Dernier processus ayant TERMINÉ un quantum
} State;

int get_quantum(int prio) {
    if (prio >= 4) return 1;  // Prio 5,4 → quantum 1
    if (prio >= 2) return 2;  // Prio 3,2 → quantum 2
    return 4;                  // Prio 1,0 → quantum 4
}

void q_init(PrioQueue *q) {
    q->size = 0;
}

void q_push(PrioQueue *q, int idx) {
    if (q->size < MAXP)
        q->items[q->size++] = idx;
}

int q_pop(PrioQueue *q) {
    if (q->size == 0) return -1;
    int val = q->items[0];
    for (int i = 0; i < q->size - 1; i++)
        q->items[i] = q->items[i + 1];
    q->size--;
    return val;
}

int q_contains(PrioQueue *q, int idx) {
    for (int i = 0; i < q->size; i++)
        if (q->items[i] == idx) return 1;
    return 0;
}

void q_remove(PrioQueue *q, int idx) {
    for (int i = 0; i < q->size; i++) {
        if (q->items[i] == idx) {
            for (int j = i; j < q->size - 1; j++)
                q->items[j] = q->items[j + 1];
            q->size--;
            return;
        }
    }
}

int in_any_queue(State *s, int idx) {
    for (int p = 0; p < MAX_PRIO; p++)
        if (q_contains(&s->queues[p], idx)) return 1;
    return 0;
}

int highest_prio_waiting(State *s) {
    for (int p = MAX_PRIO - 1; p >= 0; p--)
        if (s->queues[p].size > 0) return p;
    return -1;
}

int select_next(State *s) {
    for (int p = MAX_PRIO - 1; p >= 0; p--)
        if (s->queues[p].size > 0)
            return q_pop(&s->queues[p]);
    return -1;
}

void init_state(State *s, int n) {
    for (int i = 0; i < MAX_PRIO; i++) q_init(&s->queues[i]);
    s->time = 0;
    s->running = -1;
    s->last_proc = -1;
    s->quanta_count = 0;
    s->quantum_used = 0;
    s->quantum_size = 0;
    s->cpu_resting = 0;
    s->timeline_len = 0;
    
    s->wait_time = calloc(n, sizeof(int));
    s->first_run = malloc(n * sizeof(int));
    s->end_time = malloc(n * sizeof(int));
    s->timeline = malloc(1000 * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        s->first_run[i] = -1;
        s->end_time[i] = -1;
    }
}

void free_state(State *s) {
    free(s->wait_time);
    free(s->first_run);
    free(s->end_time);
    free(s->timeline);
}

void print_queues(State *s, Process procs[]) {
    printf("      [Files: ");
    int empty = 1;
    for (int p = MAX_PRIO - 1; p >= 0; p--) {
        if (s->queues[p].size > 0) {
            empty = 0;
            printf("Prio%d(", p);
            for (int i = 0; i < s->queues[p].size; i++) {
                printf("%s", procs[s->queues[p].items[i]].name);
                if (i < s->queues[p].size - 1) printf(",");
            }
            printf(") ");
        }
    }
    if (empty) printf("vides");
    printf("]\n");
}

void multi_level(Process procs[], int n) {
    State s;
    init_state(&s, n);
    
    // Sauvegarder priorités initiales
    int *init_prio = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
        init_prio[i] = procs[i].priority;
    
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════════════╗\n");
    printf("║     ORDONNANCEMENT MULTI-NIVEAUX À PRIORITÉS DYNAMIQUES               ║\n");
    printf("╠═══════════════════════════════════════════════════════════════════════╣\n");
    printf("║ RÈGLES:                                                               ║\n");
    printf("║  • Quantum: Priorité 5,4 → 1 | Priorité 3,2 → 2 | Priorité 1,0 → 4   ║\n");
    printf("║  • Priorité -1 UNIQUEMENT si quantum TERMINÉ (jamais < 0)            ║\n");
    printf("║  • Préemption si processus plus prioritaire arrive (prio inchangée)  ║\n");
    printf("║  • Fatigue CPU: >%d quanta consécutifs → repos 1 unité                ║\n", FATIGUE_THRESHOLD);
    printf("║  • Anti-famine: attente >%d unités → priorité +1 (max 5)              ║\n", FAMINE_THRESHOLD);
    printf("╚═══════════════════════════════════════════════════════════════════════╝\n\n");
    
    printf("PROCESSUS EN ENTRÉE:\n");
    printf("┌──────┬─────────┬───────┬──────────┐\n");
    printf("│ Nom  │ Arrivée │ Durée │ Priorité │\n");
    printf("├──────┼─────────┼───────┼──────────┤\n");
    for (int i = 0; i < n; i++)
        printf("│ %-4s │ %7d │ %5d │ %8d │\n", 
               procs[i].name, procs[i].arrival, procs[i].duration, procs[i].priority);
    printf("└──────┴─────────┴───────┴──────────┘\n\n");
    
    printf("Simulation en cours...\n\n");
    
    int done = 0;
    
    while (done < n && s.time < 500) {
        
        // ÉTAPE 1: CPU en repos (fatigue)?
        if (s.cpu_resting) {
            s.timeline[s.timeline_len++] = -1;
            s.cpu_resting = 0;
            for (int i = 0; i < n; i++)
                if (in_any_queue(&s, i))
                    s.wait_time[i]++;
            s.time++;
            continue;
        }
        
        // ÉTAPE 2: Arrivée de nouveaux processus
        for (int i = 0; i < n; i++) {
            if (procs[i].arrival == s.time && procs[i].remaining_time > 0) {
                if (!in_any_queue(&s, i) && s.running != i) {
                    q_push(&s.queues[procs[i].priority], i);
                }
            }
        }
        
        // ÉTAPE 3: Anti-famine (attente > 6 unités)
        for (int i = 0; i < n; i++) {
            if (s.wait_time[i] > FAMINE_THRESHOLD && procs[i].priority < 5) {
                for (int p = 0; p < MAX_PRIO; p++) {
                    if (q_contains(&s.queues[p], i)) {
                        q_remove(&s.queues[p], i);
                        procs[i].priority++;
                        q_push(&s.queues[procs[i].priority], i);
                        s.wait_time[i] = 0;
                        break;
                    }
                }
            }
        }
        
        // ÉTAPE 4: Vérifier PRÉEMPTION
        int best_waiting = highest_prio_waiting(&s);
        if (s.running != -1 && best_waiting > procs[s.running].priority) {
            if (s.quantum_used < s.quantum_size) {
                q_push(&s.queues[procs[s.running].priority], s.running);
                s.running = -1;
                s.quantum_used = 0;
                s.quantum_size = 0;
            }
        }
        
        // ÉTAPE 5: Vérifier fin de quantum
        if (s.running != -1 && s.quantum_used >= s.quantum_size && s.quantum_size > 0) {
            if (procs[s.running].priority > 0)
                procs[s.running].priority--;
            
            if (s.running == s.last_proc) {
                s.quanta_count++;
            } else {
                s.quanta_count = 1;
            }
            s.last_proc = s.running;
            
            if (s.quanta_count > FATIGUE_THRESHOLD) {
                s.cpu_resting = 1;
                s.quanta_count = 0;
                s.last_proc = -1;
            }
            
            q_push(&s.queues[procs[s.running].priority], s.running);
            s.running = -1;
            s.quantum_used = 0;
            s.quantum_size = 0;
        }
        
        // ÉTAPE 6: Sélectionner prochain processus
        if (s.running == -1 && !s.cpu_resting) {
            int next = select_next(&s);
            if (next != -1) {
                s.running = next;
                s.quantum_used = 0;
                s.quantum_size = get_quantum(procs[s.running].priority);
                if (s.first_run[s.running] == -1)
                    s.first_run[s.running] = s.time;
            }
        }
        
        // ÉTAPE 7: Exécuter 1 unité de temps
        if (s.running == -1) {
            s.timeline[s.timeline_len++] = -1;
        } else {
            s.timeline[s.timeline_len++] = s.running;
            procs[s.running].remaining_time--;
            s.quantum_used++;
            s.wait_time[s.running] = 0;
            
            if (procs[s.running].remaining_time == 0) {
                s.end_time[s.running] = s.time + 1;
                done++;
                s.last_proc = -1;
                s.quanta_count = 0;
                s.running = -1;
                s.quantum_used = 0;
                s.quantum_size = 0;
            }
        }
        
        // Augmenter attente des processus en file
        for (int i = 0; i < n; i++) {
            if (s.running != i && in_any_queue(&s, i))
                s.wait_time[i]++;
        }
        
        s.time++;
    }
    
    // ════════════════════════════════════════════════════════════════
    // AFFICHAGE DES RÉSULTATS
    // ════════════════════════════════════════════════════════════════
    
    printf("\n════════════════════════ RÉSULTATS ════════════════════════\n\n");
    
    // Chronologie
    printf("CHRONOLOGIE D'EXÉCUTION:\n");
    printf("─────────────────────────\n");
    for (int t = 0; t < s.timeline_len; t++) {
        if (s.timeline[t] == -1)
            printf("[IDLE:%d→%d] ", t, t+1);
        else
            printf("[%s:%d→%d] ", procs[s.timeline[t]].name, t, t+1);
        if ((t + 1) % 8 == 0) printf("\n");
    }
    printf("\n\n");
    
    // Diagramme de Gantt
    printf("DIAGRAMME DE GANTT:\n");
    printf("───────────────────\n");
    printf("Time ");
    for (int t = 0; t <= s.timeline_len && t <= 50; t++) printf("%2d ", t);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("%-4s ", procs[i].name);
        for (int t = 0; t < s.timeline_len && t <= 50; t++)
            printf("%s", s.timeline[t] == i ? "## " : "   ");
        printf("\n");
    }
    
    // Statistiques
    printf("\nSTATISTIQUES DES PROCESSUS:\n");
    printf("┌──────┬─────────┬───────┬──────────┬───────┬─────┬────────────┬─────────┐\n");
    printf("│ Proc │ Arrivée │ Durée │ Prio_ini │ Début │ Fin │ Turnaround │ Attente │\n");
    printf("├──────┼─────────┼───────┼──────────┼───────┼─────┼────────────┼─────────┤\n");
    
    float sum_turn = 0, sum_wait = 0;
    for (int i = 0; i < n; i++) {
        int turn = s.end_time[i] - procs[i].arrival;
        int wait = turn - procs[i].duration;
        sum_turn += turn;
        sum_wait += wait;
        printf("│ %-4s │ %7d │ %5d │ %8d │ %5d │ %3d │ %10d │ %7d │\n",
               procs[i].name, procs[i].arrival, procs[i].duration,
               init_prio[i], s.first_run[i], s.end_time[i], turn, wait);
    }
    
    printf("└──────┴─────────┴───────┴──────────┴───────┴─────┴────────────┴─────────┘\n");
    printf("\n");
    printf("Temps de rotation moyen: %.2f unités\n", sum_turn / n);
    printf("Temps d'attente moyen:   %.2f unités\n", sum_wait / n);
    printf("Temps total simulation:  %d unités\n", s.time);
    
    free_state(&s);
    free(init_prio);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "menu.h"

// Déclarations des fonctions de scheduling
void scheduler_fifo(Process procs[], int count);
void scheduler_rr(Process procs[], int count);
void scheduler_priority(Process procs[], int count);
void scheduler_multi_level(Process procs[], int count);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s fichier_processus.txt\n", argv[0]);
        return 1;
    }

    Process procs[MAXP];
    int count = 0;

    printf("Tentative d'ouverture du fichier: %s\n", argv[1]);
    
    // Lecture des processus
    if (read_processes_from_file(argv[1], procs, &count) != 0) {
        printf("Erreur de lecture du fichier. Vérifiez le nom et les permissions.\n");
        return 1;
    }

    if (count == 0) {
        printf("Aucun processus trouvé dans le fichier.\n");
        return 1;
    }

    printf("\n=== Processus chargés : %d ===\n", count);
    for (int i = 0; i < count; i++) {
        printf("%s arrive à %d | durée %d | priorité %d\n",
               procs[i].name, procs[i].arrival, 
               procs[i].duration, procs[i].priority);
    }

    // Menu des politiques
    afficher_policies();
    int choix = choisir_politique();
    
    // Exécution de la politique choisie
    executer_politique(choix, procs, count);

    return 0;
}

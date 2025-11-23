#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "menu.h"
#include "process.h"

// Déclarations des fonctions de scheduling
void scheduler_fifo(Process procs[], int count);
void scheduler_rr(Process procs[], int count, int quantum);
void scheduler_priority(Process procs[], int count);
void scheduler_multi_level(Process procs[], int count);

void afficher_policies() {
    printf("\n=== Politiques disponibles ===\n");
    printf("1. FIFO (First-In-First-Out)\n");
    printf("2. Round Robin\n");
    printf("3. Priorité Préemptive\n");
    printf("4. Multi-Level avec Fatigue CPU\n");
}

int choisir_politique() {
    int choix;
    char input[100];
    
    printf("\nChoisissez une politique (1-4) [1=FIFO par défaut]: ");
    
    // Lire l'entrée comme chaîne de caractères
    if (fgets(input, sizeof(input), stdin) != NULL) {
        // Convertir en entier
        if (sscanf(input, "%d", &choix) == 1) {
            if (choix >= 1 && choix <= 4) {
                return choix;
            }
        }
    }
    
    // Si entrée invalide, retourner une valeur par défaut
    printf("Choix invalide! Utilisation de FIFO par défaut.\n");
    return 1;
}

// Fonction pour exécuter la politique choisie
void executer_politique(int choix, Process procs[], int count) {

    switch (choix) {
        case 1:
            scheduler_fifo(procs, count);
            break;

        case 2: {
            int q;
            printf("Vous avez choisi Round Robin.\n");
            printf("➡ Entrez le quantum : ");
            scanf("%d", &q);

            if (q <= 0) q = 1;

            scheduler_rr(procs, count, q);
            break;
        }

        case 3:
            scheduler_priority(procs, count);
            break;

        case 4:
            scheduler_multi_level(procs, count);
            break;

        default:
            printf("Choix invalide.\n");
    }

}

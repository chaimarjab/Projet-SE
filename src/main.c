#include <stdio.h>
#include <string.h>
#include "process.h"
#include "menu.h"
#include "gui.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s fichier_processus.txt\n", argv[0]);
        return 1;
    }

    Process procs[MAXP];
    int count = 0;
    read_processes_from_file(argv[1], procs, &count);

    // Demander à l'utilisateur son choix d'interface
    printf("\n╔═══════════════════════════════════════════════════╗\n");
    printf("║     ORDONNANCEUR DE PROCESSUS - MODE D'AFFICHAGE  ║\n");
    printf("╚═══════════════════════════════════════════════════╝\n\n");
    printf("Choisissez le mode d'affichage :\n");
    printf("  1. Interface console (terminal)\n");
    printf("  2. Interface graphique (GTK)\n\n");
    printf("Votre choix : ");

    char choix[10];
    fgets(choix, sizeof(choix), stdin);
    
    int mode = 0;
    sscanf(choix, "%d", &mode);

    if (mode == 2) {
        // Mode interface graphique
        printf("\n🖥️  Lancement de l'interface graphique...\n");
        lancer_interface_gtk(procs, count);
    } else {
        // Mode console (par défaut)
        printf("\n💻 Mode console sélectionné\n");
        afficher_policies();
        int index = choisir_politique();
        executer_politique(index, procs, count);
    }

    return 0;
}

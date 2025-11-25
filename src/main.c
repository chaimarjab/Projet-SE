#include <stdio.h>
#include "process.h"
#include "menu.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s fichier_processus.txt\n", argv[0]);
        return 1;
    }

    Process procs[MAXP];
    int count = 0;

    read_processes_from_file(argv[1], procs, &count);

    afficher_policies();
    int index = choisir_politique();

    executer_politique(index, procs, count);

    return 0;
}

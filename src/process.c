#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

int read_processes_from_file(const char *filename, Process procs[], int *count) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Erreur : impossible d'ouvrir le fichier %s.\n", filename);
        return -1;
    }

    *count = 0;
    char line[200];
    int line_num = 0;

    printf("=== Début de la lecture du fichier ===\n");

    while (fgets(line, sizeof(line), f) != NULL && *count < MAXP) {
        line_num++;

        // Nettoyage de la ligne
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' || line[len-1] == ' ')) {
            line[len-1] = '\0';
            len--;
        }

        // Ignorer les lignes vides et commentaires
        if (strlen(line) == 0 || line[0] == '#')
            continue;

        // Extraction des données
        char temp_name[100];
        int temp_arrival, temp_duration, temp_priority;
        
        int r = sscanf(line, "%99s %d %d %d",
                      temp_name, &temp_arrival, &temp_duration, &temp_priority);

        if (r == 4) {
            strcpy(procs[*count].name, temp_name);
            procs[*count].arrival = temp_arrival;
            procs[*count].duration = temp_duration;
            procs[*count].priority = temp_priority;
            procs[*count].remaining_time = temp_duration;
            (*count)++;
        } else if (r > 0) {
            printf("Ligne %d: format invalide (attendu: nom arrivee duree priorite)\n", line_num);
        }
    }

    fclose(f);
    return 0;
}

#ifndef MENU_H
#define MENU_H

#include "process.h"

// Charger les politiques disponibles
void charger_politiques(void);

// Afficher les politiques disponibles
void afficher_policies(void);

// Choisir une politique
int choisir_politique(void);

// Exécuter la politique choisie
void executer_politique(int index, Process procs[], int count);

#endif

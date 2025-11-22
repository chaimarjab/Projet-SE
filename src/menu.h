#ifndef MENU_H
#define MENU_H

// Inclure process.h pour connaître le type Process
#include "process.h"

// Fonctions du menu
void afficher_policies();
int choisir_politique();
void executer_politique(int choix, Process procs[], int count);

#endif

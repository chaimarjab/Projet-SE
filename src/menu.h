#ifndef MENU_H
#define MENU_H

#include "process.h"

void afficher_policies();
int choisir_politique();
void executer_politique(int index, Process procs[], int count);

#endif

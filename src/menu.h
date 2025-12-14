#ifndef MENU_H
#define MENU_H

#include "process.h"


void charger_politiques(void);


void afficher_policies(void);


int choisir_politique(void);


void executer_politique(int index, Process procs[], int count);

#endif

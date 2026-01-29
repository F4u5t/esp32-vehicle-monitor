#ifndef CONSOLE_MENU_H
#define CONSOLE_MENU_H

#include <Arduino.h>

void initConsole();
void handleConsole();
bool isConsoleActive(); // Check if menu is blocking logs

#endif

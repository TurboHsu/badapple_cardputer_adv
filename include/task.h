#pragma once

void startPlayTask();
void startMenuTask();

void TaskMenu( void *pvParameters );
void TaskPlayVideo( void *pvParameters );
void TaskPlayAudio( void *pvParameters );
void TaskPlayWatchdog( void *pvParameters );

#ifndef INICIARGAMECARD_H_
#define INICIARGAMECARD_H_

#include "common_utils.h"
#include "fileSystemTallGrass.h"

uint32_t socket_broker;
on_request request;
char* PUERTO_BROKER;
char* IP_BROKER;

uint32_t id_connection;

void iniciarGameCard();
void suscribirseATodo();
void finalizarGameCard();
void connect_client();
#endif
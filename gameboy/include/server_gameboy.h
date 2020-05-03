#ifndef SERVER_GAMEBOY_H
#define SERVER_GAMEBOY_H

#include "common_utils.h"

const static struct{
    op_code operation;
    char* name; 
}message_string [] = {
    {NEW_POKEMON, "NEW_POKEMON"},
    {APPEARED_POKEMON, "APPEARED_POKEMON"},
    {CATCH_POKEMON, "CATCH_POKEMON"},
    {CAUGHT_POKEMON, "CAUGHT_POKEMON"},
    {GET_POKEMON, "GET_POKEMON"},
    {SUSCRIPTOR, "SUSCRIPTOR"},
    {LOCALIZED_POKEMON, "LOCALIZED_POKEMON"},
    {ERROR, "ERROR"}
};

void send_message(char** message, int socket_cliente,t_log*  optional_logger);
op_code stringToEnum(char* message);
void send_new_connection(uint32_t socket_broker);
void send_reconnect(uint32_t socket_broker);
uint32_t receive_connection_id(uint32_t socket_broker);

#endif
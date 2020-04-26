#ifndef COMMON_CONNECTIONS_H
#define COMMON_CONNECTIONS_H

#include "common_utils.h"

typedef void (*on_request)(uint32_t codop, uint32_t sizeofstruct, uint32_t socketfd);

typedef struct
{
	uint32_t socket;
	on_request request_receiver;
} t_process_request;

typedef enum
{
	NEW_POKEMON = 1,
	APPEARED_POKEMON = 2,
	CATCH_POKEMON = 3,
	CAUGHT_POKEMON = 4,
	GET_POKEMON = 5,
	SUSCRIPTOR = 6,
	ERROR = 7,
	MENSAJE = 8
}op_code;

typedef struct
{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void start_server(char* ip, char* port, on_request request_receiver);
void run_server(void* processor);
void receive_new_connections(uint32_t socket_escucha, on_request request_receiver);
void serve_client(t_process_request* processor);
void* serializar_paquete(t_paquete* paquete, uint32_t bytes);
void devolver_mensaje(void* payload, uint32_t size, uint32_t socket_cliente);
uint32_t crear_conexion(char *ip, char* puerto);
void enviar_mensaje(char* mensaje, uint32_t socketfd);
char* recibir_mensaje(uint32_t socket_cliente, uint32_t buffer_size);
void process_message(uint32_t client_fd, uint32_t size);
void pthread_create_and_detach(void* function, void* args);
void receiveMessageSubscriptor(uint32_t cod_op, uint32_t sizeofstruct, uint32_t socketfd);
#endif
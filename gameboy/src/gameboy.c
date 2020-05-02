#include "gameboy.h"

void closeAll(t_log* optional_logger,t_log* obligatory_logger, t_config* config, int connection){
    
    //Destruyo todo: logger opcional, logger obligatorio, config y conexion.
    log_destroy(optional_logger);
    log_destroy(obligatory_logger);
    config_destroy(config);
    close(connection);
}

void countTime(void* timePassed){ 
    sleep((uint32_t) timePassed);
    exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv){

    if(strcmp(argv[1],"-test")==0){
        run_tests_gameboy();
    }else{
    //Inicializando la config
    config = config_create("./cfg/gameboy.config");

    int showConsole = config_get_int_value(config,"LOG_SHOW");

    //Inicializando el logger
    obligatory_logger = log_create("./cfg/obligatory.log", "obligatory", showConsole, LOG_LEVEL_INFO); 
    optional_logger = log_create("./cfg/optional.log", "optional", showConsole, LOG_LEVEL_INFO);

    char* ip; 
    char* port;

    char* server = argv[1];
    //Busco el ip y el puerto
    if(strcmp(server,"BROKER") == 0 || strcmp(server, "SUSCRIPTOR") == 0){
        ip = config_get_string_value(config, "IP_BROKER");
        port = config_get_string_value(config, "PUERTO_BROKER");
    }else if(strcmp(server,"TEAM") == 0){
        ip = config_get_string_value(config, "IP_TEAM");
        port = config_get_string_value(config, "PUERTO_TEAM");
    }else if(strcmp(server,"GAMECARD") == 0){
        ip = config_get_string_value(config, "IP_GAMECARD");
        port = config_get_string_value(config, "PUERTO_GAMECARD");
    }else{
        log_error(optional_logger,"No se pudo conectar");
        exit(4);
    }

    //Creo la conexion
    int connection = crear_conexion(ip, port);

    //Mando el mensaje
    send_message(argv, connection, optional_logger);

    //Me suscribo si el mensaje es SUSCRIPTOR
    if(strcmp(server, "SUSCRIPTOR") == 0){
       
        t_process_request* process_request = malloc(sizeof(t_process_request)); 
        (*process_request).socket = connection; 
        (*process_request).request_receiver = receiveMessageSubscriptor;
    
        pthread_t threadConnection; //Creo un hilo asi cuenta el tiempo de conexion
        pthread_create(&threadConnection, NULL, (void*) countTime, (void*) atoi(argv[3]));

        send_new_connection(connection);
        uint32_t cod_op = -1;   
        void* stream = malloc(sizeof(uint32_t)); 

        while(1){
            while(recv(stream, (void*) &cod_op,sizeof(uint32_t), MSG_WAITALL) == 0){
                send_reconnect(connection);
                sleep(2);
            }
            serve_client(process_request);
        }     
    }

    //Cierro y elimino todo
    closeAll(optional_logger, obligatory_logger,config,connection);
    
    return 0;
    }
}
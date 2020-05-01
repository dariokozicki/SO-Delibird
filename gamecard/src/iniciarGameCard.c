#include "iniciarGameCard.h"

void iniciarGameCard(){

    config = config_create("./cfg/gamecard.config");
    uint32_t showConsole = config_get_int_value(config,"LOG_SHOW");
    obligatory_logger = log_create("./cfg/obligatory.log", "obligatory", showConsole, LOG_LEVEL_INFO); 
    optional_logger = log_create("./cfg/optional.log", "optional", showConsole, LOG_LEVEL_INFO);
    //char* IP_GAMECARD = config_get_string_value(config,"IP_GAMECARD");
    //char* PUERTO_GAMECARD = config_get_string_value(config,"PUERTO_GAMECARD");

    //Se intentara suscribir globalmente al Broker a las siguientes colas de mensajes
    //TODO
    
    pthread_t hiloClienteBroker;
    pthread_create(&hiloClienteBroker,NULL,(void*)suscribirseATodo,NULL);
    pthread_join(hiloClienteBroker,NULL);
    //receiveMessageSubscriptor();
    //start_server(ipGameCard, puertoGameCard);
    finalizarGameCard();
}


void suscribirseATodo(){
    char* PUERTO_BROKER = config_get_string_value(config,"PUERTO_BROKER");
    char* IP_BROKER = config_get_string_value(config,"IP_BROKER");
    socket_broker = crear_conexion(IP_BROKER,PUERTO_BROKER);
    suscribirseA(NEW_POKEMON,socket_broker);
    suscribirseA(CATCH_POKEMON,socket_broker);
    suscribirseA(GET_POKEMON,socket_broker);
}

void finalizarGameCard(){
    log_destroy(obligatory_logger);
    log_destroy(optional_logger);
    config_destroy(config);
    close(socket_broker);
}
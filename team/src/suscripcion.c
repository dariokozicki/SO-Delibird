#include "suscripcion.h"
#include <string.h>
#include "team.h"

void* connection_broker_global_suscribe() {
    connection_broker_suscribe_to_appeared_pokemon(APPEARED_POKEMON);
    connection_broker_suscribe_to_caught_pokemon(CAUGHT_POKEMON);
    connection_broker_suscribe_to_localized_pokemon(LOCALIZED_POKEMON);
    
    pthread_join(suscripcionAppearedPokemon,NULL);
    pthread_join(suscripcionCaughtPokemon,NULL);
    pthread_join(suscripcionLocalizedPokemon,NULL);

    return NULL;
 }

void connection_broker_suscribe_to_appeared_pokemon(op_code code) {   
    //se envia un connect por cada cola de mensajes a suscribirse
    uint32_t server_connection_appeared_pokemon = crear_conexion(config_values.ip_broker, config_values.puerto_broker);
    send_new_connection(server_connection_appeared_pokemon); 
    
    log_info(obligatory_logger, "Connection to broker succesfully\n"); 
    args_pthread *arguments = thread_suscribe_arguments(code, server_connection_appeared_pokemon);    
    pthread_create(&suscripcionAppearedPokemon, NULL, (void*)suscribeOnThreadList, arguments);
}

void connection_broker_suscribe_to_caught_pokemon(op_code code) {	
    uint32_t server_connection_caught_pokemon = crear_conexion(config_values.ip_broker, config_values.puerto_broker);
    send_new_connection(server_connection_caught_pokemon); 

    log_info(obligatory_logger, "Connection to broker succesfully\n");
    args_pthread *arguments = thread_suscribe_arguments(code, server_connection_caught_pokemon);   
    pthread_create(&suscripcionCaughtPokemon, NULL, (void*)suscribeOnThreadList, arguments);
}

void connection_broker_suscribe_to_localized_pokemon(op_code code) {
    uint32_t server_connection_localized_pokemon = crear_conexion(config_values.ip_broker, config_values.puerto_broker);   
    send_new_connection(server_connection_localized_pokemon); 

    log_info(obligatory_logger, "Connection to broker succesfully\n");	
    args_pthread *arguments = thread_suscribe_arguments(code, server_connection_localized_pokemon);   
    pthread_create(&suscripcionLocalizedPokemon, NULL, (void*)suscribeOnThreadList, arguments); 	
}

args_pthread* thread_suscribe_arguments(op_code code, uint32_t socket) {
    args_pthread* arguments = malloc(sizeof(args_pthread)); 
    arguments->codigoCola = code;
    arguments->socket = malloc(sizeof(uint32_t));
    *arguments->socket = socket;
    return arguments;
}

void suscribeOnThreadList(args_pthread* arguments){
    suscribirseA(arguments->codigoCola,*arguments->socket);
    
    switch(arguments->codigoCola){
        case APPEARED_POKEMON:
            structAppearedPokemon = malloc(sizeof(threadSubscribe));
            structAppearedPokemon->idQueue = APPEARED_POKEMON;
            structAppearedPokemon->socket = *arguments->socket;
            pthread_mutex_lock(&threadSubscribeList_mutex);
            list_add(threadSubscribeList, structAppearedPokemon);
            pthread_mutex_unlock(&threadSubscribeList_mutex);
            log_info(optional_logger, "Queue subscription request APPEARED_POKEMON successfully.\n");
            connect_client(*arguments->socket,APPEARED_POKEMON);
            break;
        case CAUGHT_POKEMON:
            structCaughtPokemon = malloc(sizeof(threadSubscribe));
            structCaughtPokemon->idQueue = CAUGHT_POKEMON;
            structCaughtPokemon->socket = *arguments->socket;
            pthread_mutex_lock(&threadSubscribeList_mutex);
            list_add(threadSubscribeList, structCaughtPokemon);
            pthread_mutex_unlock(&threadSubscribeList_mutex);
            log_info(optional_logger, "Queue subscription request CAUGHT_POKEMON successfully.\n");
            connect_client(*arguments->socket,CAUGHT_POKEMON);
            break;
        case LOCALIZED_POKEMON:
            structLocalizedPokemon = malloc(sizeof(threadSubscribe));
            structLocalizedPokemon->idQueue = LOCALIZED_POKEMON;
            structLocalizedPokemon->socket = *arguments->socket;
            pthread_mutex_lock(&threadSubscribeList_mutex);
            list_add(threadSubscribeList, structLocalizedPokemon);
            pthread_mutex_unlock(&threadSubscribeList_mutex);
            log_info(optional_logger, "Queue subscription request LOCALIZED_POKEMON successfully.\n");
            connect_client(*arguments->socket,LOCALIZED_POKEMON);
            break;
        default:
            break;
    }
}

void connect_client(uint32_t socket,op_code codeOperation){
    t_process_request* process_request = malloc(sizeof(t_process_request)); 
    (*process_request).socket = malloc(sizeof(uint32_t));
    *(*process_request).socket = socket; 
    (*process_request).request_receiver = request;

    uint32_t id_connection = receive_connection_id(socket);

    switch(codeOperation){
        case APPEARED_POKEMON:
            structAppearedPokemon->idConnection = id_connection;
            break;
        case CAUGHT_POKEMON:
            structCaughtPokemon->idConnection = id_connection;
            break;
        case LOCALIZED_POKEMON:
            structLocalizedPokemon->idConnection = id_connection;
            break;
        default:
            break;
    }
    
    while(1){
        serve_client(process_request);
        close(socket);
        socket = crear_conexion(config_values.ip_broker, config_values.puerto_broker);
        process_request = malloc(sizeof(t_process_request)); 
        (*process_request).socket = malloc(sizeof(uint32_t));
        *(*process_request).socket = socket; 
        (*process_request).request_receiver = request;
        send_reconnect(socket, id_connection);
    } 

    pthread_join(server, NULL);
}

void listen_to_gameboy() { 
   start_server(config_values.ip_team, config_values.puerto_team, request);
}

void reception_message_queue_subscription(uint32_t code, uint32_t sizeofstruct, uint32_t client_fd) {
	void* stream = malloc(sizeofstruct);
    uint32_t* id_message = malloc(sizeof(uint32_t));
    uint32_t* id_message_correlational = malloc(sizeof(uint32_t));
    recv(client_fd, stream, sizeofstruct, MSG_WAITALL);

    switch(code){
        case APPEARED_POKEMON:;
            appeared_pokemon* appeared_pokemon_Message = stream_to_appeared_pokemon(stream, id_message, NULL, false); 
            appeared_pokemon_Message->pokemon = realloc(appeared_pokemon_Message->pokemon, appeared_pokemon_Message->sizePokemon+1);
            appeared_pokemon_Message->pokemon[appeared_pokemon_Message->sizePokemon] = '\0';
            log_info(obligatory_logger, "Receiving Message Appeared pokemon.");
            log_info(obligatory_logger, "Pokemon Appeared: %s, position: (%d,%d)", appeared_pokemon_Message->pokemon, appeared_pokemon_Message->position.posx, appeared_pokemon_Message->position.posy);
            send_ack(client_fd, *id_message);

            pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
            pokemonCompareGlobalObjetive = malloc(strlen(appeared_pokemon_Message->pokemon)+1);
            strcpy(pokemonCompareGlobalObjetive, appeared_pokemon_Message->pokemon);
            bool anyPokemonInGlobalObjetive = list_any_satisfy(globalObjetive, analyzePokemonInGlobal);
            free(pokemonCompareGlobalObjetive);
            pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
            
            if(anyPokemonInGlobalObjetive){
                t_pokemon_on_map* newPokemonAppeared = malloc(sizeof(t_pokemon_on_map));
                newPokemonAppeared->state = P_FREE;
                newPokemonAppeared->position.posx = appeared_pokemon_Message->position.posx;
                newPokemonAppeared->position.posy = appeared_pokemon_Message->position.posy;
                newPokemonAppeared->pokemon = malloc(strlen(appeared_pokemon_Message->pokemon)+1);
                pthread_mutex_lock(&pokemonsOnMap_mutex);
                newPokemonAppeared->id = list_size(pokemonsOnMap) + 1;
                strcpy(newPokemonAppeared->pokemon, appeared_pokemon_Message->pokemon);
                list_add(pokemonsOnMap, newPokemonAppeared);
                pthread_mutex_unlock(&pokemonsOnMap_mutex);

                sem_post(&plannerSemaphore);
            }
            free(appeared_pokemon_Message->pokemon);
            free_appeared_pokemon(appeared_pokemon_Message);
            break;
        case CAUGHT_POKEMON:;
			caught_pokemon* caught_Pokemon_Message = stream_to_caught_pokemon(stream, id_message, id_message_correlational, false);
            log_info(obligatory_logger, "Receiving Message Caught pokemon, Result %d", caught_Pokemon_Message->success);
            processCaughtPokemon(*id_message_correlational, caught_Pokemon_Message->success);
            
            send_ack(client_fd, *id_message);
            free_caught_pokemon(caught_Pokemon_Message);
            break;
        case LOCALIZED_POKEMON:;
            localized_pokemon* localized_Pokemon_Message = stream_to_localized_pokemon(stream, id_message, id_message_correlational, false);
            localized_Pokemon_Message->pokemon[localized_Pokemon_Message->sizePokemon] = '\0';
            int indexOfPokemonToLocalyze = getIndexPokemonToLocalizedByMessage(*id_message);
            if(indexOfPokemonToLocalyze != -1){
                //list_remove(pokemonsToLocalize, indexOfPokemonToLocalyze);
                list_remove_and_destroy_element(pokemonsToLocalize, indexOfPokemonToLocalyze, (void*)destroy_pokemonsToLocalize);
                log_info(obligatory_logger, "Receiving Message Localized pokemon");
                log_info(obligatory_logger, "Pokemon %s", localized_Pokemon_Message->pokemon);
                
                for(int i = 0; i<list_size(localized_Pokemon_Message->positions); i++) {
                    t_position* positionAux = list_get(localized_Pokemon_Message->positions, i);
                    log_info(obligatory_logger,"Find in position: (%d,%d)", positionAux->posx, positionAux->posy);
                }
                send_ack(client_fd, *id_message);
            }
            free_localized_pokemon(localized_Pokemon_Message);
            break;
         case CONNECTION:;
            connection* connectionMessage = stream_to_connection(stream);
            pthread_mutex_lock(&threadSubscribeList_mutex);
            threadSubscribe* thread = list_find_with_args(threadSubscribeList, compareSockets, (void*)client_fd);
            pthread_mutex_unlock(&threadSubscribeList_mutex);
            thread->idConnection = connectionMessage->id_connection;

            suscribirseA(thread->idQueue, client_fd);

            log_info(optional_logger, "Established Connection."); 
            log_info(optional_logger, "This is the id connection: %d", connectionMessage->id_connection);
            log_info(optional_logger, "Subscribing to queues %d, %d and & %d", APPEARED_POKEMON, CAUGHT_POKEMON, LOCALIZED_POKEMON);
            break;
    }
    free(stream);
    free(id_message);
    free(id_message_correlational);
}
bool compareSockets(void* element, void* args){
    threadSubscribe* thread = (threadSubscribe*) element;
    
    return thread->socket == (uint32_t) args;
}

void* send_get_pokemon_global_team(){
    uint32_t client_fd = crear_conexion(config_values.ip_broker, config_values.puerto_broker);
    uint32_t* id_message = malloc(sizeof(uint32_t));
    get_pokemon* getPokemonMessage;
    void* stream;
    char* pokemonToSend;
    pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
    int globalObjetiveCount = list_size(globalObjetive);
    pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);

    for(int i=0; i< globalObjetiveCount; i++) {
        pthread_mutex_lock(&pokemonCompareGlobalObjetive_mutex);
        pokemonToSend = (char*)list_get(globalObjetive, i);
        getPokemonMessage = malloc(sizeof(get_pokemon));
        getPokemonMessage->pokemon = malloc(strlen(pokemonToSend)+1);
        strcpy(getPokemonMessage->pokemon, pokemonToSend);
        getPokemonMessage->sizePokemon = strlen(getPokemonMessage->pokemon)+1;
        pthread_mutex_unlock(&pokemonCompareGlobalObjetive_mutex);
        *id_message = -1;
        stream =get_pokemon_to_stream(getPokemonMessage, id_message);

        t_paquete* packageToSend = stream_to_package(GET_POKEMON, stream, size_of_get_pokemon(getPokemonMessage));
        int bytes = packageToSend->buffer->size + 2*sizeof(uint32_t);
        void* buffer = (void *) serializar_paquete(packageToSend, bytes);

        send(client_fd, buffer, bytes, 0);
        free(buffer);
        free(getPokemonMessage->pokemon);
        free(getPokemonMessage);
        free_package(packageToSend);
        log_info(optional_logger, "Pokemon %s: ", pokemonToSend);

        uint32_t sizeOfBuffer = sizeof(uint32_t) * 3;
        buffer = malloc(sizeOfBuffer);
        recv(client_fd, buffer, sizeOfBuffer, MSG_WAITALL);
        ack* acknowledgementMessage = stream_to_ack(buffer+8);
        log_info(optional_logger, "Id mensaje get: %d", acknowledgementMessage->id_message);

        addPokemonToLocalize(pokemonToSend, acknowledgementMessage->id_message);
        free(buffer);
        free(acknowledgementMessage);
    }
    free(id_message);
    return NULL;
}

void addPokemonToLocalize(char* pokemon, uint32_t idMessage){
    t_pokemonToLocalized* pokemonToLocalizeAux;
    bool pokemonExists = false;

    for(int i=0; i<list_size(pokemonsToLocalize) && !pokemonExists;i++){
        pokemonToLocalizeAux = (t_pokemonToLocalized*)list_get(pokemonsToLocalize, i);
        if(strcmp(pokemon, pokemonToLocalizeAux->pokemon) == 0){
            pokemonExists = true;
        }
    }

    if(!pokemonExists){
        pokemonToLocalizeAux = malloc(sizeof(t_pokemonToLocalized));
        pokemonToLocalizeAux->idMessage = idMessage;
        pokemonToLocalizeAux->pokemon = malloc(strlen(pokemon)+1);
        strcpy(pokemonToLocalizeAux->pokemon, pokemon);
        list_add(pokemonsToLocalize, pokemonToLocalizeAux);
    }
}

int getIndexPokemonToLocalizedByMessage(uint32_t id_message){
    int result = -1;
    t_pokemonToLocalized* pokemonToLocalizeAux;

    for(int i=0; i<list_size(pokemonsToLocalize) && result == -1; i++){
        pokemonToLocalizeAux = (t_pokemonToLocalized*)list_get(pokemonsToLocalize, i);
        if(pokemonToLocalizeAux->idMessage == id_message){
            result = i;
        }
    }

    return result;
}

void processCaughtPokemon(uint32_t id_message, uint32_t success){
    t_threadTrainer* threadTrainerAux;
    pthread_mutex_lock(&threadsTrainers_mutex);
    int threadsTrainersCount = list_size(threadsTrainers);
    pthread_mutex_unlock(&threadsTrainers_mutex);
    for(int i=0; i<threadsTrainersCount; i++){
        pthread_mutex_lock(&threadsTrainers_mutex);
        threadTrainerAux = (t_threadTrainer*)list_get(threadsTrainers, i);
        pthread_mutex_unlock(&threadsTrainers_mutex);
        if(threadTrainerAux->idMessageCatch == id_message){
            if(success == 1){
                catch_succesfull(threadTrainerAux);
                return;
            }else{
                pthread_mutex_lock(&threadsTrainers_mutex);
                threadTrainerAux->state = BLOCKED;
                removePokemonOnMap(threadTrainerAux->positionTo);
                sem_post(&plannerSemaphore);
                pthread_mutex_unlock(&threadsTrainers_mutex);
            }
        }
    }
}
#include "memory.h"

void initializeMemory(){
    memory.partitions = list_create();
    memory.configuration.memoryAlgorithm = cfg_values.algoritmo_memoria;
    memory.configuration.replaceAlgorithm = cfg_values.algoritmo_reemplazo;
    memory.configuration.freePartitionAlgorithm = cfg_values.algoritmo_particion_libre;
    memory.configuration.size = cfg_values.tamano_memoria;
    memory.configuration.minimunPartitionSize = cfg_values.tamano_minimo_particion;
    memory.configuration.countFailedSearchForCompact = cfg_values.frecuencia_compactacion;
    memory.failedSearchCount = 0;
    memory.data = malloc(memory.configuration.size);
    memory.m_partitions_modify = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(memory.m_partitions_modify, NULL);
    memory.m_failed_search_modify = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(memory.m_failed_search_modify, NULL );
    t_data* data = malloc(sizeof(t_data));
    data->size = memory.configuration.size;
    data->partition_size = data->size;
    data->offset = 0;
    data->state = FREE;

    list_add(memory.partitions, data);
}

void* mallocMemory(uint32_t idMensaje, uint32_t size){
    t_data* freePartition = seekPartitionAvailable(size);
    freePartition->id = idMensaje;
    allocateData(size, freePartition);
    return memory.data + freePartition->offset;
}
void setIdQueue(uint32_t idQueue, uint32_t idMensaje){
    pthread_mutex_lock(memory.m_partitions_modify);
    t_data* partition = (t_data*)list_find_with_args(memory.partitions, partition_match_id_mensaje,(void*)idMensaje);
    pthread_mutex_unlock(memory.m_partitions_modify);
    if(partition == NULL) return;
    partition->idQueue = idQueue;
}
void* getData(uint32_t idMensaje){
    pthread_mutex_lock(memory.m_partitions_modify);
    t_data* partition = (t_data*)list_find_with_args(memory.partitions, partition_match_id_mensaje,(void*)idMensaje);
    pthread_mutex_unlock(memory.m_partitions_modify);
    if(partition == NULL) return NULL;
    //partition->lastTimeUsed = time(NULL);
    return memory.data + partition->offset;
}
bool partition_match_id_mensaje(void* data, void* idMensaje){
    return data ? ((t_data*) data)->id == (uint32_t) idMensaje && ((t_data*) data)->state == USING : false;
}

t_data* seekPartitionAvailable(uint32_t sizeData){
    uint32_t partition_size = sizeData > memory.configuration.minimunPartitionSize ? 
                sizeData : memory.configuration.minimunPartitionSize;
    t_data* freePartition = getPartitionAvailable(partition_size);

    if(freePartition == NULL){
        if(verifMustCompact()){
            compact();
        }else{
            destroyPartition();
        }
        return seekPartitionAvailable(partition_size);
    }else{
        return freePartition;
    }
}

t_data* getPartitionAvailable(uint32_t sizeData){
    if(strcmp(memory.configuration.freePartitionAlgorithm, "FF") == 0){
        return FF_getPartitionAvailable(sizeData);
    }else{
        return BF_getPartitionAvailable(sizeData);
    }
}

bool verifMustCompact(){
    if(memory.configuration.countFailedSearchForCompact == -1){
        pthread_mutex_lock(memory.m_partitions_modify);
        bool response = list_all_satisfy(memory.partitions, partition_is_free);
        pthread_mutex_unlock(memory.m_partitions_modify);
        return response;
    }
    pthread_mutex_lock(memory.m_failed_search_modify);
    bool response = memory.failedSearchCount == memory.configuration.countFailedSearchForCompact;
    pthread_mutex_unlock(memory.m_failed_search_modify);
    return response;
}

bool partition_is_free(void* data) {
    t_data* partition = (t_data*)data;
    return partition->state == FREE;
}

void compact(){
    memory.failedSearchCount = 0;
    if(strcmp(memory.configuration.memoryAlgorithm, "BS") == 0){
        BS_compact();
    }else{
        DP_compact();
    }
}

void destroyPartition(){
    pthread_mutex_lock(memory.m_failed_search_modify);
    memory.failedSearchCount++;
    pthread_mutex_unlock(memory.m_failed_search_modify);
    if(strcmp(memory.configuration.replaceAlgorithm, "FIFO") == 0){
        FIFO_destroyPartition();
    }else{
        LRU_destroyPartition();
    }
}

void allocateData(uint32_t sizeData, t_data* freePartition){
    uint32_t partition_size = sizeData > memory.configuration.minimunPartitionSize ? 
                sizeData : memory.configuration.minimunPartitionSize;
    if(strcmp(memory.configuration.memoryAlgorithm, "BS") == 0){
        BS_allocateData(partition_size, freePartition);
    }else{
        DP_allocateData(partition_size, freePartition);
    }
}


//region memory algorithms
t_data* FF_getPartitionAvailable(uint32_t sizeData){
    pthread_mutex_lock(memory.m_partitions_modify);
    t_data* response = (t_data*)list_find_with_args(memory.partitions, partition_size_validation,(void*)sizeData);
    pthread_mutex_unlock(memory.m_partitions_modify);
    return response;
}
t_data* BF_getPartitionAvailable(uint32_t sizeData){
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList =  list_size(memory.partitions);
    uint32_t minimunSize = sizeData;
    t_data* dataAux;
    for(int i = 0; i < sizeList; i++){
        dataAux = (t_data*)list_get(memory.partitions, i);
        if(dataAux->state == FREE && dataAux->partition_size < minimunSize){
            minimunSize = dataAux->partition_size;
        }
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
    return FF_getPartitionAvailable(minimunSize);
}

bool partition_size_validation(void* data, void* sizeData){
    return data ? ((t_data*) data)->partition_size >= (uint32_t) sizeData && ((t_data*) data)->state == FREE : false;
}

void BS_compact(){
    //Debería unir solo cuando son particiones del "mismo bloque"
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList =  list_size(memory.partitions);
    pthread_mutex_unlock(memory.m_partitions_modify);
    t_data* previousPartition = NULL;
    t_data* dataAux;
    bool mustFinish = false;
    for(int i = 0; i < sizeList && !mustFinish; i++){
        pthread_mutex_lock(memory.m_partitions_modify);
        dataAux = (t_data*)list_get(memory.partitions, i);
        pthread_mutex_unlock(memory.m_partitions_modify);
        if(previousPartition == NULL){
            previousPartition = dataAux;
        }else{
            if(previousPartition->partition_size == dataAux->partition_size){
                if(previousPartition->state == FREE && dataAux->state == FREE){
                    //I join them
                    previousPartition->partition_size += dataAux->partition_size;
                    pthread_mutex_lock(memory.m_partitions_modify);
                    list_remove(memory.partitions, i);
                    pthread_mutex_unlock(memory.m_partitions_modify);
                    mustFinish = true;
                }else{
                    previousPartition = NULL;
                }
            }else{
                previousPartition = dataAux;
            }
        }
        if(mustFinish) BS_compact();
    }
    
}

bool sortByState(void* elem1, void* elem2){
    t_data* data1 = elem1, *data2 = elem2;
    if (data1->state == FREE && data2->state == USING){
        memcpy(memory.data + data1->offset,memory.data + data2->offset, data2->partition_size);
        data2->offset = data1->offset;
        data1->offset = data2->offset + data2->partition_size;
        return false;
    } else {
        return true;
    }
}



void DP_compact(){
    //It moves the partitions with state using to the init an join the free partitions
    pthread_mutex_lock(memory.m_partitions_modify);
    if (list_size(memory.partitions) == 1) return;
    list_sort(memory.partitions, sortByState);
    for (int i = list_size(memory.partitions) - 2 ; i >= 0 ; i--){
        t_data* elem1 = list_get(memory.partitions, i);
        t_data* elem2 = list_get(memory.partitions, i + 1 );
        if (elem1 && elem2 && elem1->state == FREE && elem2->state == FREE){
            elem1->partition_size += elem2->partition_size;
            list_remove(memory.partitions, i + 1);
            free(elem2);
        }
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
}


void FIFO_destroyPartition(){
    uint32_t minimumId = 4294967295;
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList =  list_size(memory.partitions);
    uint32_t indexFound = 0;
    t_data* dataAux;
    for(int i = 0; i < sizeList; i++){
        dataAux = (t_data*)list_get(memory.partitions, i);
        if(dataAux->state == USING && dataAux->id < minimumId){
            minimumId = dataAux->id;
            indexFound = i;
        }
    }
    t_data* partitionSelected = (t_data*)list_get(memory.partitions, indexFound);
    if (partitionSelected && partitionSelected->state == USING){
        partitionSelected->state = FREE;
        pthread_mutex_lock(partitionSelected->m_receivers_modify);
        list_destroy(partitionSelected->receivers);
        pthread_mutex_unlock(partitionSelected->m_receivers_modify);
        pthread_mutex_destroy(partitionSelected->m_receivers_modify);
        condense(indexFound);
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
}

void condense(int indexFound){
    t_data* partitionSelected = list_get(memory.partitions, indexFound );
    if (indexFound + 1 <= list_size(memory.partitions) - 1){
        t_data* nextPartition = list_get(memory.partitions, indexFound + 1 );
        if (nextPartition->state == FREE){
            list_remove(memory.partitions, indexFound + 1);
            partitionSelected->partition_size += nextPartition->partition_size;
            partitionSelected->creationTime = timestamp();
        }
    }
    if (indexFound - 1 >= 0){
        t_data* previousPartition = list_get(memory.partitions, indexFound -1 );
        if (previousPartition->state == FREE){
            list_remove(memory.partitions, indexFound - 1);
            partitionSelected->partition_size += previousPartition->partition_size; 
            partitionSelected->size = partitionSelected->partition_size; 
            partitionSelected->offset = previousPartition->offset;
            partitionSelected->creationTime = timestamp();
        }
    }
}

void LRU_destroyPartition(){
    uint64_t oldestTime = timestamp();
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList =  list_size(memory.partitions);
    uint32_t indexFinded = 0;
    t_data* dataAux;
    for(int i = 0; i < sizeList; i++){
        dataAux = (t_data*)list_get(memory.partitions, i);
        if(dataAux->state == USING && dataAux->lastTimeUsed < oldestTime){
            oldestTime = dataAux->lastTimeUsed;
            indexFinded = i;
        }
    }
    t_data* partitionSelected = (t_data*)list_get(memory.partitions, indexFinded);
    if (partitionSelected && partitionSelected->state == USING){
        partitionSelected->state = FREE;
        pthread_mutex_lock(partitionSelected->m_receivers_modify);
        list_destroy(partitionSelected->receivers);
        pthread_mutex_unlock(partitionSelected->m_receivers_modify);
        pthread_mutex_destroy(partitionSelected->m_receivers_modify);
        condense(indexFinded);
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
}

void BS_allocateData(uint32_t sizeData, t_data* freePartitionData){
    if(sizeData <= freePartitionData->partition_size / 2){
        t_data* newData = malloc(sizeof(t_data));
        newData->partition_size = freePartitionData->partition_size / 2;
        newData->offset = freePartitionData->offset + newData->partition_size;
        newData->state = FREE;
        pthread_mutex_lock(memory.m_partitions_modify);
        list_add(memory.partitions, newData);
        pthread_mutex_unlock(memory.m_partitions_modify);
        freePartitionData->partition_size = freePartitionData->partition_size / 2;
        BS_allocateData(sizeData, newData);
    }else{
        freePartitionData->creationTime = timestamp();
        freePartitionData->lastTimeUsed = freePartitionData->creationTime;
        freePartitionData->state = USING;
    }
}

bool _offsetAscending(void* data1, void*data2) {
        return ((t_data*) data1)->offset < ((t_data*) data2)->offset;
}

void DP_allocateData(uint32_t sizeData, t_data* freePartitionData){   
    if(sizeData != freePartitionData->size){
        //If the size of the data is bigger than the free space, its create a new partition
        t_data* newData = malloc(sizeof(t_data));
        newData->partition_size = freePartitionData->partition_size - sizeData;
        newData->size = newData->partition_size;
        newData->offset = freePartitionData->offset + sizeData;
        newData->state = FREE;
        pthread_mutex_lock(memory.m_partitions_modify);
        list_add(memory.partitions, newData);
        list_sort(memory.partitions, _offsetAscending);
        pthread_mutex_unlock(memory.m_partitions_modify);
    }
    freePartitionData->partition_size = sizeData;
    freePartitionData->size = sizeData;
    freePartitionData->creationTime = timestamp();
    freePartitionData->lastTimeUsed = freePartitionData->creationTime;
    freePartitionData->state = USING;
}

void dumpMemory(){
    log_info(optional_logger, "Dumping cache into %s", cfg_values.dump_file);
    FILE* file = txt_open_for_append(cfg_values.dump_file);

    txt_write_in_file(file, "------------------------------------------------------------------------------\n");
    dump_write_time(file);
    dump_partitions(file);
    txt_write_in_file(file, "------------------------------------------------------------------------------\n");

    txt_close_file(file);
    log_info(optional_logger, "Finished dumping.", cfg_values.dump_file);
}
void dump_write_time(FILE* file){
    time_t timer;
    char* timeFormated = malloc(26);
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(timeFormated, 26, "%d/%m/%Y %H:%M:%S", tm_info);
    char* prefix = "Dump: ";
    char* text = malloc(26 + strlen(prefix) + 1);
    strcpy(text, prefix);
    strcat(text, timeFormated);
    txt_write_in_file(file, text);
    txt_write_in_file(file, "\n");
}
void dump_partitions(FILE* file){
    pthread_mutex_lock(memory.m_partitions_modify);
    uint32_t sizeList = list_size(memory.partitions);
    char* strFormat_using = "Partición %d: %p - %p. [X] Size: %db LRU: %lu Cola: %d ID:%d\n";
    char* str_using = malloc(strlen(strFormat_using) + sizeof(void*)*2 + sizeof(int) * 4 + sizeof(long));
    char* strFormat_free = "Partición %d: %p - %p. [L] Size: %db\n";
    char* str_free = malloc(strlen(strFormat_free) + sizeof(void*)*2 + sizeof(int) * 2);
    void* initialPointer;
    void* endPointer;
    for(int i = 0; i < sizeList; i++){
        t_data* partition = list_get(memory.partitions, i);
        initialPointer = memory.data + partition->offset;
        endPointer = initialPointer + partition->partition_size;//TODO mis dudas
        if(partition->state == FREE){
            sprintf(str_free, strFormat_free, i, initialPointer, endPointer, partition->partition_size);
            txt_write_in_file(file, str_free);
        }else{
            sprintf(str_using, strFormat_using, i, initialPointer, endPointer, partition->partition_size, partition->lastTimeUsed, partition->idQueue, partition->id);
            txt_write_in_file(file, str_using);
        }
    }
    pthread_mutex_unlock(memory.m_partitions_modify);
}

t_data* assign_and_return_message(uint32_t id_queue, uint32_t sizeofrawstream, void* stream){
    pthread_mutex_lock(&m_new_partition);
    uint32_t sizeofdata;
    t_data* freePartition;
    switch(id_queue){
        case NEW_POKEMON:
        case CATCH_POKEMON:
        case GET_POKEMON:
            sizeofdata = sizeofrawstream - sizeof(uint32_t);
            freePartition = seekPartitionAvailable(sizeofdata);
            break;
        case APPEARED_POKEMON:
        case CAUGHT_POKEMON:
        case LOCALIZED_POKEMON:
            sizeofdata = sizeofrawstream - 2 * sizeof(uint32_t);
            freePartition = seekPartitionAvailable(sizeofdata);
            memcpy(&freePartition->id_correlational, stream + sizeofdata + sizeof(uint32_t), sizeof(uint32_t));
            break;
        default:
            return NULL;
    }
    allocateData(sizeofdata, freePartition);
    log_debug(optional_logger, "Creating new partition at position: %d", freePartition->offset);
    void* data = memory.data + freePartition->offset;
    memcpy(data, stream, sizeofdata);
    pthread_mutex_lock(&m_id_message);
    id_message++;
    freePartition->id = id_message;
    pthread_mutex_unlock(&m_id_message);
    freePartition->size = sizeofdata;
    freePartition->idQueue = id_queue;
    freePartition->receivers = list_create();
    freePartition->m_receivers_modify = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(freePartition->m_receivers_modify,NULL);
    pthread_mutex_unlock(&m_new_partition);
    return freePartition;
} 

void send_all_messages(t_connection* conn, uint32_t id_queue){
    pthread_mutex_lock(memory.m_partitions_modify);
    bool isFromQueue(void* elem){ //intellisense no lo reconoce pero compila
        return elem && ((t_data*) elem)->state == USING && ((t_data*) elem)->idQueue == id_queue;
    }
    t_list* queueMessages = list_filter(memory.partitions, isFromQueue);
    void sendMessage(void* data){
        t_data* message = data;
        bool hasReceiver(void* receiver){
            return ((t_receiver*) receiver)->conn->id_connection == conn->id_connection;
        }
        pthread_mutex_lock(message->m_receivers_modify);
        t_receiver* rec = list_find(message->receivers, hasReceiver);
        pthread_mutex_unlock(message->m_receivers_modify);
        if(rec == NULL){
            void* stream = memory.data + message->offset;
            t_paquete* package = stream_to_package(id_queue, stream, message->size);
            void* a_enviar = serializar_paquete(package,sizeof(uint32_t)*2 + package->buffer->size);
            send(conn->socket, a_enviar, sizeof(uint32_t)*2 + package->buffer->size, 0);
            message->lastTimeUsed = timestamp();
            t_receiver* receiver = malloc(sizeof(t_receiver));
            receiver->conn = conn;
            receiver->sent = true;
            pthread_mutex_lock(message->m_receivers_modify);
            list_add(message->receivers, receiver);
            pthread_mutex_unlock(message->m_receivers_modify);
        }
    }
    list_iterate(queueMessages, sendMessage);
    pthread_mutex_unlock(memory.m_partitions_modify);
}

//end region
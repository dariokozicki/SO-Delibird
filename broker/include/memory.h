#ifndef MEMORY_H
#define MEMORY_H

#include <commons/collections/list.h>
#include <commons/txt.h>
#include "common_utils.h"
#include "initializer.h"

typedef enum {
	FREE = 1,
	USING = 2
}e_dataState;

typedef struct t_data{
    uint32_t size;
    uint32_t offset;
    uint32_t idQueue;
    uint32_t id;
    uint32_t id_correlational;
    t_list* receivers;
    pthread_mutex_t* m_receivers_modify;
    e_dataState state;
    time_t lastTimeUsed;
    time_t creationTime;
} t_data;

typedef struct t_memory_configuration{
    char* memoryAlgorithm;
    char* replaceAlgorithm;
    char* freePartitionAlgorithm;
    uint32_t size;
    uint32_t minimunPartitionSize;
    uint32_t countFailedSearchForCompact;
} t_memory_configuration;

typedef struct t_memory{
    t_memory_configuration configuration;
    t_list* partitions;
    int32_t failedSearchCount;
    void* data;
} t_memory;

t_memory memory;

void initializeMemory();
void* mallocMemory(uint32_t idMensaje, uint32_t size);
void setIdQueue(uint32_t idQueue, uint32_t idMensaje);
bool partition_match_id_mensaje(void* data, void* idMensaje);
void* getData(uint32_t idMensaje);

t_data* seekPartitionAvailable(uint32_t sizeData);
t_data* getPartitionAvailable(uint32_t sizeData);
bool verifMustCompact();
bool partition_is_free(void* data);
void compact();
void destroyPartition();
void allocateData(uint32_t sizeData, t_data* freePartition);

t_data* FF_getPartitionAvailable();
t_data* BF_getPartitionAvailable();

bool partition_size_validation(void* data, void* sizeData);

void BS_compact();
void DP_compact();

void FIFO_destroyPartition();
void LRU_destroyPartition();

void BS_allocateData(uint32_t sizeData, t_data* freePartition);
void DP_allocateData(uint32_t sizeData, t_data* freePartition);

void dumpMemory();
void dump_write_time(FILE* file);
void dump_partitions(FILE* file);

t_data* assign_and_return_message(uint32_t id_queue, uint32_t sizeofrawstream, void* stream);
#endif
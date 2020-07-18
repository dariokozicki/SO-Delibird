#ifndef TEAM_H
#define TEAM_H

#include "common_utils.h"
#include "inicializacion.h"
#include "mapa.h"

t_position positionCompare;

void calculateTrainerFromNewToReady();
void calculateTrainerToReady(enum_process_state threadTrainerState);
t_threadTrainer* getClosestTrainer(t_position position, enum_process_state threadTrainerState);
uint32_t calculateDistance(t_position positionFrom, t_position positionTo);
//Called always that a trainer its ready
void calculateTrainerFromReadyToExec();
bool existsThreadTrainerInExec(void* threadTrainer);
void setTrainerToExec();
void setTrainerToExec_FirstCome();
void setTrainerToExec_SJF();
//Called when a pokemon appear, on deadlock thread, and on message "caught pokemon"
void calculateLeaveBlockedFromAppear();
void calculateLeaveBlockedFromDeadlock(uint32_t idTrainer, t_position positionTo);
void calculateLeaveBlockedFromCaught(uint32_t idTrainer);
bool trainerCompleteOwnObjetives(t_trainer* trainerAux);
bool compareStrings(void* string1, void* string2);
void calculateTrainerInExit(uint32_t idTrainer);
void writeTrainerMetrics(uint32_t idTrainer);
bool trainerStateIsExit(void* threadTrainer);
void writeTeamMetrics();
void finishTeam();



//algoritmos de planificacion 
void executeAlgorithm();
bool threadTrainerInExec(void* threadTrainerAux);
void execThreadTrainerSetedFCFS(t_threadTrainer*);
void execThreadTrainerSetedRR(t_threadTrainer*);
void execThreadTrainerSetedSJF_SD(t_threadTrainer*);
void execThreadTrainerSetedSJF_CD(t_threadTrainer*);

t_pokemon_on_map* getPokemonByPosition(t_position pokemonPosition);
bool pokemonsOnMapComparePosition(void* pokemonOnMapAux);
void move_to_objetive(t_trainer* trainerAux, t_position positionTo);
bool sendCatch(t_pokemon_on_map* pokemon);
void catch_succesfull(uint32_t id_trainer);
int calculatePokemonsOnReady();
void calculate_cpu_burst_threadTrainerOnExec(t_threadTrainer*, uint32_t);
void move_to_objetive_FCFS(t_threadTrainer*, t_pokemon_on_map);
void move_to_objetive_RR(t_threadTrainer*, t_pokemon_on_map, uint32_t);
void move_to_objetive_SJF_CD(t_threadTrainer*, t_pokemon_on_map, uint32_t);
bool isPokemonOnMapFree(void *);
bool isPokemonOnMapChasing(void *);
void send_catch_pokemon(uint32_t, t_pokemon_on_map);



#endif
#include "team.h"

int main(int argc, char ** argv){
    
    inicializar();

    //establecer conexion con broker como cliente
    //establecerConexionBroker();
    
    //establecer puerto escucha con gameboy como servidor
    //establecerConexionGAmeBoy();
    
    liberar_recursos();

    return EXIT_SUCCESS;
}

/*void establecerConexionGameBoy()
{
   start_server(valores.ip_team, valores.puerto_team, p_on_request);
}

void establecerConexionBroker()
{
    int conexion = crear_conexion(valores.ip_broker, valores.puerto_broker);
    enviar_mensaje("Prueba", conexion);
}

void stop_server(int socketServer)
{
    close(socketServer);
}
*/
void leer_config()
{   
    char* config_path = "./cfg/team.config";
    config = config_create(config_path); 
    if(config == NULL)   
    {
        error_show("Error al leer el config del proceso TEAM en %s\n", config_path);
        exit(CONFIG_FAIL);
    }           
}
void cargar_posiciones_team()
{   
    valores.posicion_entrenador = config_get_array_value(config, "POSICIONES_ENTRENADORES");
}
void cargar_pokemones_team()
{
    valores.pokemon_entrenador = config_get_array_value(config, "POKEMON_ENTRENADORES");
}
void cargar_objetivo_team()
{
    valores.objetivo_entrenador = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
}

void cargar_valores_config(t_config * config)
{
   // valores.posicion_entrenador = config_get_array_value(config, "POSICIONES_ENTRENADORES");
    //valores.pokemon_entrenador = config_get_array_value(config, "POKEMON_ENTRENADORES");
   // valores.objetivo_entrenador = config_get_array_value(config, "OBJETIVOS_ENTRENADORES");
    valores.tiempo_reconexion = (uint32_t)config_get_int_value(config, "TIEMPO_RECONEXION");
    valores.retardo_ciclo_cpu = (uint32_t)config_get_int_value(config, "RETARDO_CICLO_CPU");
    valores.algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    valores.quantum = (uint32_t)config_get_int_value(config, "QUANTUM");
    valores.estimacion_inicial = (uint32_t)config_get_int_value(config, "ESTIMACION_INICIAL");
    valores.ip_broker = config_get_string_value(config, "IP_BROKER");
    valores.puerto_broker= config_get_int_value(config, "PUERTO_BROKER");
    valores.ip_team = config_get_string_value(config, "IP_TEAM");
    valores.puerto_team = config_get_int_value(config, "PUERTO_TEAM");
}

void iniciar_logger_obligatorio()
{      
    char* log_config = config_get_string_value(config, "LOG_FILE");
    obligatory_logger = log_create(log_config, "TEAM", 1, LOG_LEVEL_INFO);
    if(obligatory_logger == NULL)
    {
        error_show("Error al crear el logger del proceso TEAM en %s\n", log_config);
        exit(LOG_FAIL);
    }
    log_info(obligatory_logger, "Log Obligatorio creado correctamente\n");
}
 
 void iniciar_logger_opcional()
{      
    char* log_config = config_get_string_value(config, "LOG_FILE_OPTIONAL");
    optional_logger = log_create(log_config, "TEAM", 1, LOG_LEVEL_INFO);
    if(optional_logger == NULL)
    {
        error_show("Error al crear el logger del proceso TEAM en %s\n", log_config);
        exit(LOG_FAIL);
    }
    log_info(optional_logger, "Log Opcional creado correctamente\n");
}
void inicializar()
{    
    leer_config();
    iniciar_logger_obligatorio();
    iniciar_logger_opcional();
    cargar_valores_config(config);
    log_info(optional_logger, "Inicializacion y carga de configuracion exitosa\n", LOG_LEVEL_INFO);
}

void liberar_recursos()
{ 
    if(config)
        config_destroy(config);

    if(obligatory_logger)
        log_destroy(obligatory_logger);

    if(optional_logger)
        log_destroy(optional_logger);
     
  //  stop_server(socketServer);
   // close(conexion);
}

enum t_algoritmo obtenerAlgoritmo()
{
	enum t_algoritmo esAlgoritmo;
	char* sAlgort = valores.algoritmo_planificacion;
    if(string_equals_ignore_case(sAlgort, "FIFO"))
		esAlgoritmo = fifo;
	else if(string_equals_ignore_case(sAlgort, "RR"))
		esAlgoritmo = rr;
	else if(string_equals_ignore_case(sAlgort, "SJF-SD"))
		esAlgoritmo = sjf_sd;
	else if(string_equals_ignore_case(sAlgort, "SJF-SD"))
		esAlgoritmo = sjf_cd;
	return esAlgoritmo;
}

#include "broker.h"


int main(int argc, char ** argv){
    initialize();

    start_server(
        cfg_values.ip_broker, 
        cfg_values.puerto_broker,
        p_on_request    
    );
    void* coso = NULL;
    pthread_join(client_listener, coso);
    finalize();
    return EXIT_SUCCESS;
}
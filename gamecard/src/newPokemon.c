#include "newPokemon.h"

//void newPokemon(int id_mensaje,char* pokemon,coordenadaPokemon coord, int cantidad){
    //Verificar si esta el pokemon en el directorio Pokemon, si no esta crearlo
    //Verificar si no hay otro proceso que lo esté abriendo. En caso de que si lo haya finalizar hilo y reintentar en "TIEMPO_DE_REINTENTO_OPERACION"
    //Verificar si la coordenada ya existe dentro del archivo. En caso de existir, agregar la cantidad pasada por parámetro a la actual. En caso de no existir se debe agregar al final del archivo una nueva línea indicando la cantidad de Pokémon pasadas.
    //Cerrar el archivo
    //Enviar un mensaje que contenga:ID del mensaje recibido, Pokémon y Coordenada a la cola de mensajes "APPEARED_POKEMON" 
    /**En caso de que no se pueda establecer conexion con el broker notificarlo por Logs y continuar**/
//}

void newPokemonTallGrass(new_pokemon* newPokemon){
    char* directorio = "./TALL_GRASS/Files/";
    char buffer[50];   
    char* stream = malloc(newPokemon->sizePokemon + 1);
    memcpy(stream, newPokemon->pokemon, newPokemon->sizePokemon); 
    stream[newPokemon->sizePokemon] = '\0';

    strcpy(buffer, "");
    strcat(buffer, directorio);
    strcat(buffer, stream);

    char* directory = malloc(strlen(directorio) + newPokemon->sizePokemon + 2);
    memcpy(directory, buffer, strlen(directorio) + newPokemon->sizePokemon + 1);
    memcpy(directory + strlen(directorio) + newPokemon->sizePokemon + 1, "\0", sizeof(char));

    int created = mkdir(directory, ACCESSPERMS);
    if(created != -1){
        pthread_mutex_lock(&mutexListOfMutex);
        mutexDirectory* mutex = malloc(sizeof(mutexDirectory));
        mutex->nombreDirectorio = stream;
        pthread_mutex_init(&mutex->mutex, NULL);
        list_add(mutexListDirectory, mutex); //Asi solo tiene el nombre del pokemon 
        pthread_mutex_unlock(&mutexListOfMutex);
    }
    
    createMetadataPokemon(directory, newPokemon);
    free(directory);
    free(stream);
    free(newPokemon->pokemon);
    free_new_pokemon(newPokemon);
}

void createMetadataPokemon(char* directory, new_pokemon* newPokemon){
    char* metadata = "/Metadata.bin";
    char* directorioMetadata = malloc(strlen(directory) + strlen(metadata) + 1);
    strcpy(directorioMetadata,"");
    strcat(directorioMetadata,directory);
    strcat(directorioMetadata, metadata);

    FILE* file = fopen(directorioMetadata,"ab+");

    fseek(file, 0, SEEK_END);
    int sizeFile = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);

    if(sizeFile == 0){
        configMetadataCreate(directorioMetadata);
    }

    char* stream = malloc(newPokemon->sizePokemon + 1);
    memcpy(stream, newPokemon->pokemon, newPokemon->sizePokemon); 
    stream[newPokemon->sizePokemon + 1] = '\0';

    if(metadataBlocks(directorioMetadata, stream) == 0){
        addBlockMetadata(directorioMetadata, newPokemon);
    }else{
        agregarDatosYOrdenarBloques(directorioMetadata, newPokemon);
    }
    
    free(directorioMetadata);
}

char* crearBloque(new_pokemon* newPokemon){
    char* directorio = "./TALL_GRASS/Blocks/";
    char* extension = ".bin";

    size_t sizeBitmap = bitarray_get_max_bit(bitmap);
    int bin = 1;  
    while(bin <= (int)sizeBitmap + 1){
        
        bool testBit = bitarray_test_bit(bitmap, bin - 1);
        if(testBit == 0){
            char* binChar = malloc(10);
            
            strcpy(binChar,"");
            sprintf(binChar, "%d", bin);
            char* directorioBloques = malloc(strlen(directorio) + strlen(binChar) + strlen(extension)+1); //ver de cambiarlo 
            strcpy(directorioBloques,"");
            strcat(directorioBloques,directorio);
            strcat(directorioBloques,binChar);
            strcat(directorioBloques,extension);
            
            FILE* binary = fopen(directorioBloques,"wb"); 

            char* posX = malloc(10);
            strcpy(posX,"");
            sprintf(posX,"%d",newPokemon->position.posx);//(char); 

            char* posY = malloc(10);
            strcpy(posY,"");
            sprintf(posY,"%d",newPokemon->position.posy);//(char); 

            char* quantity = malloc(10);
            strcpy(quantity,"");
            sprintf(quantity,"%d",newPokemon->quantity);//(char); 

            char* writeBinary = malloc(strlen(posX) + strlen("-") + strlen(posY) + strlen("=") + strlen("\n") + 3); 
            strcpy(writeBinary,"");
            strcat(writeBinary,posX);
            strcat(writeBinary,"-");
            strcat(writeBinary,posY);
            strcat(writeBinary, "=");
            strcat(writeBinary,quantity);
            strcat(writeBinary, "\n");

            fwrite(writeBinary, strlen(writeBinary) + 1, 1, binary);
            fclose(binary);
            bitarray_set_bit(bitmap, bin-1);
            free(posX);
            free(posY);
            free(quantity);
            free(directorioBloques);
            free(writeBinary);
            //imprimirBITARRAY(bitmap);
            return binChar;
        }else{
            bin++;
        }
    }

    return NULL;
}


void configMetadataCreate(char* metadata){
    t_config* configMetadataTallGrass = config_create("./cfg/tall_grass_metadata.config");

    config_save_in_file(configMetadataTallGrass, metadata);
    config_destroy(configMetadataTallGrass);
}

void addBlockMetadata(char* metadata, new_pokemon* newPokemon){
    char* stream = malloc(newPokemon->sizePokemon + 1);
    memcpy(stream, newPokemon->pokemon, newPokemon->sizePokemon); 
    stream[newPokemon->sizePokemon] = '\0';

    abrirMetadata(metadata, stream);
    char* block = crearBloque(newPokemon);
    t_config* configMetadataTallGrass = config_create(metadata);

    int size = config_get_int_value(configMetadataTallGrass, "SIZE");
    int cantidadBloques = ceil((float)size / configM.blockSize);
    int cantidadBloquesMasUno = cantidadBloques + 1;
    
    char** bloques = config_get_array_value(configMetadataTallGrass,"BLOCKS");

    char* bloquesConfig = malloc(sizeof(char)*2*(cantidadBloques + 1) + 1);
    strcpy(bloquesConfig,"");
    strcat(bloquesConfig,"[");
    for(int i = 0; i < cantidadBloques; i++){
        strcat(bloquesConfig,bloques[i]);
        strcat(bloquesConfig,",");
    }
    strcat(bloquesConfig,block); 
    strcat(bloquesConfig,"]");

    char* directorio = "./TALL_GRASS/Blocks/";
    char* extension = ".bin";

    char* bloque = malloc(strlen(directorio) + strlen(block) + strlen(extension) + 1);
    strcpy(bloque,"");
    strcat(bloque, directorio); 
    strcat(bloque, block); 
    strcat(bloque, extension);
    
    FILE* file = fopen(bloque,"rb");

    fseek(file, 0, SEEK_END);
    int sizeFile = ftell(file);
    fseek(file, 0, SEEK_SET);

    fclose(file);

    size += sizeFile;

    char* sizeChar = malloc(sizeof(uint32_t));
    strcpy(sizeChar, "");
    sprintf(sizeChar, "%d", size);

    config_set_value(configMetadataTallGrass, "SIZE", sizeChar);
    config_set_value(configMetadataTallGrass,"BLOCKS", bloquesConfig);

    config_save(configMetadataTallGrass);

    config_destroy(configMetadataTallGrass);
    cerrarMetadata(metadata, stream);
    free(stream);
    free(bloque);
    free(bloquesConfig);
    free(bloques);
    free(sizeChar);
    free(block);
}

void abrirMetadata(char* metadata, char* pokemon){

    uint32_t timeConfig = config_get_int_value(config,"TIEMPO_DE_REINTENTO_OPERACION");
    
    pthread_mutex_lock(&mutexListOfMutex);
    mutexDirectory* mutexPokemon = list_find_with_args(mutexListDirectory, esPokemon, pokemon);
    pthread_mutex_unlock(&mutexListOfMutex);

    pthread_mutex_lock(&mutexPokemon->mutex);
    t_config* configMetadataTallGrass = config_create(metadata);
    config_set_value(configMetadataTallGrass, "OPEN", "Y");
    config_save(configMetadataTallGrass);

    config_destroy(configMetadataTallGrass);
}

void cerrarMetadata(char* metadata, char* pokemon){
    t_config* configMetadataTallGrass = config_create(metadata);
    
    uint32_t timeRetardo = config_get_int_value(config,"TIEMPO_RETARDO_OPERACION");
    sleep(timeRetardo);

    pthread_mutex_lock(&mutexListOfMutex);
    mutexDirectory* mutexPokemon = list_find_with_args(mutexListDirectory, esPokemon, pokemon);
    pthread_mutex_unlock(&mutexListOfMutex);

    config_set_value(configMetadataTallGrass, "OPEN", "N");
    config_save(configMetadataTallGrass);

    config_destroy(configMetadataTallGrass);
    pthread_mutex_unlock(&mutexPokemon->mutex);
}

int metadataBlocks(char* metadata, char* pokemon){
    abrirMetadata(metadata, pokemon);
    t_config* configMetadataTallGrass = config_create(metadata);

    int size = config_get_int_value(configMetadataTallGrass, "SIZE");
    int cantidadBloques = ceil((float)size / configM.blockSize);

    config_destroy(configMetadataTallGrass);
    cerrarMetadata(metadata,pokemon);
    return cantidadBloques;
}

void agregarDatosYOrdenarBloques(char* metadata, new_pokemon* newPokemon){
    char* stream = malloc(newPokemon->sizePokemon + 1);
    memcpy(stream, newPokemon->pokemon, newPokemon->sizePokemon); 
    stream[newPokemon->sizePokemon] = '\0';

    abrirMetadata(metadata, stream);
    t_config* configMetadataTallGrass = config_create(metadata);

    char** bloques = config_get_array_value(configMetadataTallGrass,"BLOCKS");
    int size = config_get_int_value(configMetadataTallGrass, "SIZE");
    int cantidadBloques = ceil((float)size / configM.blockSize);

    config_destroy(configMetadataTallGrass);
    cerrarMetadata(metadata, stream);
    char* ultimoBloque = bloques[cantidadBloques-1]; 

    char* directorio = "./TALL_GRASS/Blocks/";
    char* extension = ".bin";

    char* bloque = malloc(strlen(directorio) + sizeof(char) + strlen(extension)+1);
    strcpy(bloque,"");
    strcat(bloque, directorio); 
    strcat(bloque, ultimoBloque); 
    strcat(bloque, extension);

    FILE* file = fopen(bloque,"ab+");
    fseek(file, 0, SEEK_END);
    int sizeFile = ftell(file);
    fseek(file,0,SEEK_SET);

    if(sizeFile >= configM.blockSize){
        addBlockMetadata(metadata, newPokemon);
    }else{

        char* posX = malloc(10);
        strcpy(posX,"");
        sprintf(posX,"%d",newPokemon->position.posx);//(char); 

        char* posY = malloc(10);
        strcpy(posY,"");
        sprintf(posY,"%d",newPokemon->position.posy);//(char); 

        char* quantity = malloc(10);
        strcpy(quantity,"");
        sprintf(quantity,"%d",newPokemon->quantity);//(char); 

        t_list* lista = levantarBloquesAMemoria(bloques, cantidadBloques);

        positionQuantity* posicionNewPokemon = malloc(sizeof(positionQuantity));
        posicionNewPokemon->posicionX = atoi(posX); 
        posicionNewPokemon->posicionY = atoi(posY);
        posicionNewPokemon->cantidad = atoi(quantity);

        positionQuantity* posicionEncontrada = list_find_with_args(lista, coincidePosicion, (void*)posicionNewPokemon);

        if(posicionEncontrada != NULL){
            posicionEncontrada->cantidad = posicionEncontrada->cantidad + posicionNewPokemon->cantidad;
            free(posicionNewPokemon);
        }else{
            list_add(lista, posicionNewPokemon);
        }

        char* sizeMetadata = bajarBloquesADisco(lista, bloques, cantidadBloques, newPokemon->pokemon, newPokemon->position.posx, newPokemon->position.posy, newPokemon->quantity, metadata);
        abrirMetadata(metadata, stream); 
        t_config* configMetadataUpdated = config_create(metadata);
        config_set_value(configMetadataUpdated, "SIZE", sizeMetadata);
        config_save(configMetadataUpdated); 
        cerrarMetadata(metadata, stream);

        list_destroy_and_destroy_elements(lista,free);
        free(posY);
        free(posX);
        free(quantity);
        free(sizeMetadata);
        free(configMetadataUpdated);
    }

    fclose(file);
    free(bloque);
    free(bloques);
}

t_list* levantarBloquesAMemoria(char** bloques, int cantidadBloques){
    char* directorio = "./TALL_GRASS/Blocks/";
    char* extension = ".bin";
    t_list* listaPosiciones = list_create();
    int caracterActual = 0; 
    char buffer[10];
    memset(buffer,'\0',10);
    positionQuantity* lineaBloque = malloc(sizeof(positionQuantity));
    bool posx = true;
    bool posy = false;
    bool cant = false;
        
    for(int i = 0; i<cantidadBloques; i++){
        char* direccionBinario = malloc(strlen(directorio) + strlen(bloques[i]) + strlen(extension) + 1);

        strcpy(direccionBinario,"");
        strcat(direccionBinario,directorio);
        strcat(direccionBinario,bloques[i]);
        strcat(direccionBinario,extension);

        FILE* fileBloque = fopen(direccionBinario, "rb");

        int c; 
        
        while((c=fgetc(fileBloque)) != EOF){
            char position = (char) c; 
            if(posx){
                if(isdigit(position)){
                    buffer[caracterActual] = position;
                    caracterActual++;
                }else{
                    lineaBloque->posicionX = atoi(buffer);
                    posx = false;
                    posy = true;
                    memset(buffer,'\0',10);
                    caracterActual = 0;
                    continue;
                }     
            }else if(posy){
                if(isdigit(position)){
                    buffer[caracterActual] = position;
                    caracterActual++; 
                }else{
                    lineaBloque->posicionY = atoi(buffer);
                    posy = false;
                    cant = true;
                    memset(buffer,'\0',10);
                    caracterActual = 0;
                    continue;
                } 
            }else if(cant){
                if(isdigit(position)){
                    buffer[caracterActual] = position;
                    caracterActual++; 
                }else{
                    lineaBloque->cantidad = atoi(buffer);
                    posx = true;
                    cant = false;
                    memset(buffer,'\0',10);
                    caracterActual = 0;
                    list_add(listaPosiciones, lineaBloque);
                    lineaBloque = malloc(sizeof(positionQuantity));
                    continue;
                } 
            }
        }
        fclose(fileBloque);
        free(direccionBinario);
    }
    
    free(lineaBloque);
    return listaPosiciones;
}

bool coincidePosicion(void* elem, void* args){
    positionQuantity* posLista = (positionQuantity*) elem; 
    positionQuantity* posNewPokemon = (positionQuantity*) args;

    if(posLista->posicionX == posNewPokemon->posicionX &&
        posLista->posicionY == posNewPokemon->posicionY){
            return true;
    }
    return false; 
}

char* bajarBloquesADisco(t_list* lista, char** bloques, int cantidadBloques, char* nombrePokemon, uint32_t posx, uint32_t posy, uint32_t quantity, char* metadata){
    t_list* writeListBinary = list_map(lista, structALinea);
    char* listaConcatenada = concatenarStrings(writeListBinary);
    uint32_t sizeTotal = strlen(listaConcatenada) + 1;  //revisar \0
    int j=0;
    char* directorio = "./TALL_GRASS/Blocks/";
    char* extension = ".bin";
    for(int i = 0; i<cantidadBloques; i++){
        char* direccionBinario = malloc(strlen(directorio) + strlen(bloques[i]) + strlen(extension) + 1);

        strcpy(direccionBinario,"");
        strcat(direccionBinario,directorio);
        strcat(direccionBinario,bloques[i]);
        strcat(direccionBinario,extension);

        FILE* fileBloque = fopen(direccionBinario, "wb");
        
        int sizeArchivo = 0; 

        while(sizeArchivo < configM.blockSize){
            if(strlen(listaConcatenada) < j){
                break;
            }
            fputc(listaConcatenada[j],fileBloque);
            j++;
            sizeArchivo++;
        }
        fclose(fileBloque);

        if(sizeTotal > sizeArchivo && sizeArchivo >= configM.blockSize && bloques[i] == bloques[cantidadBloques - 1]){
            new_pokemon* newPokemon = malloc(sizeof(new_pokemon));
            newPokemon->sizePokemon = strlen(nombrePokemon); 
            newPokemon->pokemon = nombrePokemon;
            newPokemon->position.posx = posx;
            newPokemon->position.posy = posy; 
            newPokemon->quantity = quantity;
            addBlockMetadata(metadata, newPokemon);
            cantidadBloques++;
            t_config* configMetadataTallGrass = config_create(metadata);
            bloques = config_get_array_value(configMetadataTallGrass, "BLOCKS");
            free(newPokemon);
            config_destroy(configMetadataTallGrass);
        }
        
        free(direccionBinario);
    }

    list_destroy_and_destroy_elements(writeListBinary, free);
    free(listaConcatenada);

    char* sizeTotalChar = malloc(20);
    strcpy(sizeTotalChar, "");
    sprintf(sizeTotalChar, "%d", sizeTotal);

    return sizeTotalChar; 
}

void* structALinea(void* posicion){
    positionQuantity* lineaStruct = (positionQuantity*)posicion;

    char* posX = malloc(10);
    strcpy(posX,"");
    sprintf(posX,"%d",lineaStruct->posicionX);

    char* posY = malloc(10);
    strcpy(posY,"");
    sprintf(posY,"%d",lineaStruct->posicionY);

    char* quantity = malloc(10);
    strcpy(quantity,"");
    sprintf(quantity,"%d",lineaStruct->cantidad); 

    char* writeBinary = malloc(strlen(posX) + strlen("-") + strlen(posY) + strlen("=") + strlen(quantity) + 3); 
    strcpy(writeBinary,"");
    strcat(writeBinary,posX);
    strcat(writeBinary,"-");
    strcat(writeBinary,posY);
    strcat(writeBinary, "=");
    strcat(writeBinary,quantity);
    strcat(writeBinary, "\n");

    free(posX);
    free(posY);
    free(quantity);

    return (void*)writeBinary;
}

char* concatenarStrings(t_list* lista){
    uint32_t tamano = 0;
    for(int i = 0; i<list_size(lista); i++){
        tamano += strlen(list_get(lista, i));
    }
    tamano++;
    char* concatenacion = malloc(tamano);
    strcpy(concatenacion,"");

    for(int i = 0; i<list_size(lista); i++){
        strcat(concatenacion, (char*)list_get(lista,i));
    }

    return concatenacion;
}

bool esPokemon(void* elem, void* args){
    mutexDirectory* directorio = (mutexDirectory*) elem; 
    char* pokemon = (char*) args; 

    if(!strcmp(directorio->nombreDirectorio,pokemon)){
        return true;
    }
    return false;
}
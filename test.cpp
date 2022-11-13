#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;
//funciones
void rescatar();
void recuperarIp(string archivo);
int countIp(string archivo);
void pingear(string ip);
void nuevoHilo(string temp);
//variables globales
std::vector<thread> threads(1);
int countHilo = 0;
string packet;
mutex mtx;

int main(int argc, char * argv[]) {
    if(argv[1] == NULL ||argv[2] == NULL){
        cout << "No se han puestos los parametros necesarios para la ejecucion" << endl;
        cout << "Cerrando..." << endl;
        return -1;
    }
    packet = argv[2];
    int totalIp = 0;
    totalIp=countIp(argv[1]);
    threads.resize(totalIp);
    recuperarIp(argv[1]);
    for (auto& th : threads) {
        th.join();
    }
}

//Se cuentan las ip's para cambiar el tamaño del vector con los hilos
int countIp(string archivo){
    ifstream txt;
    string temp;
    int contador = 0;
    txt.open(archivo,ios::in);

    if(txt.fail()){
        cout << "No se ha encontrado archivo " << archivo << endl;
        return 0;
    }

    while(!txt.eof()){
        getline(txt,temp);
        if(temp != "") contador++;
    }
    txt.close();
    return contador;
}

//Se comienza el proceso para recuperar ip's para pingear
void recuperarIp(string archivo){
    ifstream txt;
    string temp;
    txt.open(archivo,ios::in);

    if(txt.fail()){
        cout << "No se ha encontrado archivo " << archivo << endl;
        return;
    }

    while(!txt.eof()){
        getline(txt,temp);
        if(temp != "") nuevoHilo(temp);
    }
    txt.close();
    return;
}

//Comienza el "pingeo". Imprime en consola los resultados deseados,usa txt de intermediarios para obtener la informacion
void pingear(string ip){
    string comando;
    string nombre_txt = ip + ".txt";
    comando = "ping " + ip + " -q -c"+ packet +" > " + nombre_txt;
    system(comando.c_str());

    ifstream txtIP;
    txtIP.open(nombre_txt,ios::in);  
    
    //buscamos la el substring --- en el archivo
    string cadena, flag="---";
    do{
        txtIP >> cadena;
    } while (!txtIP.eof() && cadena!=flag);
    //salto de linea para llegar a la linea de interes
    getline(txtIP, cadena);
    //guardamos en cadena, la linea con el resumen del comando ping
    getline(txtIP, cadena);

    txtIP.close();

    //borrar archivo temporal de guardado
    comando = "rm " + nombre_txt;
    system(comando.c_str());
    //calculamos el largo del string
    int largoString = cadena.length();

    //Buscamos el numero antes de "paquetes transmitidos" del comando ping
    int i= cadena.find("r");
    int transmitidos =  atoi(cadena.substr(0, i-1).c_str());

    i= cadena.find(",");
    cadena= cadena.substr(i+1, largoString-i);
    largoString = cadena.length();
    i= cadena.find("r");
    string recibidos = cadena.substr(0, i);

    i= cadena.find(",");
    cadena= cadena.substr(i+1, largoString-i);

    //Buscamos el numero antes de "% packet loss" del comando ping
    i= cadena.find("i");
    int perdidos = atoi(cadena.substr(1, i-1).c_str());
    perdidos = (perdidos * transmitidos)/100;

    //mutex para el print en consola
    mtx.lock();
    cout << "ip = " << ip << " Trans: "<< transmitidos << " Rec: "<< recibidos << " Lost: "<< perdidos << endl;
    mtx.unlock();

}

//Genera un nuevo hilo para pingear
void nuevoHilo(string ip){
    threads[countHilo] = thread(pingear, ip);
    countHilo++;
}

    

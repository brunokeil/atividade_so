#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <random>

using namespace std;

// Comando para compilar:
// g++ ponte_mao_unica.cpp -o ponte_mao_unica -pthread -std=c++20

mutex mtx_console;

class Ponte {
private:
    mutex mtx;
    condition_variable cv;
    
    int sentido_atual = -1;
    int carros_na_ponte = 0;
    int carros_esperando[2] = {0, 0};
    
    int consecutivos = 0;
    const int LIMITE_CONSECUTIVOS = 2;

public:
    void entrar(int id, int sentido) {
        unique_lock<mutex> lock(mtx);
        carros_esperando[sentido]++;

        cv.wait(lock, [this, sentido]() {
            bool sentido_livre = (sentido_atual == -1 || sentido_atual == sentido);
            bool deve_ceder_vez = (consecutivos >= LIMITE_CONSECUTIVOS && carros_esperando[1 - sentido] > 0);
            return sentido_livre && !deve_ceder_vez;
        });

        carros_esperando[sentido]--;
        carros_na_ponte++;
        
        if (sentido_atual != sentido) {
            sentido_atual = sentido;
            consecutivos = 0; 
        }
        consecutivos++;

        {
            lock_guard<mutex> console_lock(mtx_console);
            string dir = (sentido == 0) ? "[ESQUERDA -> DIREITA]" : "[DIREITA -> ESQUERDA]";
            cout << " > Carro " << id << " ENTROU na ponte. " << dir 
                 << " (Total atravessando: " << carros_na_ponte << ")\n";
        }
    }

    void sair(int id, int sentido) {
        unique_lock<mutex> lock(mtx);
        carros_na_ponte--;

        {
            lock_guard<mutex> console_lock(mtx_console);
            cout << " < Carro " << id << " SAIU da ponte.\n";
        }

        if (carros_na_ponte == 0) {
            if (carros_esperando[1 - sentido] > 0) {

                sentido_atual = 1 - sentido; 
                consecutivos = 0;
            } else if (carros_esperando[sentido] == 0) {

                sentido_atual = -1; 
                consecutivos = 0;
            }
        }
        

        cv.notify_all();
    }
};

Ponte ponte_estreita;

void rotina_carro(int id, int sentido) {
    this_thread::sleep_for(chrono::milliseconds(100 + (rand() % 400)));

    ponte_estreita.entrar(id, sentido);

    this_thread::sleep_for(chrono::milliseconds(300));

    ponte_estreita.sair(id, sentido);
}

int main() {
    srand(time(NULL));
    cout << "--- Atividade 6.6: Ponte de Mao Unica ---\n";
    cout << "Iniciando controle de trafego...\n\n";

    vector<thread> frota;
    int total_carros = 10;

    for (int i = 1; i <= total_carros; ++i) {
        int sentido = i % 2; 
        frota.push_back(thread(rotina_carro, i, sentido));
    }

    for (auto& t : frota) {
        t.join();
    }

    cout << "\nTodos os carros atravessaram.\n";
    return 0;
}
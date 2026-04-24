#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <semaphore>
#include <chrono>

using namespace std;

// g++ produtor_consumidor.cpp -o produtor_consumidor -pthread -std=c++20
// ./produtor_consumidor

const int TAM_BUFFER = 10;
const int NUM_PRODUTORES = 2;
const int NUM_CONSUMIDORES = 2;
const int NUM_ITENS_POR_THREAD = 50000;
const int TOTAL_ESPERADO = NUM_PRODUTORES * NUM_ITENS_POR_THREAD;

int buffer_inseguro[TAM_BUFFER];
int in_inseguro = 0;
int out_inseguro = 0;
int total_consumido_inseguro = 0; 
int itens_no_buffer_inseguro = 0;

void produtor_inseguro() {
    for (int i = 0; i < NUM_ITENS_POR_THREAD; ++i) {
        buffer_inseguro[in_inseguro] = 1;
        in_inseguro = (in_inseguro + 1) % TAM_BUFFER;
        itens_no_buffer_inseguro++;
    }
}

void consumidor_inseguro() {
    for (int i = 0; i < NUM_ITENS_POR_THREAD; ++i) {
        int item = buffer_inseguro[out_inseguro];

        out_inseguro = (out_inseguro + 1) % TAM_BUFFER;
        itens_no_buffer_inseguro--;
    
        total_consumido_inseguro += item;
    }
}

void testar_inseguro() {
    vector<thread> produtores;
    vector<thread> consumidores;

    for (int i = 0; i < NUM_PRODUTORES; ++i)
        produtores.push_back(thread(produtor_inseguro));
        
    for (int i = 0; i < NUM_CONSUMIDORES; ++i)
        consumidores.push_back(thread(consumidor_inseguro));

    for (auto& t : produtores) t.join();
    for (auto& t : consumidores) t.join();
}

int buffer_seguro[TAM_BUFFER];
int in_seguro = 0;
int out_seguro = 0;
int total_consumido_seguro = 0;

counting_semaphore<TAM_BUFFER> sem_vazios(TAM_BUFFER);

counting_semaphore<TAM_BUFFER> sem_cheios(0);

mutex mtx_buffer;

mutex mtx_metricas;

void produtor_seguro() {
    for (int i = 0; i < NUM_ITENS_POR_THREAD; ++i) {
        sem_vazios.acquire();

        mtx_buffer.lock();
        buffer_seguro[in_seguro] = 1;
        in_seguro = (in_seguro + 1) % TAM_BUFFER;
        mtx_buffer.unlock();

        sem_cheios.release();
    }
}

void consumidor_seguro() {
    for (int i = 0; i < NUM_ITENS_POR_THREAD; ++i) {
        sem_cheios.acquire();

        mtx_buffer.lock();
        int item = buffer_seguro[out_seguro];
        buffer_seguro[out_seguro] = 0;
        out_seguro = (out_seguro + 1) % TAM_BUFFER;
        mtx_buffer.unlock();

        sem_vazios.release();

        lock_guard<mutex> lock(mtx_metricas);
        total_consumido_seguro += item;
    }
}

void testar_seguro() {
    vector<thread> produtores;
    vector<thread> consumidores;

    for (int i = 0; i < NUM_PRODUTORES; ++i)
        produtores.push_back(thread(produtor_seguro));
        
    for (int i = 0; i < NUM_CONSUMIDORES; ++i)
        consumidores.push_back(thread(consumidor_seguro));

    for (auto& t : produtores) t.join();
    for (auto& t : consumidores) t.join();
}


int main() {
    cout << "--- Atividade 6.2: Produtor-Consumidor com Buffer Limitado ---\n\n";

    cout << "1. Executando teste SEM protecao...\n";
    testar_inseguro();
    cout << "   -> Total consumido inseguro: " << total_consumido_inseguro << " (Esperado: " << TOTAL_ESPERADO << ")\n";
    cout << "   -> Itens perdidos no buffer (inseguro): " << itens_no_buffer_inseguro << " (Esperado: 0)\n\n";

    cout << "2. Executando teste COM protecao (Semaforos + Mutex)...\n";
    testar_seguro();
    cout << "   -> Total consumido seguro: " << total_consumido_seguro << " (Esperado: " << TOTAL_ESPERADO << ")\n\n";

    cout << "--- Comparacao dos Resultados ---\n";
    if (total_consumido_inseguro != TOTAL_ESPERADO) {
        cout << "A versao sem protecao gerou sobrescrita de dados\n"
             << "e leitura de posicoes vazias/repetidas.\n"
             << "Dados foram perdidos ou corrompidos.\n\n";
    }

    if (total_consumido_seguro == TOTAL_ESPERADO) {
        cout << "A versao com protecao garantiu a integridade dos dados.\n"
             << "Nenhum dado foi sobrescrito antes de ser lido, e nenhuma leitura ocorreu no vazio.\n";
    }

    return 0;
}
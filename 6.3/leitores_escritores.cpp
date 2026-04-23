#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

using namespace std;

// g++ leitores_escritores.cpp -o leitores -pthread
// ./leitores;

const int NUM_LEITORES = 5;
const int NUM_ESCRITORES = 2;

int recurso_inseguro = 0;

void leitor_inseguro(int id) {
    cout << "Leitor " << id << " lendo recurso: " << recurso_inseguro << "\n";
    this_thread::sleep_for(chrono::milliseconds(10));
}

void escritor_inseguro(int id) {
    int temp = recurso_inseguro;
    this_thread::sleep_for(chrono::milliseconds(20)); // Forçando condição de corrida
    recurso_inseguro = temp + 1;
    cout << "Escritor " << id << " atualizou recurso para: " << recurso_inseguro << "\n";
}

void testar_inseguro() {
    vector<thread> threads;
    for (int i = 0; i < NUM_ESCRITORES; ++i) threads.push_back(thread(escritor_inseguro, i));
    for (int i = 0; i < NUM_LEITORES; ++i) threads.push_back(thread(leitor_inseguro, i));
    for (auto& t : threads) t.join();
}

int recurso_seguro = 0;
int num_leitores = 0;
mutex mtx_recurso; 
mutex mtx_leitores;

void leitor_seguro(int id) {
    mtx_leitores.lock();
    num_leitores++;
    if (num_leitores == 1) {
        mtx_recurso.lock();
    }
    mtx_leitores.unlock();

    cout << "Leitor " << id << " lendo recurso: " << recurso_seguro << "\n";
    this_thread::sleep_for(chrono::milliseconds(10)); 

    mtx_leitores.lock();
    num_leitores--;
    if (num_leitores == 0) {
        mtx_recurso.unlock();
    }
    mtx_leitores.unlock();
}

void escritor_seguro(int id) {
    mtx_recurso.lock();
    
    int temp = recurso_seguro;
    this_thread::sleep_for(chrono::milliseconds(20)); 
    recurso_seguro = temp + 1;
    cout << "Escritor " << id << " atualizou recurso para: " << recurso_seguro << "\n";
    
    mtx_recurso.unlock();
}

void testar_seguro() {
    vector<thread> threads;
    for (int i = 0; i < NUM_ESCRITORES; ++i) threads.push_back(thread(escritor_seguro, i));
    for (int i = 0; i < NUM_LEITORES; ++i) threads.push_back(thread(leitor_seguro, i));
    for (auto& t : threads) t.join();
}

int main() {
    cout << "--- Atividade 6.3: Leitores e Escritores ---\n\n";

    cout << "1. Executando teste SEM protecao (Concorrencia Indevida)...\n";
    testar_inseguro();

    cout << "\n2. Executando teste COM protecao (Sincronizacao Adequada)...\n";
    testar_seguro();

    return 0;
}
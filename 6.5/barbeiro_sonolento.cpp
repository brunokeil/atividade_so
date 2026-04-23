#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <condition_variable>

using namespace std;

// g++ barbeiro_sonolento.cpp -o barbeiro -pthread
// ./barbeiro

const int NUM_CADEIRAS = 3;
const int NUM_CLIENTES = 8;
const int CORTES_DO_DIA = NUM_CLIENTES; 

int clientes_esperando_inseguro = 0;
int cortes_feitos_inseguro = 0;
bool simulacao_insegura_ativa = true;

void barbeiro_inseguro() {
    while (simulacao_insegura_ativa) {
        if (clientes_esperando_inseguro <= 0) {
            cout << "[Inseguro] Barbeiro dormindo...\n";
            this_thread::sleep_for(chrono::milliseconds(10));
        } else {
            int temp = clientes_esperando_inseguro;
            this_thread::sleep_for(chrono::milliseconds(5)); 
            clientes_esperando_inseguro = temp - 1;
            
            cortes_feitos_inseguro++;
            cout << "[Inseguro] Barbeiro cortando cabelo. Clientes na espera: " << clientes_esperando_inseguro << "\n";
            this_thread::sleep_for(chrono::milliseconds(15)); // Tempo do corte
        }
    }
}

void cliente_inseguro(int id) {
    this_thread::sleep_for(chrono::milliseconds(id * 5)); // Chegadas em tempos diferentes
    
    if (clientes_esperando_inseguro < NUM_CADEIRAS) {
        int temp = clientes_esperando_inseguro;
        this_thread::sleep_for(chrono::milliseconds(10)); 
        clientes_esperando_inseguro = temp + 1;
        cout << "[Inseguro] Cliente " << id << " sentou. Esperando: " << clientes_esperando_inseguro << "\n";
    } else {
        cout << "[Inseguro] Cliente " << id << " foi embora. Barbearia cheia.\n";
        cortes_feitos_inseguro++;
    }
}

void testar_inseguro() {
    simulacao_insegura_ativa = true;
    
    thread thread_barbeiro(barbeiro_inseguro);
    vector<thread> threads_clientes;
    
    for (int i = 0; i < NUM_CLIENTES; ++i) {
        threads_clientes.push_back(thread(cliente_inseguro, i));
    }
    
    for (auto& t : threads_clientes) {
        t.join();
    }
    
    simulacao_insegura_ativa = false;
    thread_barbeiro.join();
    
    cout << "\n[Resultado Inseguro] Cortes registrados (Esperado: " << CORTES_DO_DIA 
         << ", Realidade: " << cortes_feitos_inseguro << ") -> Note a inconsistencia!\n";
}

int clientes_esperando_seguro = 0;
int cortes_feitos_seguro = 0;
mutex mtx_barbearia;
condition_variable cv_barbeiro;
condition_variable cv_clientes;

void barbeiro_seguro() {
    while (cortes_feitos_seguro < CORTES_DO_DIA) {
        unique_lock<mutex> lock(mtx_barbearia);
        
        while (clientes_esperando_seguro == 0 && cortes_feitos_seguro < CORTES_DO_DIA) {
            cout << "[Seguro] Barbeiro dormindo...\n";
            cv_barbeiro.wait(lock);
        }
        
        if (cortes_feitos_seguro >= CORTES_DO_DIA) break;

        clientes_esperando_seguro--;
        cortes_feitos_seguro++;
        cout << "[Seguro] Barbeiro acordou/pronto. Cortando cabelo. Espera: " << clientes_esperando_seguro << "\n";
        
        cv_clientes.notify_one();
        
        lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(15)); // Tempo do corte
    }
}

void cliente_seguro(int id) {
    this_thread::sleep_for(chrono::milliseconds(id * 5)); // Chegadas em tempos diferentes
    
    unique_lock<mutex> lock(mtx_barbearia);
    
    if (clientes_esperando_seguro < NUM_CADEIRAS) {
        clientes_esperando_seguro++;
        cout << "[Seguro] Cliente " << id << " sentou. Esperando: " << clientes_esperando_seguro << "\n";
        
        cv_barbeiro.notify_one();
        
        cv_clientes.wait(lock);
        cout << "[Seguro] Cliente " << id << " esta cortando o cabelo.\n";
    } else {
        cout << "[Seguro] Cliente " << id << " foi embora. Barbearia cheia.\n";
        cortes_feitos_seguro++;
        if (cortes_feitos_seguro == CORTES_DO_DIA) cv_barbeiro.notify_all();
    }
}

void testar_seguro() {
    vector<thread> threads;
    threads.push_back(thread(barbeiro_seguro));
    for (int i = 0; i < NUM_CLIENTES; ++i) threads.push_back(thread(cliente_seguro, i));
    for (auto& t : threads) t.join();
}

int main() {
    cout << "--- Atividade 6.5: Barbeiro Sonolento ---\n\n";

    cout << "1. Executando teste SEM protecao...\n";
    testar_inseguro();

    cout << "\n2. Executando teste COM protecao (Sincronizacao Adequada)...\n";
    testar_seguro();

    return 0;
}
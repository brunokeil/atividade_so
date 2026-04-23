#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <condition_variable>

using namespace std;

// g++ estacionamento.cpp -o estacionamento -pthread
// ./estacionamento

const int NUM_VAGAS = 3;
const int NUM_VEICULOS = 10;

int vagas_ocupadas_inseguro = 0;

void veiculo_inseguro(int id) {
    this_thread::sleep_for(chrono::milliseconds(id * 2));
    
    if (vagas_ocupadas_inseguro < NUM_VAGAS) {
        
        int temp = vagas_ocupadas_inseguro;
        this_thread::sleep_for(chrono::milliseconds(5)); // Força a perda de CPU para outra thread
        vagas_ocupadas_inseguro = temp + 1;
        
        cout << "[Inseguro] Veiculo " << id << " ENTROU. Ocupacao: " 
             << vagas_ocupadas_inseguro << "/" << NUM_VAGAS;
        
        if (vagas_ocupadas_inseguro > NUM_VAGAS) {
            cout << " (ERRO: SUPERLOTADO!)";
        }
        cout << "\n";

        this_thread::sleep_for(chrono::milliseconds(30));

        temp = vagas_ocupadas_inseguro;
        this_thread::sleep_for(chrono::milliseconds(5));
        vagas_ocupadas_inseguro = temp - 1;
        
        cout << "[Inseguro] Veiculo " << id << " SAIU. Ocupacao: " << vagas_ocupadas_inseguro << "\n";
    } else {
        cout << "[Inseguro] Veiculo " << id << " foi embora. Estacionamento parecia cheio.\n";
    }
}

void testar_inseguro() {
    vector<thread> threads;
    for (int i = 1; i <= NUM_VEICULOS; ++i) {
        threads.push_back(thread(veiculo_inseguro, i));
    }
    for (auto& t : threads) t.join();
}

int vagas_ocupadas_seguro = 0;
mutex mtx_estacionamento;
condition_variable cv_estacionamento;

void veiculo_seguro(int id) {
    this_thread::sleep_for(chrono::milliseconds(id * 2));
    
    unique_lock<mutex> lock(mtx_estacionamento);
    
    if (vagas_ocupadas_seguro >= NUM_VAGAS) {
        cout << "[Seguro] Veiculo " << id << " AGUARDANDO vaga...\n";
    }
    
    cv_estacionamento.wait(lock, []{ return vagas_ocupadas_seguro < NUM_VAGAS; });
    
    vagas_ocupadas_seguro++;
    cout << "[Seguro] Veiculo " << id << " ENTROU. Ocupacao: " 
         << vagas_ocupadas_seguro << "/" << NUM_VAGAS << "\n";
         
    lock.unlock(); 

    this_thread::sleep_for(chrono::milliseconds(30));

    lock.lock();
    vagas_ocupadas_seguro--;
    cout << "[Seguro] Veiculo " << id << " SAIU. Ocupacao: " << vagas_ocupadas_seguro << "\n";
    lock.unlock();
    
    cv_estacionamento.notify_one(); 
}

void testar_seguro() {
    vector<thread> threads;
    for (int i = 1; i <= NUM_VEICULOS; ++i) {
        threads.push_back(thread(veiculo_seguro, i));
    }
    for (auto& t : threads) t.join();
}

int main() {
    cout << "--- Atividade 6.7: Estacionamento com vagas limitadas ---\n\n";

    cout << "1. Executando teste SEM protecao (Vai estourar a capacidade)...\n";
    testar_inseguro();

    cout << "\n------------------------------------------------------\n";

    cout << "\n2. Executando teste COM protecao (Espera organizada e sem superlotação)...\n";
    testar_seguro();

    return 0;
}
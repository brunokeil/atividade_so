#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

using namespace std;

// Comando para compilar:
// g++ jantar_filosofos.cpp -o jantar_filosofos -pthread -std=c++20

const int NUM_FILOSOFOS = 5;
const int NUM_REFEICOES = 3;

mutex cout_mtx;

timed_mutex garfos_inseguros[NUM_FILOSOFOS];
int deadlocks_detectados = 0;
int refeicoes_inseguras = 0;

void filosofo_inseguro(int id) {
    int garfo_esq = id;
    int garfo_dir = (id + 1) % NUM_FILOSOFOS;

    for (int i = 0; i < NUM_REFEICOES; ++i) {
        garfos_inseguros[garfo_esq].lock();

        this_thread::sleep_for(chrono::milliseconds(50));

        if (!garfos_inseguros[garfo_dir].try_lock_for(chrono::milliseconds(500))) {
            lock_guard<mutex> print_lock(cout_mtx);
            cout << "   [DEADLOCK] Filosofo " << id << " travou esperando o garfo " << garfo_dir << "!\n";
            garfos_inseguros[garfo_esq].unlock();
            deadlocks_detectados++;
            return;
        }

        {
            lock_guard<mutex> print_lock(cout_mtx);
            refeicoes_inseguras++;
        }

        garfos_inseguros[garfo_dir].unlock();
        garfos_inseguros[garfo_esq].unlock();
    }
}

void testar_inseguro() {
    vector<thread> filosofos;
    for (int i = 0; i < NUM_FILOSOFOS; ++i) {
        filosofos.push_back(thread(filosofo_inseguro, i));
    }
    for (auto& t : filosofos) t.join();
}

mutex garfos_seguros[NUM_FILOSOFOS];
int refeicoes_seguras = 0;

void filosofo_seguro(int id) {
    int garfo_menor = min(id, (id + 1) % NUM_FILOSOFOS);
    int garfo_maior = max(id, (id + 1) % NUM_FILOSOFOS);

    for (int i = 0; i < NUM_REFEICOES; ++i) {
        garfos_seguros[garfo_menor].lock();
                this_thread::sleep_for(chrono::milliseconds(50));
        
        garfos_seguros[garfo_maior].lock();

        {
            lock_guard<mutex> print_lock(cout_mtx);
            refeicoes_seguras++;
        }

        garfos_seguros[garfo_maior].unlock();
        garfos_seguros[garfo_menor].unlock();
    }
}

void testar_seguro() {
    vector<thread> filosofos;
    for (int i = 0; i < NUM_FILOSOFOS; ++i) {
        filosofos.push_back(thread(filosofo_seguro, i));
    }
    for (auto& t : filosofos) t.join();
}

int main() {
    cout << "--- Atividade 6.4: Jantar dos Filosofos ---\n\n";

    cout << "1. Executando simulacao COM risco de Deadlock...\n";
    testar_inseguro();
    cout << "   -> Refeicoes concluidas (Inseguro): " << refeicoes_inseguras << " (Esperado: " << (NUM_FILOSOFOS * NUM_REFEICOES) << ")\n";
    cout << "   -> Deadlocks disparados: " << deadlocks_detectados << "\n\n";

    cout << "2. Executando simulacao COM estrategia de prevencao...\n";
    testar_seguro();
    cout << "   -> Refeicoes concluidas (Seguro): " << refeicoes_seguras << " (Esperado: " << (NUM_FILOSOFOS * NUM_REFEICOES) << ")\n";
    cout << "   -> Nenhum deadlock ocorreu!\n\n";

    return 0;
}
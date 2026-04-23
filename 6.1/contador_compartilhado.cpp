#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

// g++ contador_compartilhado.cpp -o contador -pthread
// ./contador;

const int NUM_THREADS = 10;
const int NUM_ITERATIONS = 100000;

int contador_inseguro = 0;

void operacao_insegura() {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        contador_inseguro++;
    }
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        contador_inseguro--;
    }
}

void testar_inseguro() {
    vector<thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(thread(operacao_insegura));
    }
    for (auto& t : threads) {
        t.join();
    }
}

int contador_seguro = 0;
mutex mtx;

void operacao_segura() {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        lock_guard<mutex> lock(mtx);
        contador_seguro++;
    }
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        lock_guard<mutex> lock(mtx);
        contador_seguro--;
    }
}

void testar_seguro() {
    vector<thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(thread(operacao_segura));
    }
    for (auto& t : threads) {
        t.join();
    }
}

int main() {
    cout << "--- Atividade 6.1: Contador Compartilhado ---\n\n";

    cout << "1. Executando teste SEM protecao (Condicao de Corrida)...\n";
    testar_inseguro();
    cout << "   -> Valor final do contador inseguro: " << contador_inseguro << " (Esperado: 0)\n\n";

    cout << "2. Executando teste COM protecao (Exclusao Mutua com Mutex)...\n";
    testar_seguro();
    cout << "   -> Valor final do contador seguro: " << contador_seguro << " (Esperado: 0)\n\n";

    cout << "--- Comparacao dos Resultados ---\n";
    if (contador_inseguro != 0) {
        cout << "Como demonstrado, a versao sem protecao falhou, resultando em " << contador_inseguro << " em vez de 0.\n";
    } else {
        cout << "A versao sem protecao resultou em 0 por sorte, mas e suscetivel a falhas. Tente rodar novamente.\n";
    }
    cout << "A versao com protecao garantiu a integridade dos dados, finalizando exatamente em " << contador_seguro << ".\n";

    return 0;
}
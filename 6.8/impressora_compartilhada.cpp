#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <string>

using namespace std;

// Comando para compilar:
// g++ impressora_compartilhada.cpp -o impressora_compartilhada -pthread


mutex mtx_console;


struct Documento {
    int ticket_global;
    int id_processo;
    string nome_arquivo;
};

class Impressora {
private:
    queue<Documento> fila_impressao;
    mutex mtx;
    condition_variable cv;
    bool desligando = false;

public:
    void enviar_documento(Documento doc) {
        {
            unique_lock<mutex> lock(mtx);
            fila_impressao.push(doc);
            
            lock_guard<mutex> console_lock(mtx_console);
            cout << " [+] Processo " << doc.id_processo 
                 << " enviou '" << doc.nome_arquivo 
                 << "' (Ticket #" << doc.ticket_global << "). Fila: " << fila_impressao.size() << "\n";
        }
        cv.notify_one(); 
    }

    void loop_de_impressao() {
        while (true) {
            Documento doc_atual;

            {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [this]() { return !fila_impressao.empty() || desligando; });

                if (desligando && fila_impressao.empty()) {
                    break;
                }

                doc_atual = fila_impressao.front();
                fila_impressao.pop();
            }
{
                lock_guard<mutex> console_lock(mtx_console);
                cout << "     ---> [IMPRIMINDO] Ticket #" << doc_atual.ticket_global 
                     << " do Processo " << doc_atual.id_processo 
                     << " ('" << doc_atual.nome_arquivo << "')...\n";
            }
            
            this_thread::sleep_for(chrono::milliseconds(400));
            
            {
                lock_guard<mutex> console_lock(mtx_console);
                cout << "     ---> [CONCLUIDO] Ticket #" << doc_atual.ticket_global << "\n";
            }
        }
    }

    void desligar() {
        {
            unique_lock<mutex> lock(mtx);
            desligando = true;
        }
        cv.notify_all();
    }
};

Impressora impressora_rede;
int contador_ticket_global = 1;
mutex mtx_ticket;

void rotina_processo(int id_processo, int num_documentos) {
    for (int i = 1; i <= num_documentos; ++i) {
        this_thread::sleep_for(chrono::milliseconds(100 + (rand() % 300)));

        int meu_ticket;
        {
            lock_guard<mutex> lock(mtx_ticket);
            meu_ticket = contador_ticket_global++;
        }

        string arquivo = "relatorio_v" + to_string(i) + ".pdf";
        Documento doc = {meu_ticket, id_processo, arquivo};
        
        impressora_rede.enviar_documento(doc);
    }
}

int main() {
    srand(time(NULL));
    cout << "--- Atividade 6.8: Impressora Compartilhada ---\n\n";

    thread thread_impressora(&Impressora::loop_de_impressao, &impressora_rede);

    vector<thread> processos;
    const int NUM_PROCESSOS = 3;
    const int DOCS_POR_PROCESSO = 2;

    for (int i = 1; i <= NUM_PROCESSOS; ++i) {
        processos.push_back(thread(rotina_processo, i, DOCS_POR_PROCESSO));
    }

    for (auto& p : processos) {
        p.join();
    }

    impressora_rede.desligar();
    thread_impressora.join();

    cout << "\nOrdem de envio e atendimento garantida.\n";
    return 0;
}
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <numeric>

// Recursos compartilhados para o Exemplo 1
std::mutex mtx_bloqueio;
int contador_compartilhado = 0;
const int OPERACOES_LEVES = 100000;

// Funcao que usa sincronizacao pesada (Exemplo 1)
void tarefa_com_bloqueio_extremo(int id) {
    for (int i = 0; i < OPERACOES_LEVES; ++i) {
        // A cada iteracao, a thread precisa adquirir e liberar o mutex.
        // O tempo gasto na sincronizacao se torna maior que o trabalho em si.
        std::lock_guard<std::mutex> lock(mtx_bloqueio); 
        contador_compartilhado++; 
    }
    // std::cout << "Thread " << id << " concluida." << std::endl;
}

// Funcao para trabalho leve (Exemplo 2)
void tarefa_muito_leve(int id) {
    volatile int x = 0; // 'volatile' evita otimizacoes que removeriam o loop
    for (int i = 0; i < 50; ++i) {
        x = i * i;
    }
    // A thread realiza um trabalho muito pequeno, e o custo de inicializacao 
    // e troca de contexto e desproporcional.
}

void exemplo_bloqueio_extremo() {
    std::cout << "\n--- Exemplo 1: Sincronizacao Extrema (Overhead de Mutex) ---" << std::endl;
    const int NUM_THREADS = 8;
    std::vector<std::thread> threads;
    contador_compartilhado = 0;

    // Medindo tempo com Multithreading e Alto Bloqueio
    auto inicio = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(tarefa_com_bloqueio_extremo, i);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto fim = std::chrono::high_resolution_clock::now();
    auto duracao = std::chrono::duration_cast<std::chrono::milliseconds>(fim - inicio);

    std::cout << "Resultado Final: " << contador_compartilhado << std::endl;
    std::cout << "Tempo gasto (Multi-thread com alto bloqueio): " << duracao.count() << " ms" << std::endl;
    std::cout << "Observacao: Em muitos casos, uma unica thread executando o loop sem mutex seria mais rapida." << std::endl;
}

void exemplo_tarefas_leves() {
    std::cout << "\n--- Exemplo 2: Tarefas Muito Leves (Overhead de Criacao de Thread) ---" << std::endl;
    const int NUM_TAREFAS = 1000;
    
    // Medindo tempo com Multithreading (criando 1000 threads)
    auto inicio_multi = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_TAREFAS; ++i) {
        // Criar, iniciar e destruir 1000 threads e muito caro para o SO.
        threads.emplace_back(tarefa_muito_leve, i); 
    }

    for (auto& t : threads) {
        t.join();
    }

    auto fim_multi = std::chrono::high_resolution_clock::now();
    auto duracao_multi = std::chrono::duration_cast<std::chrono::milliseconds>(fim_multi - inicio_multi);
    
    // Medindo tempo com Single Thread (executando 1000 vezes na main thread)
    auto inicio_single = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_TAREFAS; ++i) {
        tarefa_muito_leve(i);
    }
    auto fim_single = std::chrono::high_resolution_clock::now();
    auto duracao_single = std::chrono::duration_cast<std::chrono::milliseconds>(fim_single - inicio_single);

    std::cout << "Tempo gasto (Multi-thread - 1000 criacoes): " << duracao_multi.count() << " ms" << std::endl;
    std::cout << "Tempo gasto (Single-thread - 1000 execucoes): " << duracao_single.count() << " ms" << std::endl;
    std::cout << "Observacao: O tempo Single-thread deve ser muito MENOR, pois nao ha custo de criacao/troca de contexto." << std::endl;
}

int main() {
    exemplo_bloqueio_extremo();
    exemplo_tarefas_leves();
    return 0;
}

//=============================================================================
// PROBLEMA DO PRODUTOR-CONSUMIDOR
// Demonstração de Sincronização com std::thread, std::mutex e std::condition_variable
//=============================================================================

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

// === ESPECIFICAÇÕES ===
// Tamanho maximo do buffer (memoria compartilhada)
const int TAMANHO_BUFFER = 5; 
// Numero total de itens a serem produzidos e consumidos
const int MAX_ITENS = 12;      

// === RECURSOS COMPARTILHADOS ===
std::vector<int> buffer;             // O buffer compartilhado (memoria compartilhada)
std::mutex mtx;                      // Mutex para exclusao mutua (protege o buffer)
std::condition_variable cv_produtor; // Variavel de condicao para o produtor (buffer cheio)
std::condition_variable cv_consumidor; // Variavel de condicao para o consumidor (buffer vazio)

// === FUNCAO DO PRODUTOR ===
void produtor() {
    for (int i = 1; i <= MAX_ITENS; ++i) {
        // 1. Bloqueio do Mutex e Preparacao para Condicao
        std::unique_lock<std::mutex> lock(mtx);
        
        std::cout << "\n[PROD] -> Tentando produzir item " << i << ". Buffer: " << buffer.size() << "/" << TAMANHO_BUFFER << "..." << std::endl;

        // 2. Sincronizacao: Espera se o buffer estiver cheio
        // Se o buffer estiver cheio, o produtor e bloqueado.
        while (buffer.size() == TAMANHO_BUFFER) {
            std::cout << "[PROD] !! Buffer CHEIO. Produtor BLOQUEADO (Aguarda Consumidor)." << std::endl;
            // O produtor espera na cv_produtor, liberando o 'lock' enquanto espera.
            cv_produtor.wait(lock); 
        }

        // 3. Secao Critica: Produzir o item
        buffer.push_back(i);
        std::cout << "[PROD] ++ Item " << i << " PROD. Buffer: " << buffer.size() << "/" << TAMANHO_BUFFER << std::endl;

        // Adiciona um pequeno delay para melhor visualizacao da sincronizacao
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 

        // 4. Sinalizacao
        // Notifica o consumidor de que ha um novo item disponivel.
        cv_consumidor.notify_one(); 
        
        // O 'lock' (mutex) e liberado automaticamente quando o 'unique_lock' sai do escopo
        // ou quando uma das funcoes de espera (wait) e chamada.
    }
}

// === FUNCAO DO CONSUMIDOR ===
void consumidor() {
    for (int i = 1; i <= MAX_ITENS; ++i) {
        // 1. Bloqueio do Mutex e Preparacao para Condicao
        std::unique_lock<std::mutex> lock(mtx);

        std::cout << "\n[CONS] <- Tentando consumir item. Buffer: " << buffer.size() << "/" << TAMANHO_BUFFER << "..." << std::endl;

        // 2. Sincronizacao: Espera se o buffer estiver vazio
        // Se o buffer estiver vazio, o consumidor e bloqueado.
        while (buffer.empty()) {
            std::cout << "[CONS] -- Buffer VAZIO. Consumidor BLOQUEADO (Aguarda Produtor)." << std::endl;
            // O consumidor espera na cv_consumidor, liberando o 'lock' enquanto espera.
            cv_consumidor.wait(lock);
        }

        // 3. Secao Critica: Consumir o item
        int item_consumido = buffer.front();
        buffer.erase(buffer.begin()); // Remove o primeiro item (FIFO)
        std::cout << "[CONS] -- Item " << item_consumido << " CONS. Buffer: " << buffer.size() << "/" << TAMANHO_BUFFER << std::endl;

        // Adiciona um pequeno delay para melhor visualizacao da sincronizacao
        std::this_thread::sleep_for(std::chrono::milliseconds(400)); 

        // 4. Sinalizacao
        // Notifica o produtor de que ha espaco livre no buffer.
        cv_produtor.notify_one();
    }
}

// === FUNCAO PRINCIPAL ===
int main() {
    std::cout << "========================================================\n";
    std::cout << " SIMULACAO: PROBLEMA DO PRODUTOR-CONSUMIDOR EM C++\n";
    std::cout << " Tamanho do Buffer: " << TAMANHO_BUFFER << " | Total de Itens: " << MAX_ITENS << "\n";
    std::cout << "========================================================\n";

    // Cria as threads do Produtor e Consumidor
    std::thread produtor_thread(produtor);
    std::thread consumidor_thread(consumidor);

    // Aguarda a finalizacao de ambas as threads
    produtor_thread.join();
    consumidor_thread.join();

    std::cout << "\n========================================================\n";
    std::cout << " Sincronizacao concluida. Todos os itens foram processados.\n";
    std::cout << "========================================================\n";

    return 0;
}
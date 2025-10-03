#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <cmath>
#include <limits> // Para inicializar min/max com seguranca
#include <sstream> // Necessario para processar a linha de entrada

// === VARIAVEIS GLOBAIS ===
// Serão usadas para armazenar os resultados calculados pelas threads.
// O acesso a estas variaveis é seguro porque a thread-pai (main) só as lê
// APÓS todas as threads de trabalho terem terminado (com t.join()).
double valor_medio_global = 0.0;
int valor_minimo_global = 0;
int valor_maximo_global = 0;

// Lista de números global que será processada pelas threads
std::vector<int> dados;

// === FUNCOES DAS THREADS ===

// Thread 1: Calcula a média dos números
void calcular_media() {
    // Não precisa de mutex, pois a leitura do vetor 'dados' é segura (somente leitura).
    if (dados.empty()) {
        valor_medio_global = 0.0;
        return;
    }
    
    // Usa long long para a soma para evitar overflow em caso de muitos números grandes
    long long soma = std::accumulate(dados.begin(), dados.end(), 0LL);
    
    valor_medio_global = static_cast<double>(soma) / dados.size();
}

// Thread 2: Determina o valor mínimo
void calcular_minimo() {
    // Inicializa com o maior valor possível para garantir que o primeiro elemento
    // lido será sempre menor que o inicial.
    valor_minimo_global = std::numeric_limits<int>::max(); 

    if (dados.empty()) {
        valor_minimo_global = 0;
        return;
    }
    
    // std::min_element retorna um iterador para o menor elemento
    valor_minimo_global = *std::min_element(dados.begin(), dados.end());
}

// Thread 3: Determina o valor máximo
void calcular_maximo() {
    // Inicializa com o menor valor possível para garantir que o primeiro elemento
    // lido será sempre maior que o inicial.
    valor_maximo_global = std::numeric_limits<int>::min();

    if (dados.empty()) {
        valor_maximo_global = 0;
        return;
    }
    
    // std::max_element retorna um iterador para o maior elemento
    valor_maximo_global = *std::max_element(dados.begin(), dados.end());
}


// === FUNCAO PRINCIPAL (THREAD-PAI) ===
// A thread principal agora ignora argc/argv e usa cin para obter os dados.
int main() {
    std::cout << "=== Calculo de Estatisticas com Multiplos Threads ===" << std::endl;
    std::cout << "-----------------------------------------------------" << std::endl;
    
    std::string linha_entrada;
    
    // 1. Solicitacao e Processamento de Entrada (Aguarda o usuario digitar)
    std::cout << "Por favor, digite uma serie de numeros inteiros separados por espacos e pressione ENTER:" << std::endl;
    std::cout << "Exemplo: 90 81 78 95 79 72 85" << std::endl;

    // Aguarda a entrada do usuario
    std::getline(std::cin, linha_entrada);

    // Usa stringstream para processar os numeros da linha
    std::stringstream ss(linha_entrada);
    int num;

    while (ss >> num) {
        dados.push_back(num);
    }

    if (dados.empty()) {
        std::cerr << "\nERRO: Nenhuma dado valido foi inserido. Por favor, tente novamente." << std::endl;
        return 1;
    }
    
    // Confirma os dados lidos
    std::cout << "\nNumeros lidos: ";
    for (size_t i = 0; i < dados.size(); ++i) {
        std::cout << dados[i] << (i == dados.size() - 1 ? "" : " ");
    }
    std::cout << std::endl;

    // 2. Criacao e Disparo das Threads
    
    // Cada thread comeca sua execucao imediatamente e em paralelo.
    std::thread t_media(calcular_media); 
    std::thread t_minimo(calcular_minimo);
    std::thread t_maximo(calcular_maximo);

    std::cout << "Threads de trabalho criadas. Esperando finalizacao..." << std::endl;

    // 3. Sincronizacao: Aguarda a Finalizacao das Threads (join)
    
    // A funcao join() bloqueia o thread principal (main) ate que o thread filho
    // termine sua execucao. Isso e a chave para a sincronizacao neste exemplo.
    t_media.join();
    t_minimo.join();
    t_maximo.join();

    // 4. Exibicao dos Resultados pelo Thread-Pai (Apos a sincronizacao)
    std::cout << "\n=== Resultados das Estatisticas ===" << std::endl;
    // O valor medio é arredondado para exibicao, mas o calculo e feito com double
    std::cout << "O valor medio e " << static_cast<int>(std::round(valor_medio_global)) << std::endl;
    std::cout << "O valor minimo e " << valor_minimo_global << std::endl;
    std::cout << "O valor maximo e " << valor_maximo_global << std::endl;
    std::cout << "===================================" << std::endl;
    
    return 0;
}

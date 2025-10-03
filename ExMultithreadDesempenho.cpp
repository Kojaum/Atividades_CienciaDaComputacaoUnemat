#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <numeric>
#include <cmath>

// Funcao auxiliar para simular trabalho intensivo em CPU (calculo de fatorial simples)
long long calcular_fatorial(int n)
{
	long long resultado = 1;
	for (int i = 1; i <= n; ++i)
	{
		resultado *= i;
		// Simula um delay para garantir que a CPU trabalhe por um tempo
		if (i % 100000 == 0) std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
	return resultado;
}

// Funcao que sera executada por uma thread, calcula o fatorial de um intervalo
void tarefa_cpu_intensiva(int inicio, int fim, long long* resultado_parcial)
{
	long long soma_fatoriais = 0;
	for (int i = inicio; i <= fim; ++i)
	{
		// Para simplificar, calculamos o fatorial de um numero menor
		soma_fatoriais += calcular_fatorial(std::min(i, 10));
	}
	*resultado_parcial = soma_fatoriais;
}

int main()
{
	std::cout << "\n--- Exemplo 1: Processamento Intensivo em CPU (Melhoria de Desempenho) ---" << std::endl;

	const int TAMANHO_TRABALHO = 200000;
	int meio = TAMANHO_TRABALHO / 2;
	long long resultado1 = 0;
	long long resultado2 = 0;

	// Medindo o tempo com Multithreading
	auto inicio_multi = std::chrono::high_resolution_clock::now();

	// Cria duas threads para dividir o trabalho
	std::thread t1(tarefa_cpu_intensiva, 1, meio, &resultado1);
	std::thread t2(tarefa_cpu_intensiva, meio + 1, TAMANHO_TRABALHO, &resultado2);

	t1.join();
	t2.join();

	auto fim_multi = std::chrono::high_resolution_clock::now();
	auto duracao_multi = std::chrono::duration_cast<std::chrono::milliseconds>(fim_multi - inicio_multi);

	long long resultado_total = resultado1 + resultado2;
	std::cout << "Resultado Total (Multi-thread): " << resultado_total << std::endl;
	std::cout << "Tempo gasto (Multi-thread): " << duracao_multi.count() << " ms" << std::endl;

	// Medindo o tempo com Single Thread (para comparacao)
	long long resultado_single = 0;
	auto inicio_single = std::chrono::high_resolution_clock::now();

	// Executa todo o trabalho em uma unica chamada
	tarefa_cpu_intensiva(1, TAMANHO_TRABALHO, &resultado_single);

	auto fim_single = std::chrono::high_resolution_clock::now();
	auto duracao_single = std::chrono::duration_cast<std::chrono::milliseconds>(fim_single - inicio_single);

	std::cout << "Tempo gasto (Single-thread - Esperado): " << duracao_single.count() << " ms" << std::endl;

	std::cout << "Observacao: O tempo gasto na solucao Multi-thread deve ser significativamente MENOR que o tempo Single-thread." << std::endl;
	return 0;
}

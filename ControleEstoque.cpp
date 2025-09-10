//=============================================================================
// SISTEMA DE CONTROLE DE ESTOQUE - DEMONSTRACAO DE SYSTEM CALLS EM C++
//=============================================================================
// DESCRICAO:
// Sistema basico de gerenciamento de estoque que demonstra o uso DIRETO
// de system calls do Windows para manipulacao de arquivos, simulando
// um sistema real com menu e funcoes completas.
//
// FUNCIONALIDADES:
// - Cadastro de novos produtos
// - Listagem de todos os produtos
// - Busca de produtos por nome
// - Entrada e saida de mercadorias no estoque
// - Geracao de relatorio de status do estoque
//
// SYSTEM CALLS UTILIZADAS:
// - CreateFile: Criacao e abertura de arquivos
// - WriteFile: Escrita de dados
// - ReadFile: Leitura de dados
// - CloseHandle: Fechamento de recursos
// - SetFilePointer: Navegacao dentro do arquivo
//
// AUTOR: Kojão
// DISCIPLINA: Sistemas Operacionais
//=============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <stdio.h>
#include <algorithm> 
#include <cctype>    

// Definicoes de constantes para limites
#define MAX_NOME 100
#define MAX_ARQUIVO 20000 // Tamanho maximo do buffer para leitura de arquivo

// Estrutura para representar um produto no estoque
struct Produto {
    int id;
    char nome[MAX_NOME];
    int quantidade;
    double preco;
};

// Variaveis globais para nomes dos arquivos
const char* ARQUIVO_ESTOQUE = "estoque.txt";
const char* ARQUIVO_MOVIMENTACOES = "movimentacoes.txt";
const char* ARQUIVO_RELATORIO = "relatorio_estoque.txt";

// Prototipos das funcoes
void cadastrarProduto();
void listarProdutos();
void buscarProduto();
void darSaidaEmProduto();
void darEntradaEmProduto();
void gerarRelatorio();
void exibirMenu();
void tratarErro(const char* operacao);
std::vector<Produto> lerProdutosDoArquivo();
void salvarProdutosNoArquivo(const std::vector<Produto>& produtos);
void registrarMovimentacao(const Produto& produto, int quantidade, const std::string& tipo);

// Funcao auxiliar para tratar erros da API do Windows
void tratarErro(const char* operacao) {
    DWORD erro = GetLastError();
    std::cerr << "ERRO ao " << operacao << ". Codigo: " << erro << std::endl;
}

// 1. SYSTEM CALLS: CreateFile + WriteFile - Cadastro de produto
void cadastrarProduto() {
    std::cout << "\n=== CADASTRO DE PRODUTO ===\n";

    Produto novoProduto;
    std::string nomeTemp;

    // Entrada de dados do usuario
    std::cout << "ID do produto: ";
    std::cin >> novoProduto.id;
    std::cin.ignore(); // Limpa o buffer de entrada
    std::cout << "Nome do produto: ";
    std::getline(std::cin, nomeTemp);
    strcpy_s(novoProduto.nome, nomeTemp.c_str());
    std::cout << "Quantidade inicial: ";
    std::cin >> novoProduto.quantidade;
    std::cout << "Preco (R$): ";
    std::cin >> novoProduto.preco;
    std::cin.ignore();

    // SYSTEM CALL: CreateFile - Abre o arquivo para escrita no modo de anexar (append)
    HANDLE hArquivo = CreateFileA(
        ARQUIVO_ESTOQUE,             // Nome do arquivo
        GENERIC_WRITE,               // Acesso de escrita
        FILE_SHARE_READ,             // Permite que outros processos leiam o arquivo enquanto ele esta aberto
        NULL,                        // Atributos de seguranca padrao
        OPEN_ALWAYS,                 // Cria o arquivo se nao existir, ou o abre se existir
        FILE_ATTRIBUTE_NORMAL,       // Atributos normais
        NULL                         // Sem template
    );

    if (hArquivo == INVALID_HANDLE_VALUE) {
        tratarErro("criar/abrir arquivo de estoque");
        return;
    }

    // SYSTEM CALL: SetFilePointer - Move o ponteiro para o final do arquivo
    SetFilePointer(hArquivo, 0, NULL, FILE_END);

    // Formata a linha de dados a ser escrita no arquivo
    char linha[MAX_NOME + 50]; // Tamanho suficiente para os dados
    sprintf_s(linha, sizeof(linha), "%d|%s|%d|%.2f\n",
              novoProduto.id, novoProduto.nome, novoProduto.quantidade, novoProduto.preco);

    DWORD bytesEscritos;
    // SYSTEM CALL: WriteFile - Escreve a linha no arquivo
    if (!WriteFile(hArquivo,
                   linha,
                   strlen(linha),
                   &bytesEscritos,
                   NULL)) {
        tratarErro("escrever no arquivo");
        CloseHandle(hArquivo);
        return;
    }

    // SYSTEM CALL: CloseHandle - Fecha o arquivo para liberar o recurso
    CloseHandle(hArquivo);
    std::cout << "? Produto cadastrado com sucesso! (" << bytesEscritos << " bytes escritos)" << std::endl;
}

// 2. SYSTEM CALLS: CreateFile + ReadFile - Listagem de produtos
void listarProdutos() {
    std::cout << "\n=== PRODUTOS EM ESTOQUE ===\n";

    // SYSTEM CALL: CreateFile - Abre o arquivo para leitura
    HANDLE hArquivo = CreateFileA(
        ARQUIVO_ESTOQUE,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hArquivo == INVALID_HANDLE_VALUE) {
        std::cout << "Arquivo de estoque nao encontrado ou vazio." << std::endl;
        return;
    }

    // Obter o tamanho do arquivo para alocar o buffer de leitura
    DWORD tamanhoArquivo = GetFileSize(hArquivo, NULL);
    if (tamanhoArquivo == 0) {
        std::cout << "Estoque vazio." << std::endl;
        CloseHandle(hArquivo);
        return;
    }

    char* buffer = new char[tamanhoArquivo + 1];
    DWORD bytesLidos;

    // SYSTEM CALL: ReadFile - Le o conteudo do arquivo para o buffer
    BOOL sucesso = ReadFile(
        hArquivo,
        buffer,
        tamanhoArquivo,
        &bytesLidos,
        NULL
    );

    // SYSTEM CALL: CloseHandle - Fecha o arquivo
    CloseHandle(hArquivo);

    if (!sucesso) {
        tratarErro("ler arquivo");
        delete[] buffer;
        return;
    }

    buffer[bytesLidos] = '\0'; // Termina a string
    
    // Exibe o cabecalho
    printf("ID\t| Nome\t\t\t\t| Quantidade\t| Preco (R$)\n");
    printf("--------------------------------------------------------------------------------\n");

    // Processa o buffer lido linha por linha
    char* linha = strtok(buffer, "\n");
    int contador = 0;
    while (linha != NULL) {
        if (strlen(linha) > 0) {
            Produto p;
            sscanf_s(linha, "%d|%99[^|]|%d|%lf", &p.id, p.nome, (int)sizeof(p.nome), &p.quantidade, &p.preco);

            printf("%d\t| %-25s\t| %-10d\t| %.2f\n",
                   p.id, p.nome, p.quantidade, p.preco);
            contador++;
        }
        linha = strtok(NULL, "\n");
    }

    std::cout << "\nTotal de produtos: " << contador << std::endl;
    delete[] buffer;
}

// 3. SYSTEM CALLS: Leitura e filtragem - Busca de produto
void buscarProduto() {
    std::cout << "\n=== BUSCAR PRODUTO ===\n";
    std::cout << "Digite o nome (ou parte dele): ";
    std::string termoBusca;
    std::getline(std::cin, termoBusca);

    // A maneira mais simples de buscar e ler todo o arquivo
    // para a memoria, e entao filtrar
    std::vector<Produto> produtos = lerProdutosDoArquivo();

    if (produtos.empty()) {
        std::cout << "Estoque vazio ou erro ao ler o arquivo." << std::endl;
        return;
    }

    std::cout << "\nResultados da busca:\n";
    std::cout << "----------------------------------------------------------------\n";

    int encontrados = 0;
    for (const auto& p : produtos) {
        std::string nomeProduto = p.nome;
        // CORRECAO: Uso de std::transform e std::tolower apos inclusao dos cabecalhos
        std::transform(nomeProduto.begin(), nomeProduto.end(), nomeProduto.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        std::string buscaLower = termoBusca;
        std::transform(buscaLower.begin(), buscaLower.end(), buscaLower.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (nomeProduto.find(buscaLower) != std::string::npos) {
            printf("ID: %d | Nome: %s | Quantidade: %d | Preco: %.2f\n",
                   p.id, p.nome, p.quantidade, p.preco);
            encontrados++;
        }
    }

    if (encontrados == 0) {
        std::cout << "Nenhum produto encontrado com esse nome." << std::endl;
    } else {
        std::cout << "\n" << encontrados << " produto(s) encontrado(s)." << std::endl;
    }
}

// 4. SYSTEM CALLS: Leitura, modificacao e escrita - Saida de mercadoria
void darSaidaEmProduto() {
    std::cout << "\n=== SAIDA DE MERCADORIA ===\n";
    std::cout << "Digite o ID do produto: ";
    int idProduto;
    std::cin >> idProduto;
    std::cout << "Digite a quantidade a ser retirada: ";
    int quantidadeRetirada;
    std::cin >> quantidadeRetirada;

    std::vector<Produto> produtos = lerProdutosDoArquivo();
    if (produtos.empty()) {
        std::cout << "Estoque vazio ou erro ao ler o arquivo." << std::endl;
        return;
    }

    bool encontrou = false;
    for (auto& p : produtos) {
        if (p.id == idProduto) {
            encontrou = true;
            if (p.quantidade >= quantidadeRetirada) {
                p.quantidade -= quantidadeRetirada;
                std::cout << "? Retirada de " << quantidadeRetirada << " unidades de '" << p.nome << "' realizada com sucesso!" << std::endl;
                registrarMovimentacao(p, quantidadeRetirada, "SAIDA");
                // Salva a lista de produtos atualizada de volta no arquivo
                salvarProdutosNoArquivo(produtos);
            } else {
                std::cout << "? Erro: Quantidade insuficiente em estoque. Disponivel: " << p.quantidade << std::endl;
            }
            break;
        }
    }

    if (!encontrou) {
        std::cout << "? Produto nao encontrado!" << std::endl;
    }
}

// 5. SYSTEM CALLS: Leitura, modificacao e escrita - Entrada de mercadoria
void darEntradaEmProduto() {
    std::cout << "\n=== ENTRADA DE MERCADORIA ===\n";
    std::cout << "Digite o ID do produto: ";
    int idProduto;
    std::cin >> idProduto;
    std::cout << "Digite a quantidade a ser adicionada: ";
    int quantidadeAdicionada;
    std::cin >> quantidadeAdicionada;

    std::vector<Produto> produtos = lerProdutosDoArquivo();
    if (produtos.empty()) {
        std::cout << "Estoque vazio ou erro ao ler o arquivo." << std::endl;
        return;
    }

    bool encontrou = false;
    for (auto& p : produtos) {
        if (p.id == idProduto) {
            encontrou = true;
            p.quantidade += quantidadeAdicionada;
            std::cout << "? Adicao de " << quantidadeAdicionada << " unidades de '" << p.nome << "' realizada com sucesso!" << std::endl;
            registrarMovimentacao(p, quantidadeAdicionada, "ENTRADA");
            salvarProdutosNoArquivo(produtos);
            break;
        }
    }

    if (!encontrou) {
        std::cout << "? Produto nao encontrado!" << std::endl;
    }
}

// 6. SYSTEM CALLS: Criacao, leitura e escrita - Geracao de relatorio
void gerarRelatorio() {
    std::cout << "\n=== GERANDO RELATORIO DO ESTOQUE ===\n";

    std::vector<Produto> produtos = lerProdutosDoArquivo();
    if (produtos.empty()) {
        std::cout << "Estoque vazio ou erro ao ler o arquivo." << std::endl;
        return;
    }

    // SYSTEM CALL: CreateFile - Cria o arquivo de relatorio
    HANDLE hRelatorio = CreateFileA(
        ARQUIVO_RELATORIO,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,       // Sempre cria um novo arquivo, sobrescrevendo se existir
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hRelatorio == INVALID_HANDLE_VALUE) {
        tratarErro("criar arquivo de relatorio");
        return;
    }

    // Prepara o conteudo do relatorio
    char relatorio[2000];
    int totalProdutos = produtos.size();
    int totalQuantidade = 0;
    int produtosBaixoEstoque = 0;

    for (const auto& p : produtos) {
        totalQuantidade += p.quantidade;
        if (p.quantidade < 10) { // Exemplo: produtos com menos de 10 unidades
            produtosBaixoEstoque++;
        }
    }

    sprintf_s(relatorio, sizeof(relatorio),
              "=== RELATORIO DO ESTOQUE ===\n\n"
              "Gerado em: [Data e Hora Atual]\n" // Em uma versao real, voce usaria uma funcao de data/hora
              "----------------------------------------\n"
              "RESUMO\n"
              "Total de tipos de produtos: %d\n"
              "Quantidade total de itens: %d\n"
              "Produtos com baixo estoque (<10 un.): %d\n"
              "----------------------------------------\n\n"
              "LISTAGEM DETALHADA DE PRODUTOS\n",
              totalProdutos, totalQuantidade, produtosBaixoEstoque);

    DWORD bytesEscritos;
    // SYSTEM CALL: WriteFile - Escreve o cabecalho do relatorio
    WriteFile(hRelatorio, relatorio, strlen(relatorio), &bytesEscritos, NULL);

    // Escreve os detalhes de cada produto
    for (const auto& p : produtos) {
        char linha[MAX_NOME + 50];
        sprintf_s(linha, sizeof(linha), "ID: %d | Nome: %s | Qtd: %d | Preco: R$%.2f\n",
                  p.id, p.nome, p.quantidade, p.preco);
        WriteFile(hRelatorio, linha, strlen(linha), &bytesEscritos, NULL);
    }

    // SYSTEM CALL: CloseHandle
    CloseHandle(hRelatorio);
    std::cout << "? Relatorio gerado em '" << ARQUIVO_RELATORIO << "'" << std::endl;
}

// Funcao auxiliar para ler todos os produtos do arquivo para a memoria
std::vector<Produto> lerProdutosDoArquivo() {
    std::vector<Produto> produtos;
    HANDLE hArquivo = CreateFileA(
        ARQUIVO_ESTOQUE,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hArquivo == INVALID_HANDLE_VALUE) {
        return produtos; // Retorna um vetor vazio
    }

    DWORD tamanhoArquivo = GetFileSize(hArquivo, NULL);
    if (tamanhoArquivo == 0) {
        CloseHandle(hArquivo);
        return produtos;
    }

    char* buffer = new char[tamanhoArquivo + 1];
    DWORD bytesLidos;

    ReadFile(hArquivo, buffer, tamanhoArquivo, &bytesLidos, NULL);
    buffer[bytesLidos] = '\0';
    CloseHandle(hArquivo);

    char* linha = strtok(buffer, "\n");
    while (linha != NULL) {
        if (strlen(linha) > 0) {
            Produto p;
            sscanf_s(linha, "%d|%99[^|]|%d|%lf", &p.id, p.nome, (int)sizeof(p.nome), &p.quantidade, &p.preco);
            produtos.push_back(p);
        }
        linha = strtok(NULL, "\n");
    }

    delete[] buffer;
    return produtos;
}

// Funcao auxiliar para salvar todos os produtos de volta no arquivo
void salvarProdutosNoArquivo(const std::vector<Produto>& produtos) {
    HANDLE hArquivo = CreateFileA(
        ARQUIVO_ESTOQUE,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS, // Sobrescreve o arquivo existente
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hArquivo == INVALID_HANDLE_VALUE) {
        tratarErro("salvar dados no arquivo");
        return;
    }

    for (const auto& p : produtos) {
        char linha[MAX_NOME + 50];
        sprintf_s(linha, sizeof(linha), "%d|%s|%d|%.2f\n",
                  p.id, p.nome, p.quantidade, p.preco);
        DWORD bytesEscritos;
        WriteFile(hArquivo, linha, strlen(linha), &bytesEscritos, NULL);
    }

    CloseHandle(hArquivo);
}

// Funcao auxiliar para registrar uma movimentacao em um arquivo separado
void registrarMovimentacao(const Produto& produto, int quantidade, const std::string& tipo) {
    HANDLE hArquivo = CreateFileA(
        ARQUIVO_MOVIMENTACOES,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hArquivo != INVALID_HANDLE_VALUE) {
        SetFilePointer(hArquivo, 0, NULL, FILE_END);
        char linha[MAX_NOME + 50];
        // Em uma versao real, voce usaria uma funcao de data/hora
        sprintf_s(linha, sizeof(linha), "Tipo: %s | ID: %d | Nome: %s | Qtd: %d | Data: [atual]\n",
                  tipo.c_str(), produto.id, produto.nome, quantidade);
        DWORD bytesEscritos;
        WriteFile(hArquivo, linha, strlen(linha), &bytesEscritos, NULL);
        CloseHandle(hArquivo);
    }
}

// Funcao para exibir o menu principal
void exibirMenu() {
    std::cout << "\n=== MENU PRINCIPAL ===\n";
    std::cout << "1. Cadastrar Produto\n";
    std::cout << "2. Dar Entrada em Produto\n";
    std::cout << "3. Dar Saida em Produto\n";
    std::cout << "4. Listar Todos os Produtos\n";
    std::cout << "5. Buscar Produto\n";
    std::cout << "6. Gerar Relatorio de Estoque\n";
    std::cout << "0. Sair\n";
    std::cout << "Escolha uma opcao: ";
}

// Funcao principal do programa
int main() {
    int opcao;
    std::cout << "??? SISTEMA DE CONTROLE DE ESTOQUE ???\n";
    std::cout << "=====================================\n";
    std::cout << "Demonstracao de System Calls em C++\n";
    std::cout << "=====================================\n";

    do {
        exibirMenu();
        std::cin >> opcao;
        std::cin.ignore(); // Limpa o buffer de entrada

        switch (opcao) {
            case 1:
                cadastrarProduto();
                break;
            case 2:
                darEntradaEmProduto();
                break;
            case 3:
                darSaidaEmProduto();
                break;
            case 4:
                listarProdutos();
                break;
            case 5:
                buscarProduto();
                break;
            case 6:
                gerarRelatorio();
                break;
            case 0:
                std::cout << "?? Encerrando sistema... Ate logo!\n";
                break;
            default:
                std::cout << "? Opcao invalida! Tente novamente.\n";
        }
    } while (opcao != 0);

    return 0;
}

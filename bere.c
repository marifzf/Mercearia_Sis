#include "bere.h"
#include "cores.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h> 

/*==========================================================================*/
/* 1. CONSTANTES E DEFINICOES DE COR                                        */
/*==========================================================================*/
const char* RESET = "\033[0m"; 
const char* PURPLE_PHANTOM = "\033[38;2;239;177;224m";
const char* MIDNIGHT_HOWL = "\033[38;2;147;124;219m";
const char* VAMPIRE_BITE = "\033[38;2;253;104;152m";
const char* GHOUL_GREEN = "\033[38;2;129;205;165m";
const char* PUMPKIN_PUNCH = "\033[38;2;236;134;57m";
const char* VERDE        = "\033[1;32m";
const char* AMARELO      = "\033[1;33m";
const char* AZUL         = "\033[1;34m";
const char* MAGENTA      = "\033[1;35m";
const char* CIANO        = "\033[1;36m";
const char* rosaPastel = "\033[38;5;218m";
const char* verdeSuave = "\033[38;5;151m";
const char* vermelhoErro = "\033[31m"; 

/*==========================================================================*/
/* 2. ALOCACAO DE VARIAVEIS GLOBAIS (DEFINICAO)                             */
/*==========================================================================*/
Usuario *usuarios = NULL;
int numUsuarios = 0;
Usuario usuarioLogado; 
int idContadorVenda = 1; 

Cliente *clientes = NULL;
int numClientes = 0;
Produto *produtos = NULL;
int numProdutos = 0;

ItemVenda* carrinho = NULL;
int itensNoCarrinho = 0;
float totalCarrinho = 0.0;

Venda *vendas = NULL;
int numVendas = 0;

Pagamento *pagamentos = NULL;  
int numPagamentos = 0;

int caixaAberto = 0;
float saldoAbertura = 0.0;
float totalRetiradas = 0.0;

// Variaveis de Categoria
CategoriaProduto *categorias = NULL;
int numCategorias = 0;
int proximoCategoriaId = 1;


/*==========================================================================*/
/* 3. IMPLEMENTACAO: PERSISTENCIA (FILE I/O)                                */
/*==========================================================================*/

void carregarUsuarios() {
    FILE* f = fopen("usuarios.dat", "rb");
    if (f == NULL) {
        return; 
    }

    if (fread(&numUsuarios, sizeof(int), 1, f) != 1) {
        numUsuarios = 0;
        fclose(f);
        return;
    }

    if (numUsuarios > 0) {
        usuarios = (Usuario*)malloc(numUsuarios * sizeof(Usuario));
        if (usuarios == NULL) {
            printf("%s[ERRO] Falha na alocacao de memoria para usuarios.%s\n", vermelhoErro, RESET);
            numUsuarios = 0; 
            fclose(f);
            return;
        }
        
        if (fread(usuarios, sizeof(Usuario), numUsuarios, f) != numUsuarios) {
            free(usuarios);
            usuarios = NULL;
            numUsuarios = 0;
            printf("%s[ERRO] Dados de usuarios corrompidos ou incompletos.%s\n", vermelhoErro, RESET);
        }
    }
    
    fclose(f);
}

void carregarClientes() {
    FILE* f = fopen("clientes.dat", "rb");
    if (f == NULL) return;
    
    fread(&numClientes, sizeof(int), 1, f);
    if (numClientes == 0) { fclose(f); return; }
    
    clientes = malloc(numClientes * sizeof(Cliente));
    if (clientes == NULL) { numClientes = 0; fclose(f); return; }

    fread(clientes, sizeof(Cliente), numClientes, f);
    
    fclose(f);
}

void carregarProdutos() {
    FILE* f = fopen("produtos.dat", "rb");
    if (f == NULL) return;
    
    fread(&numProdutos, sizeof(int), 1, f);
    if (numProdutos == 0) { fclose(f); return; }
    
    produtos = malloc(numProdutos * sizeof(Produto));
    if (produtos == NULL) { numProdutos = 0; fclose(f); return; }

    fread(produtos, sizeof(Produto), numProdutos, f);
    
    fclose(f);
}

void carregarVendas() {
    FILE* f = fopen("vendas.dat", "rb");
    if (f == NULL) {
        idContadorVenda = 1; 
        return;
    }
    
    if (fread(&idContadorVenda, sizeof(int), 1, f) != 1) {
        idContadorVenda = 1; 
    }
    if (idContadorVenda <= 0) idContadorVenda = 1;

    fread(&numVendas, sizeof(int), 1, f);
    
    if (numVendas <= 0) { 
        numVendas = 0; 
        fclose(f); 
        return; 
    }
    
    vendas = (Venda*)malloc(numVendas * sizeof(Venda));
    if (vendas == NULL) { 
        printf("%s[ERRO CRITICO] Falha de memoria ao carregar vendas.%s\n", vermelhoErro, RESET);
        numVendas = 0; 
        fclose(f); 
        return; 
    }

    for (int i = 0; i < numVendas; i++) {
        fread(&vendas[i].idVenda, sizeof(int), 1, f);
        fread(&vendas[i].clienteCodigo, sizeof(int), 1, f);
        fread(vendas[i].dataVenda, sizeof(char), 11, f);
        fread(&vendas[i].totalVenda, sizeof(float), 1, f);
        fread(&vendas[i].status, sizeof(char), 1, f);
        fread(&vendas[i].numItens, sizeof(int), 1, f);
        
        vendas[i].itens = NULL;

        if (vendas[i].numItens > 0) {
            vendas[i].itens = (ItemVenda*)malloc(vendas[i].numItens * sizeof(ItemVenda));
            if (vendas[i].itens != NULL) {
                fread(vendas[i].itens, sizeof(ItemVenda), vendas[i].numItens, f);
            } else {
                vendas[i].numItens = 0;
            }
        }
    }
    fclose(f);
}

void carregarPagamentos() {
    FILE* f = fopen("pagamentos.dat", "rb");
    if (f == NULL) return;
    
    fread(&numPagamentos, sizeof(int), 1, f);
    if (numPagamentos == 0) { fclose(f); return; }
    
    pagamentos = malloc(numPagamentos * sizeof(Pagamento));
    if (pagamentos == NULL) { numPagamentos = 0; fclose(f); return; }

    fread(pagamentos, sizeof(Pagamento), numPagamentos, f);
    
    fclose(f);
}

void carregarCategorias() {
    FILE* arquivo = fopen("categorias.dat", "rb");
    
    if (arquivo == NULL) {
        numCategorias = 0;
        proximoCategoriaId = 1;
        
        if (categorias != NULL) {
            free(categorias);
            categorias = NULL;
        }

        cadastrarCategoriaPadrao("Alimento");
        cadastrarCategoriaPadrao("Limpeza");
        cadastrarCategoriaPadrao("Panificacao");
        
        salvarCategorias(); 
        return;
    }
    
    fread(&numCategorias, sizeof(int), 1, arquivo);
    fread(&proximoCategoriaId, sizeof(int), 1, arquivo); 

    if (numCategorias > 0) {
        categorias = (CategoriaProduto*)realloc(categorias, numCategorias * sizeof(CategoriaProduto));
        
        if (categorias == NULL) {
             printf("%s[ERRO] Alocacao de memoria para categorias falhou.%s\n", vermelhoErro, RESET);
             numCategorias = 0;
             proximoCategoriaId = 1; 
             fclose(arquivo);
             return;
        }
        fread(categorias, sizeof(CategoriaProduto), numCategorias, arquivo);
    } else {
        proximoCategoriaId = 1;
        if (categorias != NULL) {
            free(categorias);
            categorias = NULL;
        }
    }

    fclose(arquivo);
}

void carregarDadosPadrao() {
    if (numUsuarios == 0) {
        printf("%s >> Nenhum usuario encontrado. O sistema solicitara o cadastro do Administrador.%s\n", PUMPKIN_PUNCH, RESET);
    }
}

void carregarDados() {
    carregarUsuarios();
    carregarClientes();
    carregarProdutos();
    carregarVendas();
    carregarPagamentos();
    carregarCategorias();

    if (numUsuarios == 0 && numProdutos == 0) {
        carregarDadosPadrao();
    } else {
        printf("%s >> Dados carregados com sucesso!%s\n", verdeSuave, RESET);
    }
}

void salvarUsuarios() {
    FILE* f = fopen("usuarios.dat", "wb"); 
    if (f == NULL) return;
    
    fwrite(&numUsuarios, sizeof(int), 1, f);
    if (numUsuarios > 0) {
        fwrite(usuarios, sizeof(Usuario), numUsuarios, f);
    }
    
    fclose(f);
}

void salvarClientes() {
    FILE* f = fopen("clientes.dat", "wb");
    if (f == NULL) return;
    
    fwrite(&numClientes, sizeof(int), 1, f);
    if (numClientes > 0) {
        fwrite(clientes, sizeof(Cliente), numClientes, f);
    }
    
    fclose(f);
}

void salvarProdutos() {
    FILE* f = fopen("produtos.dat", "wb");
    if (f == NULL) return;
    
    fwrite(&numProdutos, sizeof(int), 1, f);
    if (numProdutos > 0) {
        fwrite(produtos, sizeof(Produto), numProdutos, f);
    }
    
    fclose(f);
}

void salvarVendas() {
    FILE *arquivo = fopen("vendas.dat", "wb");
    if (arquivo == NULL) {
        printf("%s[ERRO] Nao foi possivel salvar o arquivo de vendas.%s\n", vermelhoErro, RESET);
        return;
    }

    fwrite(&idContadorVenda, sizeof(int), 1, arquivo);

    fwrite(&numVendas, sizeof(int), 1, arquivo);

    for (int i = 0; i < numVendas; i++) {
        fwrite(&vendas[i].idVenda, sizeof(int), 1, arquivo);
        fwrite(&vendas[i].clienteCodigo, sizeof(int), 1, arquivo);
        fwrite(vendas[i].dataVenda, sizeof(char), 11, arquivo); 
        fwrite(&vendas[i].totalVenda, sizeof(float), 1, arquivo);
        fwrite(&vendas[i].status, sizeof(char), 1, arquivo);
        fwrite(&vendas[i].numItens, sizeof(int), 1, arquivo);
        
        if (vendas[i].numItens > 0) {
            fwrite(vendas[i].itens, sizeof(ItemVenda), vendas[i].numItens, arquivo);
        }
    }

    fclose(arquivo);
}

void salvarPagamentos() {
    FILE* f = fopen("pagamentos.dat", "wb");
    if (f == NULL) return;
    
    fwrite(&numPagamentos, sizeof(int), 1, f);
    if (numPagamentos > 0) {
        fwrite(pagamentos, sizeof(Pagamento), numPagamentos, f);
    }
    
    fclose(f);
}

void salvarCategorias() {
    FILE* arquivo = fopen("categorias.dat", "wb");
    if (arquivo == NULL) {
        printf("%s[ERRO] Erro ao abrir arquivo de categorias para salvar!%s\n", vermelhoErro, RESET);
        return;
    }

    fwrite(&numCategorias, sizeof(int), 1, arquivo);
    fwrite(&proximoCategoriaId, sizeof(int), 1, arquivo);

    if (numCategorias > 0) {
        fwrite(categorias, sizeof(CategoriaProduto), numCategorias, arquivo);
    }

    fclose(arquivo);
}

void salvarDados() {
    salvarUsuarios();
    salvarClientes();
    salvarProdutos();
    salvarVendas();
    salvarPagamentos();
    salvarCategorias(); 
    
    printf("\n%s >> Todos os dados foram salvos com sucesso!%s\n", verdeSuave, RESET);
}


/*==========================================================================*/
/* 4. IMPLEMENTACAO: UTILS E HELPERS                                        */
/*==========================================================================*/

void limparBuffer() {           
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void limparTela() {             
    system("cls || clear");
}

void pausar() {
    printf("\n%s[ Pressione ENTER para continuar... ]%s", VAMPIRE_BITE, RESET); 
    getchar();      
}

void obterDataAtual(char* dataString) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(dataString, "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
}

int dataEhValida(const char* data) {
    if (strlen(data) != 10) return 0;
    if (data[4] != '-' || data[7] != '-') return 0;
    
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (data[i] < '0' || data[i] > '9') return 0;
    }
    return 1;
}

int compararDatas(const char *data1, const char *data2) {
    int d1, m1, a1;
    int d2, m2, a2;
    
    sscanf(data1, "%d/%d/%d", &d1, &m1, &a1);
    sscanf(data2, "%d/%d/%d", &d2, &m2, &a2);

    if (a1 != a2) return (a1 > a2) ? 1 : -1;
    if (m1 != m2) return (m1 > m2) ? 1 : -1;
    if (d1 != d2) return (d1 > d2) ? 1 : -1;
    
    return 0; 
}

int dataParaInt(const char* dateStr) {
    int day, month, year;
    if (sscanf(dateStr, "%d/%d/%d", &day, &month, &year) != 3) {
        return -1; 
    }
    return year * 10000 + month * 100 + day;
}

int dataEhMaiorOuIgual(const char* dateA, const char* dateB) {
    return dataParaInt(dateA) >= dataParaInt(dateB);
}

int dataEstaNoPeriodo(const char* dataVenda, const char* dataInicio, const char* dataFim) {
    int vendaInt = dataParaInt(dataVenda);
    int inicioInt = dataParaInt(dataInicio);
    int fimInt = dataParaInt(dataFim);

    if (vendaInt == -1 || inicioInt == -1 || fimInt == -1) return 0;

    return (vendaInt >= inicioInt) && (vendaInt <= fimInt);
}

void lerPeriodoDatas(char* dataInicio, char* dataFim) {
    printf("%s----------------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
    printf(" Digite a Data de Inicio (DD/MM/AAAA): ");
    if (scanf("%10s", dataInicio) != 1) strcpy(dataInicio, "01/01/2000");
    limparBuffer();

    printf(" Digite a Data Final   (DD/MM/AAAA): ");
    if (scanf("%10s", dataFim) != 1) strcpy(dataFim, "31/12/2099");
    limparBuffer();
    
    if (dataParaInt(dataInicio) > dataParaInt(dataFim)) { 
        char temp[11];
        strcpy(temp, dataInicio);
        strcpy(dataInicio, dataFim);
        strcpy(dataFim, temp);
        printf("%s >> Aviso: Datas invertidas automaticamente.%s\n", PUMPKIN_PUNCH, RESET);
    }
}

int comparaClientesPorNome(const void *a, const void *b) {
    const Cliente *clienteA = (const Cliente *)a;
    const Cliente *clienteB = (const Cliente *)b;
    return strcmp(clienteA->nomeCompleto, clienteB->nomeCompleto); 
}

int comparaProdutosPorDescricao(const void *a, const void *b) {
    const Produto *produtoA = (const Produto *)a;
    const Produto *produtoB = (const Produto *)b;
    return strcmp(produtoA->descricao, produtoB->descricao);
}


/*==========================================================================*/
/* 5. IMPLEMENTACAO: BUSCAS E VALIDACOES                                    */
/*==========================================================================*/

int buscarUsuarioPorLogin(const char* login) {
    for (int i = 0; i < numUsuarios; i++) {
        if (strcmp(usuarios[i].login, login) == 0) {
            return i; 
        }
    }
    return -1; 
}

int buscarClientePorCodigo(int codigo) {
    for (int i = 0; i < numClientes; i++) {
        if (clientes[i].codigo == codigo) {
            return i; 
        }
    }
    return -1; 
}

int buscarProdutoPorCodigo(int codigo) {
    for (int i = 0; i < numProdutos; i++) {
        if (produtos[i].codigo == codigo) {
            return i; 
        }
    }
    return -1; 
}

int buscarVendaPorID(int id) {
    for (int i = 0; i < numVendas; i++) {
        if (vendas[i].idVenda == id) {
            return i; 
        }
    }
    return -1; 
}

int buscarVendaEmAberto() {
    for (int i = 0; i < numVendas; i++) {
        if (vendas[i].status == 'A') {
            return i;
        }
    }
    return -1;
}

int buscarCategoriaPorId(int id) {
    for (int i = 0; i < numCategorias; i++) {
        if (categorias[i].id == id) {
            return i;
        }
    }
    return -1;
}

int validarAdmin() {
    if (usuarioLogado.tipo != 1) {
        limparTela();
        printf("%s[ACESSO NEGADO] Apenas Administradores podem acessar esta funcao.%s\n", vermelhoErro, RESET);
        pausar();
        return 0; 
    }
    return 1; 
}

int validarUsuarioAdmin() {
    if (numUsuarios > 0 && usuarioLogado.tipo == 1) {
        return 1; 
    }

    limparTela();
    printf("%s=======================================================%s\n", PUMPKIN_PUNCH, RESET);
    printf("%s|  AUTORIZACAO REQUERIDA: CREDENCIAIS DE ADMINISTRADOR  |%s\n", PUMPKIN_PUNCH, RESET);
    printf("%s=======================================================%s\n", PUMPKIN_PUNCH, RESET);
    
    char loginTemp[50];
    char senhaTemp[50];
    int tentativas = 0;
    
    while (tentativas < 3) {
        printf("\n Login Admin: ");
        fgets(loginTemp, 50, stdin);
        loginTemp[strcspn(loginTemp, "\n")] = 0;

        printf(" Senha Admin: ");
        fgets(senhaTemp, 50, stdin);
        senhaTemp[strcspn(senhaTemp, "\n")] = 0;
        
        int idx = buscarUsuarioPorLogin(loginTemp);

        if (idx != -1 && usuarios[idx].tipo == 1 && strcmp(usuarios[idx].senha, senhaTemp) == 0) {
            printf("\n%s >> Autorizacao concedida temporariamente.%s\n", verdeSuave, RESET);
            pausar();
            return 1; 
        }
        
        tentativas++;
        printf("%s >> Credenciais invalidas ou usuario nao e Admin. Tentativa %d/3.%s\n", vermelhoErro, tentativas, RESET);
    }
    
    printf("\n%s[ACESSO NEGADO] Limite de tentativas excedido.%s\n", vermelhoErro, RESET);
    pausar();
    return 0; 
}


/*==========================================================================*/
/* 6. IMPLEMENTACAO: LIFECYCLE DO SISTEMA                                   */
/*==========================================================================*/

void exibirBoasVindas() {
    limparTela();
    printf("%s=========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                                                                       |%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                  BEM VINDO(A) AO MERCADINHO DA DONA BERE              |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s|                                                                       |%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s=========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    pausar();
}

void inicializarSistema() {
    limparTela();
    
    char statusMsg[300];
    int correcaoAplicada = 0;
    int maiorIdVenda = 0;
    
    usuarios = NULL;    
    clientes = NULL;
    produtos = NULL;
    vendas = NULL;
    pagamentos = NULL;
    carrinho = NULL;
    categorias = NULL; 

    carregarDados(); 
    
    for (int i = 0; i < numVendas; i++) {
        if (vendas[i].idVenda > maiorIdVenda) {
            maiorIdVenda = vendas[i].idVenda;
        }

        if (vendas[i].idVenda == 0) {
            vendas[i].idVenda = (maiorIdVenda > 0 ? maiorIdVenda : 0) + 1000 + i; 
            correcaoAplicada = 1;
        }
    }
    
    if (maiorIdVenda >= idContadorVenda) {
        idContadorVenda = maiorIdVenda + 1;
    }
    if (idContadorVenda == 0) idContadorVenda = 1;

    for (int i = 0; i < numClientes; i++) {
        if (clientes[i].codigo == 0) {
            clientes[i].codigo = 100 + i;
            correcaoAplicada = 1;
        }
    }

    for (int i = 0; i < numProdutos; i++) {
        if (produtos[i].codigo == 0) {
            produtos[i].codigo = 1000 + i;
            correcaoAplicada = 1;
        }
    }

    if (correcaoAplicada) {
        printf("%s >> AVISO: IDs invalidos (0) detectados e corrigidos.%s\n", PUMPKIN_PUNCH, RESET);
        salvarDados(); 
    }

    caixaAberto = 0;
    saldoAbertura = 0.0;
    totalRetiradas = 0.0;
    
    snprintf(statusMsg, sizeof(statusMsg),
             "| %sSistema inicializado com sucesso!%s\n"
             "| Total de usuarios: %s%d%s | Vendas carregadas: %s%d%s\n",
             GHOUL_GREEN, RESET, 
             VAMPIRE_BITE, numUsuarios, RESET, 
             VAMPIRE_BITE, numVendas, RESET);

    printf("%s=========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                       INICIALIZACAO DO SISTEMA                        |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s=========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    
    printf("%s", statusMsg);
    
    printf("%s=========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    pausar();
}

void liberarMemoria() {
    free(produtos);
    free(clientes);
    free(carrinho);
    free(usuarios);
    free(categorias);
    
    for (int i = 0; i < numVendas; i++) {
        free(vendas[i].itens);
    }
    free(vendas);
    free(pagamentos);

    produtos = NULL;
    clientes = NULL;
    carrinho = NULL;
    usuarios = NULL;
    vendas = NULL;
    pagamentos = NULL;
    categorias = NULL;
}

void finalizarSistema() {
    liberarMemoria();
    printf("\n%s >> Sistema encerrado com seguranca. Memoria liberada.%s\n", verdeSuave, RESET);
}


/*==========================================================================*/
/* 7. IMPLEMENTACAO: AUTENTICACAO E USUARIOS                                */
/*==========================================================================*/

void cadastrarUsuario() {
    limparTela();
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|             CADASTRO DE NOVO USUARIO           |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (numUsuarios >= 100) {
        printf("%s[ERRO] Limite maximo de usuarios atingido (100).%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }

    Usuario novoUsuario;
    int idx;
    char tempLogin[50];
    char tempSenha[50];

    do {
        printf("\n %sInforme o Login (8 a 12 caracteres):%s ", VAMPIRE_BITE, RESET);
        fgets(tempLogin, 50, stdin);
        tempLogin[strcspn(tempLogin, "\n")] = 0; 

        idx = buscarUsuarioPorLogin(tempLogin);
        if (idx != -1) {
            printf("%s[ERRO] Login ja existe. Tente outro nome.%s\n", vermelhoErro, RESET);
            continue; 
        }
        
        size_t len = strlen(tempLogin);
        if (len < 8 || len > 12) {
            printf("%s[ERRO] O Login deve ter entre 8 e 12 caracteres. (Atual: %zu)%s\n", vermelhoErro, len, RESET);
        } else {
            strcpy(novoUsuario.login, tempLogin);
            break; 
        }
    } while (1); 

    do {
        printf(" %sInforme a Senha (6 a 8 caracteres):%s ", VAMPIRE_BITE, RESET);
        fgets(tempSenha, 50, stdin);
        tempSenha[strcspn(tempSenha, "\n")] = 0; 
        
        size_t len = strlen(tempSenha);
        if (len < 6 || len > 8) {
            printf("%s[ERRO] A Senha deve ter entre 6 e 8 caracteres. (Atual: %zu)%s\n", vermelhoErro, len, RESET);
        } else {
            strcpy(novoUsuario.senha, tempSenha);
            break; 
        }
    } while (1);

    if (numUsuarios == 0) {
        printf("\n%s >> Primeiro cadastro detectado: Definindo como ADMINISTRADOR.%s\n", PUMPKIN_PUNCH, RESET);
        novoUsuario.tipo = 1;
    } else {
        do {
            printf(" %sInforme o Tipo (1 - Admin | 2 - Padrao):%s ", VAMPIRE_BITE, RESET);
            if (scanf("%d", &novoUsuario.tipo) != 1) {
                limparBuffer();
                printf("%s[ERRO] Entrada invalida. Use 1 ou 2.%s\n", vermelhoErro, RESET);
            } else if (novoUsuario.tipo != 1 && novoUsuario.tipo != 2) {
                printf("%s[ERRO] Tipo invalido. Use 1 para Admin ou 2 para Padrao.%s\n", vermelhoErro, RESET);
            } else {
                break; 
            }
        } while (1);
        limparBuffer(); 
    }

    Usuario* tempUsuarios = realloc(usuarios, (numUsuarios + 1) * sizeof(Usuario));
    
    if (tempUsuarios == NULL) {
        printf("%s[ERRO] Falha na alocacao de memoria para o novo usuario.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    
    usuarios = tempUsuarios; 
    usuarios[numUsuarios] = novoUsuario;
    numUsuarios++;

    printf("\n%s >> Usuario '%s' (Tipo: %s) cadastrado com sucesso!%s\n", 
           verdeSuave, novoUsuario.login, 
           (novoUsuario.tipo == 1 ? "Admin" : "Padrao"), 
           RESET);
           
    pausar();
}

int login() {
    char loginInput[50];
    char senhaInput[50];
    int i;
    
    limparTela();
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                  FAZER LOGIN                   |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (numUsuarios == 0) {
        printf("%s[ATENCAO] Nenhum usuario cadastrado.%s\n", vermelhoErro, RESET);
        printf("Retorne ao menu inicial e use a opcao 'Cadastrar Primeiro Admin'.\n");
        pausar();
        return 0;
    }
    
    printf(" %sLogin:%s ", CIANO, RESET);
    fgets(loginInput, 50, stdin);
    loginInput[strcspn(loginInput, "\n")] = 0; 
    
    printf(" %sSenha:%s ", CIANO, RESET);
    fgets(senhaInput, 50, stdin);
    senhaInput[strcspn(senhaInput, "\n")] = 0;
    
    for (i = 0; i < numUsuarios; i++) {
        if (strcmp(usuarios[i].login, loginInput) == 0 && 
            strcmp(usuarios[i].senha, senhaInput) == 0) 
        {
            usuarioLogado = usuarios[i]; 
            printf("\n%s >> Login bem-sucedido! Bem-vindo(a), %s.%s\n", verdeSuave, usuarioLogado.login, RESET);
            pausar();
            return 1; 
        }
    }
    
    printf("\n%s[ERRO] Login ou senha incorretos.%s\n", vermelhoErro, RESET);
    pausar();
    return 0; 
}


/*==========================================================================*/
/* 8. IMPLEMENTACAO: CATEGORIAS                                             */
/*==========================================================================*/

void cadastrarCategoriaPadrao(const char* nome) {
    CategoriaProduto* temp = realloc(categorias, (numCategorias + 1) * sizeof(CategoriaProduto));
    if (temp == NULL) {
        return; 
    }
    categorias = temp;

    categorias[numCategorias].id = proximoCategoriaId++;
    strncpy(categorias[numCategorias].nome, nome, 50);
    categorias[numCategorias].nome[50] = '\0';
    numCategorias++;
}

void cadastrarCategoria() {
    char buffer[100]; 
    int existe;

    limparTela();
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|            CADASTRO DE NOVA CATEGORIA          |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (proximoCategoriaId > 9999) { 
        printf("%s[ERRO] Limite de IDs de categoria atingido.\n%s", vermelhoErro, RESET);
        pausar();
        return;
    }

    do {
        printf(" %sNome da Categoria (max. 50 caracteres):%s ", VAMPIRE_BITE, RESET);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) return; 
        
        buffer[strcspn(buffer, "\n")] = 0; 
        
        if (strlen(buffer) == 0) {
            printf("%s[ERRO] O nome da categoria nao pode ser vazio.%s\n", vermelhoErro, RESET);
            continue;
        }

        if (strlen(buffer) > 50) {
            printf("%s[ERRO] O nome da categoria deve ter no maximo 50 caracteres. (Atual: %zu)%s\n", 
                   vermelhoErro, strlen(buffer), RESET);
            continue;
        }
        
        existe = 0;
        for (int i = 0; i < numCategorias; i++) {
            if (strcmp(categorias[i].nome, buffer) == 0) {
                existe = 1;
                break;
            }
        }
        
        if (existe) {
             printf("%s[ERRO] Categoria '%s' ja existe. Escolha outro nome.%s\n", vermelhoErro, buffer, RESET);
             continue;
        }
        break; 

    } while (1);
    
    CategoriaProduto* temp = realloc(categorias, (numCategorias + 1) * sizeof(CategoriaProduto));
    if (temp == NULL) {
        printf("%s[ERRO] Alocacao de memoria falhou!%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    categorias = temp;

    categorias[numCategorias].id = proximoCategoriaId++;
    strncpy(categorias[numCategorias].nome, buffer, 50);
    categorias[numCategorias].nome[50] = '\0'; 
    numCategorias++;
    
    printf("\n%s >> Categoria '%s' (ID: %d) cadastrada com sucesso!%s\n", 
           verdeSuave, categorias[numCategorias - 1].nome, categorias[numCategorias - 1].id, RESET);
    pausar();
}

void listarCategorias() {
    printf("\n%s--- CATEGORIAS DISPONIVEIS (%d) ---%s\n", PUMPKIN_PUNCH, numCategorias, RESET);
    if (numCategorias == 0) {
        printf(" Nenhuma categoria cadastrada.\n");
        return;
    }
    for (int i = 0; i < numCategorias; i++) {
        printf(" ID: %s%d%s | Nome: %s\n", VAMPIRE_BITE, categorias[i].id, RESET, categorias[i].nome);
    }
    printf("%s----------------------------------%s\n", PUMPKIN_PUNCH, RESET);
}

void exibirCategorias() {
    if (numCategorias == 0) {
        printf("%sNenhuma categoria cadastrada. E necessario cadastrar uma categoria.%s\n", AMARELO, RESET);
        return;
    }
    printf("%s--- CATEGORIAS CADASTRADAS ---%s\n", MIDNIGHT_HOWL, RESET);
    for (int i = 0; i < numCategorias; i++) {
        printf(" ID: %d | Nome: %s\n", categorias[i].id, categorias[i].nome);
    }
    printf("--------------------------------\n");
}


/*==========================================================================*/
/* 9. IMPLEMENTACAO: CLIENTES (CRUD)                                        */
/*==========================================================================*/

void cadastrarCliente() {
    int opcao;

    do{
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|             CADASTRO DE CLIENTES               |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s| 1. Cadastro Cliente                            |%s\n", CIANO, RESET);
        printf("%s| 2. Voltar ao Menu                              |%s\n", CIANO, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s Digite a opcao: %s", VAMPIRE_BITE, RESET);
        
        if (scanf("%d", &opcao) != 1) {
            limparBuffer(); opcao = 0;
        } else {
            limparBuffer(); 
        }

        switch (opcao){
            case 1:
                limparTela();
                printf("%s--- NOVO CLIENTE ---%s\n", CIANO, RESET);

                Cliente* temp = realloc(clientes, (numClientes + 1) * sizeof(Cliente));
                if(temp == NULL){
                    printf("%s[ERRO] Memoria insuficiente!%s\n", vermelhoErro, RESET);
                    pausar();
                    continue;
                }
                clientes = temp;

                clientes[numClientes].codigo = 100 + numClientes;

                printf(" %sNome completo:%s ", VAMPIRE_BITE, RESET);
                fgets(clientes[numClientes].nomeCompleto, 100, stdin);
                clientes[numClientes].nomeCompleto[strcspn(clientes[numClientes].nomeCompleto, "\n")] = 0;
                
                printf(" %sNome social:%s ", VAMPIRE_BITE, RESET);
                fgets(clientes[numClientes].nomeSocial, 100, stdin);
                clientes[numClientes].nomeSocial[strcspn(clientes[numClientes].nomeSocial, "\n")] = 0;
                
                printf(" %sCPF:%s ", VAMPIRE_BITE, RESET);
                fgets(clientes[numClientes].cpf, 15, stdin);
                clientes[numClientes].cpf[strcspn(clientes[numClientes].cpf, "\n")] = 0;
                
                printf(" %sRua e numero:%s ", VAMPIRE_BITE, RESET);
                fgets(clientes[numClientes].ruaNumero, 100, stdin);
                clientes[numClientes].ruaNumero[strcspn(clientes[numClientes].ruaNumero, "\n")] = 0;
                
                printf(" %sBairro:%s ", VAMPIRE_BITE, RESET);
                fgets(clientes[numClientes].bairro, 100, stdin);
                clientes[numClientes].bairro[strcspn(clientes[numClientes].bairro, "\n")] = 0;
                
                printf(" %sTelefone:%s ", VAMPIRE_BITE, RESET);
                fgets(clientes[numClientes].celularWhats, 16, stdin);
                clientes[numClientes].celularWhats[strcspn(clientes[numClientes].celularWhats, "\n")] = 0;

                if (strlen(clientes[numClientes].nomeCompleto) == 0 || strlen(clientes[numClientes].cpf) == 0) {
                    printf("%s[ERRO] Nome completo e CPF sao obrigatorios!%s\n", vermelhoErro, RESET);
                    pausar();
                    return; 
                }

                for (int i = 0; i < numClientes; i++) {
                    if (strcmp(clientes[i].cpf, clientes[numClientes].cpf) == 0) {
                        printf("%s[ERRO] Cliente com este CPF ja cadastrado!%s\n", vermelhoErro, RESET);
                        pausar();
                        return; 
                    }
                }

                numClientes++;
                printf("\n%s >> Cliente cadastrado! Codigo: %d%s\n", verdeSuave, clientes[numClientes - 1].codigo, RESET);
                pausar();
                break;

            case 2:
                break; 

            default:
                printf("%s[ERRO] Opcao invalida!\n%s", vermelhoErro, RESET);
                pausar();
                break; 
        }
    }while (opcao != 2);
}

void excluirCliente() {
    int codCliente;
    int indiceCliente = -1;
    char confirmacao;

    limparTela();

    if (numClientes == 0) {
        printf("%sNao ha clientes cadastrados para excluir.\n%s", vermelhoErro, RESET);
        pausar();
        return;
    }

    printf("%s--- LISTA DE CLIENTES ---%s\n", CIANO, RESET); 
    for (int i = 0; i < numClientes; i++) { 
        printf(" Codigo: %s%d%s | Nome: %s\n", VAMPIRE_BITE, clientes[i].codigo, RESET, clientes[i].nomeCompleto); 
    }
    printf("%s-------------------------%s\n", CIANO, RESET);
    printf("\n%s--- EXCLUIR CLIENTE ---%s\n", CIANO, RESET);
    printf("%sDigite o codigo do cliente a EXCLUIR (ou 0 para cancelar): %s", VAMPIRE_BITE, RESET);
    
    if (scanf("%d", &codCliente) != 1) {
        limparBuffer(); codCliente = 0; 
    } else {
        limparBuffer();
    }

    if (codCliente == 0) {
        printf("%s >> Exclusao cancelada.\n%s", PUMPKIN_PUNCH, RESET);
        pausar();
        return;
    }

    for (int i = 0; i < numClientes; i++) {
        if (clientes[i].codigo == codCliente) {
            indiceCliente = i;
            break;
        }
    }

    if (indiceCliente == -1) {
        printf("\n%s[ERRO] Cliente com codigo %d nao encontrado!%s\n",vermelhoErro, codCliente, RESET);
    } else {
        printf("\n%s--- Cliente Encontrado ---%s\n", CIANO, RESET);
        printf(" Codigo: %d | Nome: %s\n CPF: %s\n",
               clientes[indiceCliente].codigo, clientes[indiceCliente].nomeCompleto, clientes[indiceCliente].cpf);
        printf("%s--------------------------%s\n", CIANO, RESET);
        
        printf("%sTem certeza que deseja excluir? (s/n): %s", VAMPIRE_BITE, RESET);
        scanf(" %c", &confirmacao);
        limparBuffer();

        if (confirmacao == 's' || confirmacao == 'S') {
            for (int i = indiceCliente; i < numClientes - 1; i++) {
                clientes[i] = clientes[i + 1];
            }
            numClientes--;

            if (numClientes == 0){ 
                free(clientes);
                clientes = NULL;
            }
            printf("%s\n >> Cliente excluido com sucesso!\n%s", verdeSuave, RESET);
        } else {
            printf("%s\n >> Exclusao cancelada.\n%s", PUMPKIN_PUNCH, RESET);
        }
    }
    pausar();
}

void listarClientesSimples() {
    if (numClientes == 0) {
        printf("%s[INFO] Nenhum cliente cadastrado.%s\n", AMARELO, RESET);
        return;
    }
    
    printf("\n%s=======================================================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s| %-6s | %-25s | %-14s | %-15s | %-30s |%s\n", CIANO, "CODIGO", "NOME COMPLETO", "CPF", "CELULAR", "ENDERECO PRINCIPAL", RESET);
    printf("%s|--------|---------------------------|----------------|-----------------|--------------------------------|%s\n", MIDNIGHT_HOWL, RESET);

    for (int i = 0; i < numClientes; i++) {
        char endereco[47]; 
        snprintf(endereco, sizeof(endereco), "%s, %s", clientes[i].ruaNumero, clientes[i].bairro);

        printf("| %-6d | %-25s | %-14s | %-15s | %-30s |\n",
               clientes[i].codigo,
               clientes[i].nomeCompleto,
               clientes[i].cpf,
               clientes[i].celularWhats,
               endereco);
    }
    printf("%s=======================================================================================================================%s\n", MIDNIGHT_HOWL, RESET);
}

void listaClientes() {
    limparTela(); 
    printf("%s============================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                                   LISTA DE CLIENTES                                      |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s============================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s| %-6s | %-30s | %-14s | %-20s |%s\n", CIANO, "CODIGO", "NOME COMPLETO", "CPF", "CELULAR", RESET);
    printf("%s|--------|--------------------------------|----------------|----------------------|%s\n", MIDNIGHT_HOWL, RESET);
    if (numClientes == 0) {
        printf("|                            Nenhum cliente cadastrado.                                    |\n");
    } else {
        for (int i = 0; i < numClientes; i++) {
            printf("| %-6d | %-30s | %-14s | %-20s |\n",
                   clientes[i].codigo,
                   clientes[i].nomeCompleto,
                   clientes[i].cpf,
                   clientes[i].celularWhats);
        }
    }
    printf("%s============================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    pausar();
}


/*==========================================================================*/
/* 10. IMPLEMENTACAO: PRODUTOS E ESTOQUE (CRUD)                             */
/*==========================================================================*/

void cadastrarProduto() {
    int opcao;
    int indiceCategoria = -1; 
    int idSelecionado;
    
    Produto produtoAtual; 

    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|             CADASTRO DE PRODUTOS               |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s| 1. Cadastrar Novo Produto                      |%s\n", CIANO, RESET);
        printf("%s| 2. Voltar ao Menu                              |%s\n", CIANO, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s Digite a opcao: %s", VAMPIRE_BITE, RESET);
        
        if (scanf("%d", &opcao) != 1) {
            limparBuffer(); opcao = 0;
        } else {
            limparBuffer();
        }

        switch (opcao) {
            case 1: {
                limparTela();
                printf("%s--- NOVO PRODUTO ---%s\n", CIANO, RESET);

                printf(" %sDescricao do produto:%s ", VAMPIRE_BITE, RESET);
                fgets(produtoAtual.descricao, 100, stdin);
                produtoAtual.descricao[strcspn(produtoAtual.descricao, "\n")] = 0;

                for (int i = 0; i < numProdutos; i++) {
                    if (strcmp(produtos[i].descricao, produtoAtual.descricao) == 0) {
                        printf("%s[ERRO] Produto '%s' ja cadastrado!%s\n", vermelhoErro, produtoAtual.descricao, RESET);
                        pausar();
                        continue; 
                    }
                }
                
                if (numCategorias == 0) {
                    printf("%s[ERRO] Nenhuma categoria cadastrada! Volte ao Menu Cadastros.%s\n", vermelhoErro, RESET);
                    pausar();
                    continue; 
                }

                do { 
                    listarCategorias(); 
                    
                    printf(" %sDigite o ID da Categoria:%s ", VAMPIRE_BITE, RESET);
                    if (scanf("%d", &idSelecionado) != 1) {
                        limparBuffer(); 
                        idSelecionado = -1;
                    } else {
                        limparBuffer();
                    }

                    indiceCategoria = buscarCategoriaPorId(idSelecionado); 
                    
                    if (indiceCategoria == -1) {
                        printf("%s[ERRO] Categoria invalida! Tente novamente.\n%s", vermelhoErro, RESET);
                        pausar();
                    }
                    
                } while (indiceCategoria == -1); 
                
                Produto* temp = (Produto*)realloc(produtos, (numProdutos + 1) * sizeof(Produto));
                if (temp == NULL) { 
                    printf("%s[ERRO] Falha de alocacao de memoria!%s\n", vermelhoErro, RESET);
                    pausar();
                    continue; 
                }
                produtos = temp;
                
                produtos[numProdutos].codigo = 1000 + numProdutos;
                strcpy(produtos[numProdutos].descricao, produtoAtual.descricao);
                strcpy(produtos[numProdutos].categoria, categorias[indiceCategoria].nome);

                
                printf(" %sPreco de custo: R$ %s", VAMPIRE_BITE, RESET);
                if (scanf("%f", &produtos[numProdutos].precoCompra) != 1 || produtos[numProdutos].precoCompra <= 0) {
                    limparBuffer(); 
                    printf("%s[ERRO] Preco de custo deve ser maior que zero.%s\n", vermelhoErro, RESET);
                    pausar();
                    return; 
                }
                limparBuffer();

                printf(" %sMargem de lucro: (%%) %s", VAMPIRE_BITE, RESET);
                if (scanf("%f", &produtos[numProdutos].margemLucro) != 1 || produtos[numProdutos].margemLucro < 0) {
                    limparBuffer(); 
                    printf("%s[ERRO] Margem de lucro nao pode ser negativa.%s\n", vermelhoErro, RESET);
                    pausar();
                    return; 
                }
                limparBuffer();

                produtos[numProdutos].precoVenda = produtos[numProdutos].precoCompra * (1 + produtos[numProdutos].margemLucro / 100.0);
                printf(" %s>> Preco de venda calculado: R$ %.2f %s\n\n", verdeSuave, produtos[numProdutos].precoVenda, RESET);
                
                printf(" %sQuantidade em estoque: %s", VAMPIRE_BITE, RESET);
                if (scanf("%d", &produtos[numProdutos].quantidadeEstoque) != 1 || produtos[numProdutos].quantidadeEstoque < 0) {
                    limparBuffer(); 
                    printf("%s[ERRO] Quantidade nao pode ser negativa.%s\n", vermelhoErro, RESET);
                    pausar();
                    return; 
                }
                limparBuffer();

                printf(" %sEstoque minimo: %s", VAMPIRE_BITE, RESET);
                if (scanf("%d", &produtos[numProdutos].estoqueMinimo) != 1 || produtos[numProdutos].estoqueMinimo < 0) {
                    limparBuffer(); 
                    printf("%s[ERRO] Estoque minimo nao pode ser negativo.%s\n", vermelhoErro, RESET);
                    pausar();
                    return; 
                }
                limparBuffer();

                numProdutos++;
                printf("\n%s >> Produto cadastrado! Codigo: %d %s\n", verdeSuave, produtos[numProdutos - 1].codigo, RESET);
                pausar();
                break;
            }

            case 2:
                break; 

            default:
                printf("%sOpcao invalida!\n%s", vermelhoErro, RESET);
                pausar();
                break; 
        } 
    } while (opcao != 2); 
}

void excluirProduto() {
    int codProduto;
    int indiceProduto = -1;
    char confirmacao;

    limparTela();
    if (numProdutos == 0) {
        printf("%sNao ha produtos cadastrados para excluir.\n%s", vermelhoErro, RESET);
        pausar();
        return;
    }
    printf("%s--- EXCLUIR PRODUTO ---%s\n", CIANO, RESET);
    listaProdutos(); 
    printf("%s Digite o codigo do produto a EXCLUIR (ou 0 para cancelar): %s", VAMPIRE_BITE, RESET);
    
    if (scanf("%d", &codProduto) != 1) {
        limparBuffer(); codProduto = 0;
    } else {
        limparBuffer();
    }

    if (codProduto == 0) {
        printf("%s >> Exclusao cancelada.\n%s", PUMPKIN_PUNCH, RESET);
        pausar();
        return;
    }

    for (int i = 0; i < numProdutos; i++) {
        if (produtos[i].codigo == codProduto) {
            indiceProduto = i;
            break;
        }
    }

    if (indiceProduto == -1) { 
        printf("\n%s[ERRO] Produto com codigo %d nao encontrado!%s\n",vermelhoErro, codProduto, RESET);
    } else {
        printf("%s\n--- Produto Encontrado ---\n%s", CIANO, RESET);
        printf(" Codigo: %d | Descricao: %s | Estoque: %d\n",
               produtos[indiceProduto].codigo, produtos[indiceProduto].descricao, produtos[indiceProduto].quantidadeEstoque);
        printf("%s--------------------------%s\n", CIANO, RESET);
        
        printf("%sTem certeza que deseja excluir? (s/n): %s", VAMPIRE_BITE, RESET);
        scanf(" %c", &confirmacao);
        limparBuffer();

        if (confirmacao == 's' || confirmacao == 'S') {
            for (int i = indiceProduto; i < numProdutos - 1; i++) {
                produtos[i] = produtos[i + 1]; 
            }
            numProdutos--;

            if (numProdutos == 0) {
                free(produtos);
                produtos = NULL;
            }

            printf("%s\n >> Produto excluido com sucesso!\n%s", verdeSuave, RESET);
        } else {
            printf("%s\n >> Exclusao cancelada.\n%s", PUMPKIN_PUNCH, RESET);
        }
    }
    pausar();
}

void adicionarEstoque() {
    int codProduto;
    int indiceProduto = -1;
    int quantidadeAdicionar;

    limparTela();
    if (numProdutos == 0) {
        printf("%sNenhum produto cadastrado.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    
    listaProdutos(); 
    printf("\n%s--- ADICIONAR ESTOQUE ---%s\n", CIANO, RESET);
    printf(" %sDigite o codigo do produto (ou 0 para cancelar):%s ", VAMPIRE_BITE, RESET);
    
    if (scanf("%d", &codProduto) != 1) {
        limparBuffer(); codProduto = 0;
    } else {
        limparBuffer();
    }

    if (codProduto == 0) {
        printf("%s >> Operacao cancelada.\n%s", PUMPKIN_PUNCH, RESET);
        pausar();
        return;
    }

    for (int i = 0; i < numProdutos; i++) {
        if (produtos[i].codigo == codProduto) {
            indiceProduto = i;
            break;
        }
    }

    if (indiceProduto == -1) {
        printf("%s[ERRO] Produto com codigo %d nao encontrado!%s\n",vermelhoErro, codProduto,RESET);
    } else {
        printf(" Produto: %s%s%s | Estoque atual: %s%d%s\n", CIANO, produtos[indiceProduto].descricao, RESET, CIANO, produtos[indiceProduto].quantidadeEstoque, RESET);
        printf(" %sDigite a quantidade a ADICIONAR:%s ", VAMPIRE_BITE, RESET);
        
        if (scanf("%d", &quantidadeAdicionar) != 1) {
            limparBuffer(); quantidadeAdicionar = 0;
        } else {
            limparBuffer();
        }

        if (quantidadeAdicionar <= 0) {
            printf("%s[ERRO] Quantidade invalida. Deve ser positiva.\n%s", vermelhoErro, RESET);
        } else {
            produtos[indiceProduto].quantidadeEstoque += quantidadeAdicionar;
            printf("%s >> Estoque atualizado! Novo estoque de '%s': %d%s\n", verdeSuave, produtos[indiceProduto].descricao, produtos[indiceProduto].quantidadeEstoque, RESET);
        }
    }
    pausar();
}

void listaProdutos() {
    limparTela();
    printf("%s========================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                                 LISTA DE PRODUTOS                                    |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s========================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s| %-6s | %-25s | %-15s | %-12s | %-7s |%s\n", CIANO, "CODIGO", "DESCRICAO", "CATEGORIA", "PRECO", "ESTOQUE", RESET);
    printf("%s|--------|---------------------------|-----------------|--------------|---------|%s\n", MIDNIGHT_HOWL, RESET);
    
    if (numProdutos == 0) {
        printf("|                        Nenhum produto cadastrado.                             |\n");
    } else { 
        for (int i = 0; i < numProdutos; i++) {
            const char* cor = (produtos[i].quantidadeEstoque <= produtos[i].estoqueMinimo) ? vermelhoErro : RESET;
            printf("| %s%-6d%s | %-25s | %-15s | R$ %9.2f | %-7d |\n",
                   cor, 
                   produtos[i].codigo, RESET, produtos[i].descricao, produtos[i].categoria,
                   produtos[i].precoVenda, produtos[i].quantidadeEstoque);
        }
    }
    printf("%s========================================================================================%s\n", MIDNIGHT_HOWL, RESET);
}

void atualizarEstoqueArquivoDat() {
    int codProduto;
    int qtdAdicionar;
    Produto produto;
    FILE *cfPtr;

    if ((cfPtr = fopen("produtos.dat", "rb+")) == NULL) {
        printf("%s[ERRO] O arquivo produtos.dat nao pode ser aberto.\n%s", vermelhoErro, RESET);
        pausar();
        return;
    }

    limparTela();
    printf("%s--- Atualizacao Rapida (Direto no Disco) ---%s\n", CIANO, RESET);
    printf(" %sDigite o codigo do produto a atualizar:%s ", VAMPIRE_BITE, RESET);
    if (scanf("%d", &codProduto) != 1) {
        limparBuffer(); fclose(cfPtr); return;
    }
    limparBuffer();
    
    long pos = ftell(cfPtr); 
    
    while (fread(&produto, sizeof(Produto), 1, cfPtr) == 1) {
        if (produto.codigo == codProduto) {
            printf(" Produto: %s | Estoque atual: %d\n", produto.descricao, produto.quantidadeEstoque);
            printf(" %sQuantidade a ADICIONAR:%s ", VAMPIRE_BITE, RESET);
            
            if (scanf("%d", &qtdAdicionar) != 1 || qtdAdicionar <= 0) {
                limparBuffer();
                printf("%s[ERRO] Quantidade invalida.%s\n", vermelhoErro, RESET);
                fclose(cfPtr);
                return;
            }
            limparBuffer();

            produto.quantidadeEstoque += qtdAdicionar;

            fseek(cfPtr, pos, SEEK_SET);

            fwrite(&produto, sizeof(Produto), 1, cfPtr);

            printf("%s >> Estoque atualizado no arquivo .dat! Novo: %d%s\n", verdeSuave, produto.quantidadeEstoque, RESET);
            printf("%s(Nota: Reinicie o sistema para ver a alteracao)%s\n", PUMPKIN_PUNCH, RESET);
            
            fclose(cfPtr);
            pausar();
            return;
        }
        pos = ftell(cfPtr); 
    }

    printf("%s[ERRO] Produto %d nao encontrado no arquivo.%s\n", vermelhoErro, codProduto, RESET);
    fclose(cfPtr);
    pausar();
}


/*==========================================================================*/
/* 11. IMPLEMENTACAO: VENDAS E CARRINHO                                     */
/*==========================================================================*/

void listarCarrinhoAtual() {
    float totalGeral = 0.0;
    
    printf("\n%s==============================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                      CARRINHO ATUAL                        |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s==============================================================%s\n", MIDNIGHT_HOWL, RESET);
    
    if (itensNoCarrinho == 0) {
        printf("|                     %sCARRINHO VAZIO%s                         |\n", PUMPKIN_PUNCH, RESET);
        printf("%s==============================================================%s\n", MIDNIGHT_HOWL, RESET);
        return;
    }
    
    printf("| %-5s | %-20s | %-5s | %-9s | %-9s |\n", "COD", "DESCRICAO", "QTD", "UNIT", "TOTAL");
    printf("%s|-------|----------------------|-------|-----------|-----------|%s\n", MIDNIGHT_HOWL, RESET);
    
    for (int i = 0; i < itensNoCarrinho; i++) {
        totalGeral += carrinho[i].totalItem;
        
        printf("| %-5d | %-20s | %-5d | %9.2f | %9.2f |\n", 
            carrinho[i].produtoCodigo,
            carrinho[i].descricao,
            carrinho[i].quantidade,
            carrinho[i].precoUnitario,
            carrinho[i].totalItem);
    }
    
    printf("%s|------------------------------------------------------------|%s\n", MIDNIGHT_HOWL, RESET);
    printf("| %-45s %sR$ %9.2f%s |\n", "TOTAL GERAL:", VERDE, totalGeral, RESET);
    printf("%s==============================================================%s\n", MIDNIGHT_HOWL, RESET);
    totalCarrinho = totalGeral; 
}

void novaVenda() {
    int codigoProduto, quantidade, idxProduto;
    int codigoCliente = -1; 
    int idxCliente = -1;
    char continuar = 's';
    float totalVenda = 0.0; 
    
    if (caixaAberto == 0) {
        printf("%s[ERRO] E necessario abrir o Caixa antes de vender.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    
    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|       NOVA VENDA - SELECAO DE CLIENTE          |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);

        if (numClientes == 0) {
            char opcaoCad;
            printf("\n%s[ALERTA] NENHUM CLIENTE CADASTRADO.%s\n", vermelhoErro, RESET);
            printf("Para prosseguir, e necessario vincular um cliente.\n");
            printf("%sDeseja cadastrar um novo cliente agora? (s/n): %s", PUMPKIN_PUNCH, RESET);
            
            if (scanf(" %c", &opcaoCad) != 1) {
                limparBuffer(); opcaoCad = 'n';
            }
            limparBuffer();

            if (opcaoCad == 's' || opcaoCad == 'S') {
                cadastrarCliente(); 
                
                if (numClientes == 0) {
                    printf("%sCadastro cancelado. Venda encerrada.%s\n", PUMPKIN_PUNCH, RESET);
                    pausar();
                    return;
                }
                continue; 
            } else {
                printf("%sVenda cancelada por falta de cliente.%s\n", PUMPKIN_PUNCH, RESET);
                pausar();
                return;
            }
        } 

        printf("\n%s--- CLIENTES DISPONIVEIS ---%s\n", CIANO, RESET);
        listarClientesSimples(); 
        
        printf("\n%s[INFO] Nenhum cliente vinculado a compra!%s\n", vermelhoErro, RESET);
        printf("--------------------------------------------------\n");
        printf(" %sInforme o Codigo do Cliente (ou 0 para VOLTAR):%s ", VAMPIRE_BITE, RESET);
        
        if (scanf("%d", &codigoCliente) != 1) {
            limparBuffer();
            codigoCliente = -1; 
        }
        limparBuffer();

        if (codigoCliente == 0) {
            printf("%sVenda cancelada pelo usuario.%s\n", PUMPKIN_PUNCH, RESET);
            pausar();
            return; 
        }
        
        if (codigoCliente < 0) {
             printf("%s[ERRO] Codigo invalido.%s\n", vermelhoErro, RESET);
             pausar();
             continue;
        }

        idxCliente = buscarClientePorCodigo(codigoCliente);
        
        if (idxCliente == -1) {
            printf("%s[ERRO] Cliente %d nao encontrado!%s\n", vermelhoErro, codigoCliente, RESET);
            pausar();
        } else {
            printf("\n%s--- CLIENTE ENCONTRADO ---%s\n", verdeSuave, RESET);
            printf(" Nome: %s%s%s | CPF: %s\n", PUMPKIN_PUNCH, clientes[idxCliente].nomeCompleto, RESET, clientes[idxCliente].cpf);

            char confirma;
            printf(" Confirmar este cliente? (s/n): ");
            if (scanf(" %c", &confirma) == 1 && (confirma == 's' || confirma == 'S')) {
                limparBuffer();
                printf("%sCliente vinculado com sucesso!%s\n", verdeSuave, RESET);
                pausar();
                break; 
            } else {
                limparBuffer();
                printf("%sVinculacao cancelada.%s\n", PUMPKIN_PUNCH, RESET);
                codigoCliente = -1; 
                pausar();
            }
        }
        
    } while (1);

    itensNoCarrinho = 0;
    totalCarrinho = 0.0; 
    if (carrinho != NULL) free(carrinho); 
    carrinho = (ItemVenda*)malloc(0); 
    
    do {
        limparTela();
        printf("%s=== TELA DE VENDAS - ID %d ===%s\n", AZUL, idContadorVenda, RESET);
        printf("%sCLIENTE: %s (Cod: %d)%s\n", CIANO, clientes[idxCliente].nomeCompleto, codigoCliente, RESET);
        
        printf("\n%s--- PRODUTOS DISPONIVEIS ---%s\n", CIANO, RESET);
        listaProdutos(); 
        
        listarCarrinhoAtual(); 
        
        printf("\n %sInforme codigo do produto (ou 0 para finalizar):%s ", VAMPIRE_BITE, RESET);
        if (scanf("%d", &codigoProduto) != 1) {
            limparBuffer();
            codigoProduto = -1; 
        }
        limparBuffer();

        if (codigoProduto == 0) {
            break; 
        }

        idxProduto = buscarProdutoPorCodigo(codigoProduto);

        if (idxProduto == -1) {
            printf("%s[ERRO] Produto %d nao encontrado.%s\n", vermelhoErro, codigoProduto, RESET);
            pausar();
            continue;
        }

        if (produtos[idxProduto].quantidadeEstoque == 0) {
            printf("%s[ALERTA] Produto esgotado!%s\n", vermelhoErro, RESET);
            pausar();
            continue;
        }

        printf(" Selecionado: %s%s%s (R$ %.2f) | Estoque: %d\n", 
                PUMPKIN_PUNCH, produtos[idxProduto].descricao, RESET, produtos[idxProduto].precoVenda, produtos[idxProduto].quantidadeEstoque);

        printf(" %sInforme a quantidade:%s ", VAMPIRE_BITE, RESET);
        if (scanf("%d", &quantidade) != 1 || quantidade <= 0) {
            limparBuffer();
            printf("%s[ERRO] Quantidade invalida.%s\n", vermelhoErro, RESET);
            pausar();
            continue;
        }
        limparBuffer();
        
        if (quantidade > produtos[idxProduto].quantidadeEstoque) {
            char confirmar;
            printf("%s[ALERTA] Estoque insuficiente! (Solicitado: %d, Disponivel: %d)%s\n", 
                    PUMPKIN_PUNCH, quantidade, produtos[idxProduto].quantidadeEstoque, RESET);
            printf(" Continuar mesmo assim? (s/n): ");
            
            if (scanf(" %c", &confirmar) != 1 || (confirmar != 's' && confirmar != 'S')) {
                limparBuffer();
                printf("%sCancelado.%s\n", PUMPKIN_PUNCH, RESET);
                pausar();
                continue; 
            }
            limparBuffer();
        }
        
        int estoqueRestante = produtos[idxProduto].quantidadeEstoque - quantidade;
        if (estoqueRestante <= produtos[idxProduto].estoqueMinimo) {
            printf("%s[AVISO] Estoque ficara abaixo do minimo (%d) apos esta venda.%s\n", 
                    MAGENTA, produtos[idxProduto].estoqueMinimo, RESET);
            pausar(); 
        }

        carrinho = realloc(carrinho, (itensNoCarrinho + 1) * sizeof(ItemVenda));
        if (carrinho == NULL) {
            printf("%s[ERRO] Falha de memoria no carrinho.%s\n", vermelhoErro, RESET);
            break; 
        }

        carrinho[itensNoCarrinho].produtoCodigo = codigoProduto;
        strcpy(carrinho[itensNoCarrinho].descricao, produtos[idxProduto].descricao); 
        
        carrinho[itensNoCarrinho].quantidade = quantidade;
        carrinho[itensNoCarrinho].precoUnitario = produtos[idxProduto].precoVenda;
        carrinho[itensNoCarrinho].totalItem = quantidade * produtos[idxProduto].precoVenda;
        
        totalCarrinho += carrinho[itensNoCarrinho].totalItem; 
        
        if (idxProduto != -1) {
            produtos[idxProduto].quantidadeEstoque -= quantidade;
            printf("\n%s >> Item adicionado! Novo estoque: %d%s\n", 
                    verdeSuave, produtos[idxProduto].quantidadeEstoque, RESET);
        }
        
        itensNoCarrinho++; 
        
        printf(" Adicionar outro item? (s/n): ");
        if (scanf(" %c", &continuar) != 1) {
            continuar = 'n';
        }
        limparBuffer();
        
        if (continuar != 's' && continuar != 'S') {
            codigoProduto = 0; 
        }

    } while (codigoProduto != 0);

    if (itensNoCarrinho > 0) {
        
        totalVenda = totalCarrinho; 
        char dataVenda[11]; 
        obterDataAtual(dataVenda);
        
        vendas = realloc(vendas, (numVendas + 1) * sizeof(Venda));
        
        if (vendas == NULL) {
            printf("%s[ERRO] Falha ao alocar memoria para a nova venda.%s\n", vermelhoErro, RESET);
            for (int i = 0; i < itensNoCarrinho; i++) {
                int codigo = carrinho[i].produtoCodigo;
                int quantidadeVendida = carrinho[i].quantidade;
                int idx = buscarProdutoPorCodigo(codigo);
                if (idx != -1) produtos[idx].quantidadeEstoque += quantidadeVendida;
            }
            if (carrinho != NULL) free(carrinho);
            itensNoCarrinho = 0;
            return;
        }
        
        int idxNovaVenda = numVendas;
        
        vendas[idxNovaVenda].idVenda = idContadorVenda; 
        vendas[idxNovaVenda].clienteCodigo = codigoCliente; 
        strcpy(vendas[idxNovaVenda].dataVenda, dataVenda); 
        vendas[idxNovaVenda].totalVenda = totalVenda; 
        vendas[idxNovaVenda].status = 'A'; // ABERTA
        vendas[idxNovaVenda].numItens = itensNoCarrinho;
        
        vendas[idxNovaVenda].itens = (ItemVenda*)malloc(itensNoCarrinho * sizeof(ItemVenda));
        if (vendas[idxNovaVenda].itens == NULL) {
            printf("%s[ERRO] Falha memoria itens venda.%s\n", vermelhoErro, RESET);
            for (int i = 0; i < itensNoCarrinho; i++) {
                int codigo = carrinho[i].produtoCodigo;
                int qtd = carrinho[i].quantidade;
                int idx = buscarProdutoPorCodigo(codigo);
                if (idx != -1) produtos[idx].quantidadeEstoque += qtd;
            }
            if (carrinho != NULL) free(carrinho);
            itensNoCarrinho = 0;
            return;
        }
        memcpy(vendas[idxNovaVenda].itens, carrinho, itensNoCarrinho * sizeof(ItemVenda));
        
        numVendas++;
        idContadorVenda++; 
        
        printf("------------------------------------------\n");
        printf("%sVENDA ID %d CRIADA! Total: R$ %.2f%s\n", verdeSuave, vendas[idxNovaVenda].idVenda, totalVenda, RESET);

        char escolha;
        printf("\n%sDeseja pagar agora (s/n)?%s ", PUMPKIN_PUNCH, RESET);
        
        if (scanf(" %c", &escolha) == 1 && (escolha == 's' || escolha == 'S')) {
            limparBuffer();
            printf("%sIndo para pagamento...%s\n", CIANO, RESET);

            pagamento(); 
            
            if (vendas[idxNovaVenda].status == 'C') { 
                printf("%s[CANCELADO] Restaurando estoque...%s\n", vermelhoErro, RESET);
                for (int i = 0; i < vendas[idxNovaVenda].numItens; i++) {
                    int codigo = vendas[idxNovaVenda].itens[i].produtoCodigo;
                    int quantidadeVendida = vendas[idxNovaVenda].itens[i].quantidade;
                    int idxProduto = buscarProdutoPorCodigo(codigo);
                    if (idxProduto != -1) {
                        produtos[idxProduto].quantidadeEstoque += quantidadeVendida;
                    }
                }
                printf("%sEstoque restaurado!%s\n", GHOUL_GREEN, RESET);
            }
            
        } else {
            limparBuffer();
            printf("%sVenda ID %d salva em ABERTO.%s\n", MAGENTA, vendas[idxNovaVenda].idVenda, RESET);
            printf("%s[INFO] O estoque permanece reservado.%s\n", PUMPKIN_PUNCH, RESET);
        }

    } else {
        printf("%sCarrinho vazio. Venda cancelada.%s\n", PUMPKIN_PUNCH, RESET);
    }

    if (carrinho != NULL) {
        free(carrinho);
        carrinho = NULL;
    }
    itensNoCarrinho = 0;
    
    pausar();
}

void listarVendasEmAberto() {
    int encontrou = 0;
    
    printf("\n%s--- VENDAS ABERTAS PENDENTES ---%s\n", PUMPKIN_PUNCH, RESET);
    printf("%s--------------------------------------------------------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s| %-5s | %-10s | %-15s | %-30s | %-10s |%s\n", CIANO, "ID", "DATA", "COD. CLI", "NOME CLIENTE", "TOTAL", RESET);
    printf("%s|-------|------------|-----------------|--------------------------------|------------|%s\n", MIDNIGHT_HOWL, RESET);
    
    for (int i = 0; i < numVendas; i++) {
        if (vendas[i].status == 'A') {
            encontrou = 1;
            
            int codCliente = vendas[i].clienteCodigo;
            int idxCliente = buscarClientePorCodigo(codCliente);
            char nomeCliente[31]; 
            
            if (codCliente == 0 || idxCliente == -1) {
                strcpy(nomeCliente, "Nao Identificado");
            } else {
                strncpy(nomeCliente, clientes[idxCliente].nomeCompleto, 30);
                nomeCliente[30] = '\0';
            }
            
            printf("| %-5d | %-10s | %-15d | %-30s | %10.2f |\n", 
                   vendas[i].idVenda, 
                   vendas[i].dataVenda, 
                   codCliente, 
                   nomeCliente, 
                   vendas[i].totalVenda);
        }
    }
    
    printf("%s--------------------------------------------------------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);

    if (!encontrou) {
        printf("%s >> Nenhuma venda em aberto.%s\n", verdeSuave, RESET);
    }
}

void listarTodasVendas() {
    limparTela();
    printf("%s====================================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                               HISTORICO GERAL DE VENDAS                                          |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s====================================================================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (numVendas == 0) {
        printf("%s >> Nenhuma venda registrada no sistema.%s\n", PUMPKIN_PUNCH, RESET);
        printf("%s====================================================================================================%s\n", MIDNIGHT_HOWL, RESET);
        pausar();
        return;
    }

    printf("%s| %-5s | %-10s | %-25s | %-12s | %-15s |%s\n", 
           CIANO, "ID", "DATA", "CLIENTE", "TOTAL (R$)", "STATUS", RESET);
    printf("%s|-------|------------|---------------------------|--------------|-----------------|%s\n", MIDNIGHT_HOWL, RESET);

    for (int i = 0; i < numVendas; i++) {
        char statusStr[20];
        const char* corStatus;
        char nomeCliente[26];

        switch (vendas[i].status) {
            case 'A':
                strcpy(statusStr, "EM ABERTO");
                corStatus = PUMPKIN_PUNCH;
                break;
            case 'F':
                strcpy(statusStr, "FINALIZADA");
                corStatus = verdeSuave; 
                break;
            case 'C':
                strcpy(statusStr, "CANCELADA");
                corStatus = vermelhoErro;
                break;
            default:
                strcpy(statusStr, "DESCONHECIDO");
                corStatus = RESET;
                break;
        }

        int idxCliente = buscarClientePorCodigo(vendas[i].clienteCodigo);
        if (idxCliente != -1) {
            strncpy(nomeCliente, clientes[idxCliente].nomeCompleto, 25);
            nomeCliente[25] = '\0';
        } else {
            if (vendas[i].clienteCodigo == 0 || vendas[i].clienteCodigo == -1) {
                 strcpy(nomeCliente, "Nao Identificado");
            } else {
                 snprintf(nomeCliente, 25, "Excluido (Cod %d)", vendas[i].clienteCodigo);
            }
        }

        printf("| %-5d | %-10s | %-25s | %12.2f | %s%-15s%s |\n", 
               vendas[i].idVenda, 
               vendas[i].dataVenda, 
               nomeCliente, 
               vendas[i].totalVenda,
               corStatus, statusStr, RESET); 
    }

    printf("%s====================================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    pausar();
}


/*==========================================================================*/
/* 12. IMPLEMENTACAO: FINANCEIRO E CAIXA                                    */
/*==========================================================================*/

void abrirCaixa() {
    if (!validarUsuarioAdmin()) return; 

    if (caixaAberto == 1) {
        printf("%s[ERRO] O caixa ja esta aberto.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }

    printf("\n%s=== ABERTURA DE CAIXA ===%s\n", CIANO, RESET);
    printf(" %sInforme o saldo inicial (R$):%s ", VAMPIRE_BITE, RESET);
    
    if (scanf("%f", &saldoAbertura) != 1 || saldoAbertura < 0) {
        limparBuffer();
        printf("%s[ERRO] Saldo invalido. Cancelado.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    limparBuffer();

    caixaAberto = 1;
    totalRetiradas = 0.0;
    printf("%s >> Caixa aberto! Saldo inicial: R$ %.2f%s\n", verdeSuave, saldoAbertura, RESET);
    pausar();
}

void fecharCaixa() {
    if (!validarUsuarioAdmin()) return; 

    if (caixaAberto == 0) {
        printf("%s[ERRO] O caixa ja esta fechado.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    
    if (itensNoCarrinho > 0) {  
        limparTela();
        printf("%s[ATENCAO] Venda em andamento no carrinho!%s\n", vermelhoErro, RESET);
        printf("Finalize ou cancele a venda antes de fechar.\n");
        pausar(); 
        return;
    }

    if (buscarVendaEmAberto() != -1) {
        limparTela();
        printf("%s[ATENCAO CRITICA] Ha vendas ABERTAS no sistema.%s\n", vermelhoErro, RESET);
        printf("Feche ou cancele TODAS as vendas pendentes antes do fechamento de caixa.\n");
        pausar();
        return;
    }
    
    char dataHoje[11];
    obterDataAtual(dataHoje);

    limparTela();
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|           FECHAMENTO DE CAIXA (%s)       |%s\n", PURPLE_PHANTOM, dataHoje, RESET);
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
    
    float totalRecebido = 0.0;
    float vlPagoD_MD = 0.0; 
    float vlPagoC_MC = 0.0; 

    for (int i = 0; i < numPagamentos; i++) {
        if (strcmp(pagamentos[i].dataPagamento, dataHoje) == 0) {
            
            if (strcmp(pagamentos[i].tipo, "d") == 0 || strcmp(pagamentos[i].tipo, "md") == 0) {
                vlPagoD_MD += pagamentos[i].valor;
            } else if (strcmp(pagamentos[i].tipo, "c") == 0 || strcmp(pagamentos[i].tipo, "mc") == 0) {
                vlPagoC_MC += pagamentos[i].valor;
            }
        }
    }
    
    float saldoFinalTeorico = saldoAbertura + vlPagoD_MD - totalRetiradas;
    
    printf(" Saldo Abertura:          R$ %10.2f\n", saldoAbertura);
    printf(" Dinheiro Recebido:       R$ %10.2f\n", vlPagoD_MD);
    printf(" Pagamentos Cartao:       R$ %10.2f\n", vlPagoC_MC);
    printf(" Retiradas (Sangria):     R$ %10.2f\n", totalRetiradas);
    printf("%s--------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
    printf(" %sSALDO TEORICO (Dinheiro): R$ %10.2f%s\n", CIANO, saldoFinalTeorico, RESET);
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);

    float contado = 0;
    printf("\n %sInforme o valor CONTADO (Dinheiro): R$%s ", VAMPIRE_BITE, RESET);
    if (scanf("%f", &contado) != 1) {
        limparBuffer(); contado = 0;
    } else {
        limparBuffer();
    }
    
    float AJUSTE = contado - saldoFinalTeorico;

    if (AJUSTE > 0.01 || AJUSTE < -0.01) { 
        printf("\n%s[DIVERGENCIA] O caixa nao bateu!%s\n", vermelhoErro, RESET);
        printf("%sAJUSTE (Sobra/Falta): R$ %.2f%s\n", PUMPKIN_PUNCH, AJUSTE, RESET);
        printf("Situacao: %s\n", (AJUSTE > 0 ? "SOBRA" : "FALTA"));
    } else { 
        AJUSTE = 0.0;
        printf("\n%s >> Caixa bateu corretamente! AJUSTE: R$ 0.00%s\n", verdeSuave, RESET); 
    }
    
    char confirm;
    printf("\n Confirmar fechamento do caixa (s/n)? ");
    if (scanf(" %c", &confirm) != 1 || (confirm != 's' && confirm != 'S')) {
        limparBuffer();
        printf("%sCancelado.%s\n", PUMPKIN_PUNCH, RESET);
        pausar();
        return;
    }
    limparBuffer();

    caixaAberto = 0;
    printf("\n%s >> Caixa fechado com sucesso!%s\n", verdeSuave, RESET);
    pausar();
}

void retiradaCaixa() {
    if (!validarUsuarioAdmin()) return; 

    if (caixaAberto == 0) {
        printf("%s[ERRO] Caixa fechado.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }

    float valorRetirada;
    const float FUNDO_MINIMO_TROCO = 50.00; 
    
    float saldoAtualDinheiro = calcularTotalDinheiro();
    
    float valorMaximoRetirada = saldoAtualDinheiro - FUNDO_MINIMO_TROCO;
    
    limparTela();
    printf("\n%s=== RETIRADA DE CAIXA (SANGRIA) ===%s\n", MAGENTA, RESET);
    printf(" Saldo Total Dinheiro:    R$ %.2f\n", saldoAtualDinheiro);
    printf(" Fundo Troco Obrigatorio: R$ %.2f\n", FUNDO_MINIMO_TROCO);
    printf("%s MAXIMO PERMITIDO:      R$ %.2f%s\n", verdeSuave, valorMaximoRetirada, RESET);
    printf("--------------------------------------\n");

    if (valorMaximoRetirada <= 0) {
        printf("%s[ALERTA] Saldo insuficiente para sangria (Respeitando fundo de troco).%s\n", PUMPKIN_PUNCH, RESET);
        pausar();
        return;
    }

    printf(" %sValor da retirada:%s ", VAMPIRE_BITE, RESET);
    if (scanf("%f", &valorRetirada) != 1 || valorRetirada <= 0) {
        limparBuffer();
        printf("%s[ERRO] Valor invalido.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    limparBuffer();

    if (valorRetirada > valorMaximoRetirada) {
        printf("%s[ERRO] Valor excede o maximo permitido.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }

    totalRetiradas += valorRetirada;

    printf("\n%s >> Retirada de R$ %.2f registrada.%s\n", verdeSuave, valorRetirada, RESET);
    printf(" Novo saldo Dinheiro: R$ %.2f\n", saldoAtualDinheiro - valorRetirada);
    pausar();
}

float calcularTotalDinheiro() {
    float totalDinheiro = 0.0;
    
    for (int i = 0; i < numPagamentos; i++) {
        if (strcmp(pagamentos[i].tipo, "d") == 0 || strcmp(pagamentos[i].tipo, "md") == 0) {
            totalDinheiro += pagamentos[i].valor;
        }
    }
    
    return saldoAbertura + totalDinheiro - totalRetiradas;
}

void registrarPagamento(int idVenda, float valor, const char* tipo) {
    pagamentos = realloc(pagamentos, (numPagamentos + 1) * sizeof(Pagamento));
    if (pagamentos == NULL) {
        printf("%s[ERRO] Falha memoria pagamento.%s\n", vermelhoErro, RESET);
        return;
    }

    pagamentos[numPagamentos].idVenda = idVenda;
    pagamentos[numPagamentos].valor = valor;
    
    obterDataAtual(pagamentos[numPagamentos].dataPagamento); 
    
    strncpy(pagamentos[numPagamentos].tipo, tipo, 2);
    pagamentos[numPagamentos].tipo[2] = '\0'; 

    numPagamentos++;
}

void pagamento() {
    if (caixaAberto == 0) {
        limparTela();
        printf("%s[ERRO] O CAIXA ESTA FECHADO.%s\n", vermelhoErro, RESET);
        printf("Abra o caixa antes de receber pagamentos.\n");
        pausar();
        return;
    }

    int idVenda = -1;
    int idxVenda = -1;
    int opcao;
    float valorTotal;
    
    int pagamentoAprovado = 0; 
    float troco = 0.0;
    
    float valorDinheiro = 0.0;
    float valorCartao = 0.0;
    char tipoDinheiro[3] = ""; 
    char tipoCartao[3] = ""; 
    
    limparTela();
    printf("%s=== PROCESSAR PAGAMENTO ===%s\n", CIANO, RESET);
    listarVendasEmAberto(); 
    
    printf("\n %sID da Venda (0 cancela):%s ", VAMPIRE_BITE, RESET);
    if (scanf("%d", &idVenda) != 1) {
        limparBuffer();
        printf("%s[ERRO] Invalido.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    limparBuffer();

    if (idVenda == 0) {
        printf("%sCancelado.%s\n", PUMPKIN_PUNCH, RESET);
        pausar();
        return;
    }

    idxVenda = buscarVendaPorID(idVenda); 
    
    if (idxVenda == -1) {
        printf("%s[ERRO] Venda nao encontrada.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    
    if (vendas[idxVenda].status != 'A') {
        printf("%s[ALERTA] Venda nao esta aberta.%s\n", PUMPKIN_PUNCH, RESET);
        pausar();
        return;
    }
    
    valorTotal = vendas[idxVenda].totalVenda;

    do {
        valorDinheiro = 0.0;
        valorCartao = 0.0;
        troco = 0.0; 
        
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|          PAGAMENTO VENDA ID %-5d               |%s\n", vendas[idxVenda].idVenda, PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" Total a Pagar: %sR$ %.2f%s\n", verdeSuave, valorTotal, RESET);
        printf("--------------------------------------------------\n");

        printf(" 1 - Dinheiro\n");
        printf(" 2 - Cartao\n");
        printf(" 3 - Misto (Dinheiro + Cartao)\n");
        printf(" 4 - %sCANCELAR VENDA%s\n", vermelhoErro, RESET);
        printf("--------------------------------------------------\n");
        printf(" %sOpcao:%s ", VAMPIRE_BITE, RESET);

        if (scanf("%d", &opcao) != 1) {
            limparBuffer(); opcao = -1;
        }
        limparBuffer();

        pagamentoAprovado = 0; 

        switch (opcao) {
            case 1: {
                float recebido;
                printf(" Valor recebido (Dinheiro): R$ ");
                if (scanf("%f", &recebido) != 1 || recebido <= 0) {
                    limparBuffer();
                    printf("%s[ERRO] Valor invalido.%s\n", vermelhoErro, RESET);
                    pausar();
                    continue; 
                }
                limparBuffer();

                if (recebido >= valorTotal) {
                    strcpy(tipoDinheiro, "d");
                    valorDinheiro = valorTotal; 
                    troco = recebido - valorTotal;
                    pagamentoAprovado = 1;
                } 
                else {
                    float restante = valorTotal - recebido;
                    printf("\n%sFALTA: R$ %.2f%s\n", PUMPKIN_PUNCH, restante, RESET);
                    printf("Cobrar restante no CARTAO? (s/n): ");
                    
                    char resp;
                    if (scanf(" %c", &resp) != 1) resp = 'n';
                    limparBuffer();

                    if (resp == 's' || resp == 'S') {
                        printf(" Processando cartao... (1-OK / 2-Erro): ");
                        int stCartao;
                        scanf("%d", &stCartao);
                        limparBuffer();

                        if (stCartao == 1) {
                            strcpy(tipoDinheiro, "md");
                            valorDinheiro = recebido;
                            strcpy(tipoCartao, "mc");
                            valorCartao = restante;
                            pagamentoAprovado = 1;
                            printf("%s >> Sucesso!%s\n", verdeSuave, RESET);
                        } else {
                            printf("%sCartao recusado.%s\n", vermelhoErro, RESET);
                            pausar();
                        }
                    } else {
                        printf("%sCancelado.%s\n", PUMPKIN_PUNCH, RESET);
                        pausar();
                    }
                }
                break;
            }

            case 2: {
                float cobrarCartao;
                printf(" Valor no Cartao (0 = Total %.2f): ", valorTotal);
                if (scanf("%f", &cobrarCartao) != 1) cobrarCartao = 0;
                limparBuffer();

                if (cobrarCartao <= 0 || cobrarCartao > valorTotal) {
                    cobrarCartao = valorTotal; 
                }

                printf(" Processando R$ %.2f... (1-Aprovado / 2-Recusado): ", cobrarCartao);
                int status;
                if (scanf("%d", &status) != 1) status = 2;
                limparBuffer();
                
                if (status == 1) {
                    if (cobrarCartao >= valorTotal - 0.01) {
                        strcpy(tipoCartao, "c");
                        valorCartao = valorTotal;
                        pagamentoAprovado = 1;
                        printf("%sAPROVADO.%s\n", verdeSuave, RESET);
                    } 
                    else {
                        float restante = valorTotal - cobrarCartao;
                        printf("\n%sCARTAO APROVADO.%s Faltam R$ %.2f.\n", verdeSuave, RESET, restante);
                        printf("Receber o restante em DINHEIRO? (s/n): ");
                        
                        char resp;
                        scanf(" %c", &resp);
                        limparBuffer();

                        if (resp == 's' || resp == 'S') {
                            printf("Dinheiro recebido: ");
                            float dinRecebido;
                            scanf("%f", &dinRecebido);
                            limparBuffer();

                            if (dinRecebido >= restante) {
                                strcpy(tipoCartao, "mc");
                                valorCartao = cobrarCartao;
                                strcpy(tipoDinheiro, "md");
                                valorDinheiro = restante;
                                troco = dinRecebido - restante;
                                pagamentoAprovado = 1;
                            } else {
                                printf("%sDinheiro insuficiente. Estornando cartao...%s\n", vermelhoErro, RESET);
                                pausar();
                            }
                        } else {
                            printf("%sCancelando...%s\n", vermelhoErro, RESET);
                            pausar();
                        }
                    }
                } else {
                    printf("%sNEGADO.%s\n", vermelhoErro, RESET);
                    pausar();
                }
                break;
            }

            case 3: {
                float pagoDinheiroBruto;
                float restanteCartao;
                
                printf(" Valor em Dinheiro: R$ ");
                if (scanf("%f", &pagoDinheiroBruto) != 1 || pagoDinheiroBruto < 0) {
                    limparBuffer();
                    continue;
                }
                limparBuffer();

                restanteCartao = valorTotal - pagoDinheiroBruto;
                
                if (restanteCartao > 0) {
                    strcpy(tipoDinheiro, "md"); 
                    valorDinheiro = pagoDinheiroBruto;
                    strcpy(tipoCartao, "mc");
                    valorCartao = restanteCartao; 

                    printf(" Restante Cartao: R$ %.2f. (1-OK / 2-Erro): ", valorCartao);
                    int status;
                    if (scanf("%d", &status) != 1) status = 2;
                    limparBuffer();

                    if (status == 1) {
                        pagamentoAprovado = 1;
                    } else {
                        printf("%sCartao recusado.%s\n", vermelhoErro, RESET);
                        pausar();
                    }
                } else {
                    strcpy(tipoDinheiro, "d"); 
                    valorDinheiro = valorTotal;
                    troco = -restanteCartao;
                    pagamentoAprovado = 1;
                }
                break;
            }

            case 4: 
                vendas[idxVenda].status = 'C';
                for (int i = 0; i < vendas[idxVenda].numItens; i++) {
                    int codigo = vendas[idxVenda].itens[i].produtoCodigo;
                    int qtd = vendas[idxVenda].itens[i].quantidade;
                    int idxProd = buscarProdutoPorCodigo(codigo);
                    if (idxProd != -1) produtos[idxProd].quantidadeEstoque += qtd;
                }
                
                printf("\n%s >> Venda cancelada e estoque restaurado.%s\n", vermelhoErro, RESET);
                pausar();
                return; 
                
            default:
                printf("%sOpcao invalida.%s\n", vermelhoErro, RESET);
                pausar();
                continue;
        }
        
    } while (!pagamentoAprovado); 

    if (valorDinheiro > 0) {
        registrarPagamento(idVenda, valorDinheiro, tipoDinheiro);
    }
    
    if (valorCartao > 0) {
        registrarPagamento(idVenda, valorCartao, tipoCartao);
    }

    vendas[idxVenda].status = 'F';

    printf("\n%s==================================================%s\n", verdeSuave, RESET);
    printf("%s VENDA FINALIZADA COM SUCESSO!%s\n", verdeSuave, RESET);
    if (troco > 0.0) {
        printf(" %sTROCO A DEVOLVER: R$ %.2f%s\n", PUMPKIN_PUNCH, troco, RESET);
    }
    printf("%s==================================================%s\n", verdeSuave, RESET);
    
    pausar();
}


/*==========================================================================*/
/* 13. IMPLEMENTACAO: RELATORIOS (LISTAGEM DE DADOS)                        */
/*==========================================================================*/

void listaVendas() {
    limparTela();
    printf("%s=================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                  RESUMO DO DIA (VENDAS)                       |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s=================================================================%s\n", MIDNIGHT_HOWL, RESET);

    int qtdVendas = numVendas;
    float faturamento = 0.0;
    float vlPagoD_MD = 0.0;
    float vlPagoC_MC = 0.0;
    float totalAlimento = 0.0, totalLimpeza = 0.0, totalPani = 0.0;

    for (int i = 0; i < numVendas; i++) {
        if (vendas[i].status == 'F') { 
            faturamento += vendas[i].totalVenda;
            
            for (int j = 0; j < vendas[i].numItens; j++) {
                ItemVenda item = vendas[i].itens[j];
                int indiceProduto = buscarProdutoPorCodigo(item.produtoCodigo); 

                if (indiceProduto != -1) {
                    char *categoria = produtos[indiceProduto].categoria; 

                    if (strcmp(categoria, "Alimento") == 0) {
                        totalAlimento += item.totalItem;
                    } else if (strcmp(categoria, "Limpeza") == 0) {
                        totalLimpeza += item.totalItem;
                    } else if (strcmp(categoria, "Panificacao") == 0) {
                        totalPani += item.totalItem;
                    }
                }
            }
        }
    }

    for (int i = 0; i < numPagamentos; i++) {
        if (strcmp(pagamentos[i].tipo, "d") == 0 || strcmp(pagamentos[i].tipo, "md") == 0) {
            vlPagoD_MD += pagamentos[i].valor;
        } else if (strcmp(pagamentos[i].tipo, "c") == 0 || strcmp(pagamentos[i].tipo, "mc") == 0) {
            vlPagoC_MC += pagamentos[i].valor;
        }
    }

    if (qtdVendas == 0 && totalRetiradas == 0) {
         printf("| %-61s |\n", "Nenhuma venda/retirada hoje.");
         if (saldoAbertura > 0) {
              printf("| Saldo Abertura: R$ %-41.2f |\n", saldoAbertura);
         }
         printf("%s=================================================================%s\n", MIDNIGHT_HOWL, RESET);
         pausar();
         return;
    }
    
    printf(" Qtd Vendas:              %d\n", qtdVendas);
    printf(" Faturamento Total:       R$ %.2f\n", faturamento);
    printf(" Abertura Caixa:          R$ %.2f\n", saldoAbertura);
    printf(" Recebido em Dinheiro:    R$ %.2f\n", vlPagoD_MD);
    printf(" Recebido em Cartao:      R$ %.2f\n", vlPagoC_MC);
    printf(" Retiradas:               R$ %.2f\n", totalRetiradas);
    printf("%s-----------------------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
    printf(" %sPor Categoria (Bruto):%s\n", CIANO, RESET);
    printf("   - Alimentos:           R$ %.2f\n", totalAlimento);
    printf("   - Limpeza:             R$ %.2f\n", totalLimpeza);
    printf("   - Panificacao:         R$ %.2f\n", totalPani);
    printf("%s-----------------------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
    
    float saldoAtualDinheiro = saldoAbertura + vlPagoD_MD - totalRetiradas;
    printf(" %sSALDO FINAL CAIXA (DINHEIRO): R$ %.2f%s\n", verdeSuave, saldoAtualDinheiro, RESET);
    printf("%s=================================================================%s\n", MIDNIGHT_HOWL, RESET);
    pausar();
}

void relatorioClientesAlfa() {
    limparTela();
    printf("%s==========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|            RELATORIO DE CLIENTES (ALFABETICO)                          |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s==========================================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (numClientes == 0) {
        printf("| Nenhum cliente cadastrado.                                             |\n");
        printf("%s==========================================================================%s\n", MIDNIGHT_HOWL, RESET);
        pausar();
        return;
    }
    
    Cliente *clientesOrdenados = (Cliente*)malloc(numClientes * sizeof(Cliente));
    if (clientesOrdenados == NULL) {
        printf("%s[ERRO] Memoria insuficiente.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    memcpy(clientesOrdenados, clientes, numClientes * sizeof(Cliente));

    qsort(clientesOrdenados, numClientes, sizeof(Cliente), comparaClientesPorNome); 

    printf("%s| %-6s | %-30s | %-14s | %-15s |%s\n", CIANO, "CODIGO", "NOME COMPLETO", "CPF", "CELULAR", RESET);
    printf("%s|--------|--------------------------------|----------------|-----------------|%s\n", MIDNIGHT_HOWL, RESET);
    
    for (int i = 0; i < numClientes; i++) {
        printf("| %-6d | %-30s | %-14s | %-15s |\n",
               clientesOrdenados[i].codigo, 
               clientesOrdenados[i].nomeCompleto, 
               clientesOrdenados[i].cpf, 
               clientesOrdenados[i].celularWhats);
    }

    free(clientesOrdenados); 
    printf("%s==========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    pausar();
}

void relatorioClientesPorPeriodo() {
    char dataInicio[11], dataFim[11];
    lerPeriodoDatas(dataInicio, dataFim);
    
    limparTela();
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|     CLIENTES COMPRADORES (%s - %s)   |%s\n", PURPLE_PHANTOM, dataInicio, dataFim, RESET);
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
    
    if (numClientes == 0 || numVendas == 0) {
        printf("%s[INFO] Sem dados suficientes.%s\n", PUMPKIN_PUNCH, RESET);
        return;
    }

    int* clienteComprou = (int*)calloc(numClientes, sizeof(int));
    if (clienteComprou == NULL) {
        printf("%s[ERRO] Memoria.%s\n", vermelhoErro, RESET);
        return;
    }

    for (int i = 0; i < numVendas; i++) {
        if (vendas[i].status == 'F' && dataEstaNoPeriodo(vendas[i].dataVenda, dataInicio, dataFim)) { 
            for (int j = 0; j < numClientes; j++) {
                if (clientes[j].codigo == vendas[i].clienteCodigo) {
                    clienteComprou[j] = 1; 
                    break;
                }
            }
        }
    }

    int encontrou = 0;
    for (int i = 0; i < numClientes; i++) {
        if (clienteComprou[i] == 1) {
            printf(" Cod: %d | Nome: %s | CPF: %s\n", 
                   clientes[i].codigo, clientes[i].nomeCompleto, clientes[i].cpf);
            encontrou = 1;
        }
    }
    
    if (!encontrou) {
        printf("%s >> Nenhum cliente comprou neste periodo.%s\n", PUMPKIN_PUNCH, RESET);
    }

    free(clienteComprou);
    pausar();
}

void relatorioProdutosAlfa() {
    limparTela();
    printf("%s========================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                          RELATORIO DE PRODUTOS (ALFABETICO)                          |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s========================================================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (numProdutos == 0) {
        printf("| Nenhum produto cadastrado.                                                           |\n");
        printf("%s========================================================================================%s\n", MIDNIGHT_HOWL, RESET);
        pausar();
        return;
    }
    
    Produto *produtosOrdenados = (Produto*)malloc(numProdutos * sizeof(Produto));
    if (produtosOrdenados == NULL) {
        printf("%s[ERRO] Memoria.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }
    memcpy(produtosOrdenados, produtos, numProdutos * sizeof(Produto));

    qsort(produtosOrdenados, numProdutos, sizeof(Produto), comparaProdutosPorDescricao); 

    printf("%s| %-6s | %-30s | %-12s | %-7s | %-15s |%s\n", CIANO, "COD", "DESCRICAO", "PRECO", "ESTOQUE", "CATEGORIA", RESET);
    printf("%s|--------|--------------------------------|--------------|---------|-----------------|%s\n", MIDNIGHT_HOWL, RESET);
    
    for (int i = 0; i < numProdutos; i++) {
        printf("| %-6d | %-30s | R$ %9.2f | %-7d | %-15s |\n",
               produtosOrdenados[i].codigo, 
               produtosOrdenados[i].descricao, 
               produtosOrdenados[i].precoVenda, 
               produtosOrdenados[i].quantidadeEstoque,
               produtosOrdenados[i].categoria);
    }

    free(produtosOrdenados); 
    printf("%s========================================================================================%s\n", MIDNIGHT_HOWL, RESET);
    pausar();
}

void relatorioEstoqueMinimo() {
    limparTela();
    printf("%s==========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|                  PRODUTOS COM ESTOQUE BAIXO                            |%s\n", PURPLE_PHANTOM, RESET);
    printf("%s==========================================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (numProdutos == 0) {
        printf("| Nenhum produto cadastrado.                                             |\n");
        pausar();
        return;
    }

    Produto *produtosOrdenados = (Produto*)malloc(numProdutos * sizeof(Produto));
    if (produtosOrdenados == NULL) {
        printf("%s[ERRO] Memoria.%s\n", vermelhoErro, RESET);
        return;
    }
    memcpy(produtosOrdenados, produtos, numProdutos * sizeof(Produto));

    qsort(produtosOrdenados, numProdutos, sizeof(Produto), comparaProdutosPorDescricao);

    int contador = 0;
    printf("%s| %-6s | %-30s | %-5s | %-6s | %-10s |%s\n", CIANO, "COD", "DESCRICAO", "ATUAL", "MIN", "STATUS", RESET);
    printf("%s|--------|--------------------------------|-------|--------|------------|%s\n", MIDNIGHT_HOWL, RESET);

    for (int i = 0; i < numProdutos; i++) {
        Produto p = produtosOrdenados[i];
        
        if (p.quantidadeEstoque <= p.estoqueMinimo) {
            printf("| %-6d | %-30s | %-5d | %-6d | %s%-10s%s |\n",
                   p.codigo, p.descricao, p.quantidadeEstoque, p.estoqueMinimo, 
                   (p.quantidadeEstoque == 0 ? vermelhoErro : PUMPKIN_PUNCH), 
                   "BAIXO", RESET);
            contador++;
        }
    }

    if (contador == 0) {
        printf("| %-70s |\n", "Nenhum produto com estoque baixo encontrado.");
    }

    free(produtosOrdenados);
    printf("%s==========================================================================%s\n", MIDNIGHT_HOWL, RESET);
    pausar();
}

typedef struct {
    int codigo;
    char descricao[100];
    int quantidadeVendida;
} RankingProduto;

void relatorioProdutosMaisVendidos() {
    char dataInicio[11], dataFim[11];
    lerPeriodoDatas(dataInicio, dataFim);
    
    limparTela();
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s|   MAIS VENDIDOS (%s - %s)    |%s\n", PURPLE_PHANTOM, dataInicio, dataFim, RESET);
    printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (numProdutos == 0 || numVendas == 0) {
        printf("%s[INFO] Sem dados.%s\n", PUMPKIN_PUNCH, RESET);
        return;
    }

    RankingProduto* ranking = (RankingProduto*)calloc(numProdutos, sizeof(RankingProduto));
    if (ranking == NULL) {
        printf("%s[ERRO] Memoria.%s\n", vermelhoErro, RESET);
        return;
    }
    
    for (int i = 0; i < numProdutos; i++) {
        ranking[i].codigo = produtos[i].codigo;
        strcpy(ranking[i].descricao, produtos[i].descricao);
        ranking[i].quantidadeVendida = 0;
    }

    for (int i = 0; i < numVendas; i++) {
        if (vendas[i].status == 'F' && dataEstaNoPeriodo(vendas[i].dataVenda, dataInicio, dataFim)) { 
            for (int j = 0; j < vendas[i].numItens; j++) {
                int produtoCod = vendas[i].itens[j].produtoCodigo; 
                int quantidade = vendas[i].itens[j].quantidade;
                
                for (int k = 0; k < numProdutos; k++) {
                    if (ranking[k].codigo == produtoCod) {
                        ranking[k].quantidadeVendida += quantidade;
                        break;
                    }
                }
            }
        }
    }

    for (int i = 0; i < numProdutos - 1; i++) {
        for (int j = 0; j < numProdutos - i - 1; j++) {
            if (ranking[j].quantidadeVendida < ranking[j+1].quantidadeVendida) {
                RankingProduto temp = ranking[j];
                ranking[j] = ranking[j+1];
                ranking[j+1] = temp;
            }
        }
    }

    int encontrou = 0;
    printf("%s| %-8s | %-30s | %-12s |%s\n", CIANO, "CODIGO", "DESCRICAO", "QTD VENDIDA", RESET);
    printf("%s|----------|--------------------------------|--------------|%s\n", MIDNIGHT_HOWL, RESET);
    
    for (int i = 0; i < numProdutos; i++) {
        if (ranking[i].quantidadeVendida > 0) {
            printf("| %-8d | %-30s | %-12d |\n", ranking[i].codigo, ranking[i].descricao, ranking[i].quantidadeVendida);
            encontrou = 1;
        }
    }
    printf("%s=============================================================%s\n", MIDNIGHT_HOWL, RESET);

    if (!encontrou) {
        printf("%s >> Nenhum produto vendido no periodo.%s\n", PUMPKIN_PUNCH, RESET);
    }

    free(ranking);
    pausar();
}

void relatorioVendasPeriodo() {
    char dataInicio[11];
    char dataFim[11];
    float faturamentoTotal = 0.0;
    int vendasEncontradas = 0;
    
    limparTela();
    printf("%s=== RELATORIO DE VENDAS POR PERIODO ===%s\n", CIANO, RESET);
    lerPeriodoDatas(dataInicio, dataFim);

    printf("\n%s--- VENDAS FINALIZADAS (%s - %s) ---%s\n", CIANO, dataInicio, dataFim, RESET);
    printf("%s----------------------------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
    printf("%s| %-5s | %-10s | %-15s | %-10s | %-10s |%s\n", CIANO, "ID", "DATA", "COD. CLI", "TOTAL", "STATUS", RESET);
    printf("%s|-------|------------|-----------------|------------|------------|%s\n", MIDNIGHT_HOWL, RESET);

    for (int i = 0; i < numVendas; i++) {
        if (vendas[i].status == 'F') {

            int depoisDoInicio = compararDatas(vendas[i].dataVenda, dataInicio);
            int antesDoFim = compararDatas(vendas[i].dataVenda, dataFim);
            
            if ((depoisDoInicio >= 0) && (antesDoFim <= 0)) {
                
                vendasEncontradas++;
                faturamentoTotal += vendas[i].totalVenda;
                
                printf("| %-5d | %-10s | %-15d | %10.2f | %-10c |\n", 
                       vendas[i].idVenda, 
                       vendas[i].dataVenda, 
                       vendas[i].clienteCodigo, 
                       vendas[i].totalVenda,
                       vendas[i].status);
            }
        }
    }
    
    printf("%s----------------------------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);

    if (vendasEncontradas > 0) {
        printf(" QUANTIDADE: %d\n", vendasEncontradas);
        printf(" %sFATURAMENTO: R$ %.2f%s\n", verdeSuave, faturamentoTotal, RESET);
    } else {
        printf("%s >> Nenhuma venda encontrada no periodo.%s\n", PUMPKIN_PUNCH, RESET);
    }

    pausar();
}

void relatorioFaturamentoConsolidado() {
    char dataInicio[11], dataFim[11];
    float totalFaturamento = 0.0;
    float consolidado[4] = {0.0}; // 0=D, 1=C, 2=MD, 3=MC
    
    limparTela();
    printf("%s=== FATURAMENTO CONSOLIDADO ===%s\n", CIANO, RESET);
    lerPeriodoDatas(dataInicio, dataFim);
    
    printf("\n%sCalculando...%s\n", CIANO, RESET);
    printf("%s---------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);

    int pagamentosEncontrados = 0;

    for (int i = 0; i < numPagamentos; i++) {
        if (dataEstaNoPeriodo(pagamentos[i].dataPagamento, dataInicio, dataFim)) {
            
            pagamentosEncontrados++;
            
            if (strcmp(pagamentos[i].tipo, "d") == 0) {
                consolidado[0] += pagamentos[i].valor; 
            } else if (strcmp(pagamentos[i].tipo, "c") == 0) {
                consolidado[1] += pagamentos[i].valor; 
            } else if (strcmp(pagamentos[i].tipo, "md") == 0) {
                consolidado[2] += pagamentos[i].valor; 
            } else if (strcmp(pagamentos[i].tipo, "mc") == 0) {
                consolidado[3] += pagamentos[i].valor; 
            }
            
            totalFaturamento += pagamentos[i].valor;
        }
    }

    printf(" 1. Dinheiro (Total):         R$ %10.2f\n", consolidado[0]);
    printf(" 2. Cartao (Total):           R$ %10.2f\n", consolidado[1]);
    printf(" 3. Misto (Dinheiro Parc.):   R$ %10.2f\n", consolidado[2]);
    printf(" 4. Misto (Cartao Parc.):     R$ %10.2f\n", consolidado[3]);
    printf("%s---------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
    
    float totalDinheiroPuro = consolidado[0] + consolidado[2];
    float totalCartaoPuro = consolidado[1] + consolidado[3];

    printf(" TOTAL DINHEIRO:              R$ %10.2f\n", totalDinheiroPuro);
    printf(" TOTAL CARTAO:                R$ %10.2f\n", totalCartaoPuro);
    printf("%s=============================================%s\n", MIDNIGHT_HOWL, RESET);
    printf(" %sFATURAMENTO GERAL:           R$ %10.2f%s\n", verdeSuave, totalFaturamento, RESET);

    if (pagamentosEncontrados == 0) {
        printf("\n%s >> Nenhum pagamento encontrado.%s\n", PUMPKIN_PUNCH, RESET);
    }

    pausar();
}

void relatorioEstoqueArquivoDat() {
    int pedido;
    Produto produto;
    FILE *cfPtr;

    if ((cfPtr = fopen("produtos.dat", "rb")) == NULL) {
        printf("%s[ERRO] Arquivo produtos.dat nao encontrado.%s\n", vermelhoErro, RESET);
        pausar();
        return;
    }

    limparTela();
    printf("%s=== LEITURA DIRETA DO ARQUIVO ===%s\n", CIANO, RESET);
    printf(" 1 -> Estoque ZERADO\n");
    printf(" 2 -> Estoque BAIXO (<= Minimo)\n");
    printf(" 3 -> Estoque NORMAL\n");
    printf(" 4 -> Sair\n");
    printf(" %sOpcao:%s ", VAMPIRE_BITE, RESET);
    
    if (scanf("%d", &pedido) != 1) {
        limparBuffer(); pedido = 4;
    } else {
        limparBuffer();
    }

    while (pedido != 4) {
        printf("\n%sRESULTADOS:%s\n", CIANO, RESET);
        printf("%s| %-6s | %-25s | %-7s |%s\n", MIDNIGHT_HOWL, "COD", "DESCRICAO", "ESTOQUE", RESET);
        
        while (fread(&produto, sizeof(Produto), 1, cfPtr) == 1) {
            switch (pedido) {
                case 1: 
                    if (produto.quantidadeEstoque == 0)
                        printf("| %-6d | %-25s | %-7d |\n", produto.codigo, produto.descricao, produto.quantidadeEstoque);
                    break;
                case 2: 
                    if (produto.quantidadeEstoque > 0 && produto.quantidadeEstoque <= produto.estoqueMinimo)
                        printf("| %-6d | %-25s | %-7d |\n", produto.codigo, produto.descricao, produto.quantidadeEstoque);
                    break;
                case 3: 
                    if (produto.quantidadeEstoque > produto.estoqueMinimo)
                        printf("| %-6d | %-25s | %-7d |\n", produto.codigo, produto.descricao, produto.quantidadeEstoque);
                    break;
            }
        }

        rewind(cfPtr); 
        
        printf("\n %sNova busca? (1-Zero, 2-Baixo, 3-Normal, 4-Sair):%s ", VAMPIRE_BITE, RESET);
        if (scanf("%d", &pedido) != 1) {
            limparBuffer(); pedido = 4;
        } else {
            limparBuffer();
        }
    }
    
    fclose(cfPtr); 
}


/*==========================================================================*/
/* 14. IMPLEMENTACAO: MENUS DE NAVEGACAO                                    */
/*==========================================================================*/

int menuInicial() {
    int opcao;
    
    while (1) {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|         SISTEMA DE GESTAO - DONA BERE          |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);

        if (numUsuarios == 0) {
            printf("%s >> AVISO: Cadastre o Admin na Opcao 2.%s\n", PUMPKIN_PUNCH, RESET);
        }

        printf(" 1. Fazer Login\n");
        printf(" 2. Cadastrar Novo Usuario\n"); 
        printf(" 0. Sair do Sistema\n");
        printf("%s--------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sEscolha uma opcao:%s ", VAMPIRE_BITE, RESET);

        if (scanf("%d", &opcao) != 1) {
            limparBuffer();
            opcao = -1; 
        }
        limparBuffer(); 

        switch (opcao) {
            case 1:
                if (numUsuarios == 0) {
                    printf("%s[ERRO] Cadastre o Admin primeiro (Opcao 2).%s\n", vermelhoErro, RESET);
                    pausar();
                } else {
                    if (login() == 1) {
                        return 1; 
                    }
                }
                break;
            
            case 2:
                if (numUsuarios == 0) {
                    cadastrarUsuario();
                } else {
                    if (validarUsuarioAdmin()) {
                        cadastrarUsuario();
                    }
                }
                break;
                
            case 0:
                return 0;
            
            default:
                printf("%sOpcao invalida.%s\n", vermelhoErro, RESET);
                pausar();
                break;
        }
    }
}

void executarMenuPrincipal() {
    int opcaoMenu;
    
    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|                 MENU PRINCIPAL                 |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" Usuario: %s%s%s | Nivel: %s\n", CIANO, usuarioLogado.login, RESET, (usuarioLogado.tipo == 1 ? "Admin" : "Usuario"));
        printf("%s--------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
        printf(" 1. Cadastros\n");
        printf(" 2. Vendas\n");
        printf(" 3. Abertura de Caixa\n");
        printf(" 4. Retirada de Caixa (Sangria)\n");
        printf(" 5. Fechamento de Caixa\n");
        printf(" 6. Relatorios\n");
        printf(" 0. Sair\n");
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sDigite a opcao:%s ", VAMPIRE_BITE, RESET);
        
        if (scanf("%d", &opcaoMenu) != 1) {
            limparBuffer();
            opcaoMenu = -1;
        } else {
            limparBuffer();
        }

        switch (opcaoMenu) {
            case 1: 
                menuCadastros(); 
                break;      
            case 2: 
                menuVendas(); 
                break;
            case 3: 
                menuAberturaCaixa(); 
                break;
            case 4:
                retiradaCaixa();
                break;
            case 5: 
                menuFechamentoCaixa(); 
                break;
            case 6: 
                menuRelatorios(); 
                break;
            
            case 0: {
                int temVendaAberta = 0;
                for (int i = 0; i < numVendas; i++) {
                    if (vendas[i].status == 'A') {
                        temVendaAberta = 1;
                        break;
                    }
                }

                if (temVendaAberta) {
                    char respSair;
                    limparTela();
                    printf("%s[ATENCAO] HA VENDAS PENDENTES!%s\n", vermelhoErro, RESET);
                    printf("Deseja descartar as vendas em aberto e sair? (s/n): ");

                    if (scanf(" %c", &respSair) != 1) {
                        limparBuffer(); respSair = 'n';
                    }
                    limparBuffer();

                    if (respSair == 's' || respSair == 'S') {
                        printf("\n%s >> Cancelando pendencias...%s\n", PUMPKIN_PUNCH, RESET);
                        
                        for (int i = 0; i < numVendas; i++) {
                            if (vendas[i].status == 'A') {
                                for (int j = 0; j < vendas[i].numItens; j++) {
                                    int codProd = vendas[i].itens[j].produtoCodigo;
                                    int qtd = vendas[i].itens[j].quantidade;
                                    int idxProd = buscarProdutoPorCodigo(codProd);
                                    
                                    if (idxProd != -1) {
                                        produtos[idxProd].quantidadeEstoque += qtd;
                                    }
                                }
                                vendas[i].status = 'C'; 
                            }
                        }
                        
                        salvarDados(); 
                        return; 
                    } else {
                        opcaoMenu = -1; 
                        break; 
                    }
                }

                char salvar;
                printf("\n %sSalvar alteracoes antes de sair? (s/n):%s ", VAMPIRE_BITE, RESET);
                scanf(" %c", &salvar);
                limparBuffer(); 
                
                if (salvar == 's' || salvar == 'S') {
                    salvarDados(); 
                }
                break;
            }
            
            default:
                printf("%sOpcao invalida!%s\n", vermelhoErro, RESET);
                pausar();
                break;
        }
    } while (opcaoMenu != 0);
}

void menuCadastros() {
    int opcao;

    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|                MENU DE CADASTROS               |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" 1. Cadastrar Produto\n");
        printf(" 2. Cadastrar Cliente\n");
        printf(" 3. Excluir Produto\n");
        printf(" 4. Excluir Cliente\n");
        printf(" 5. Cadastrar Categoria\n");
        printf(" 6. Adicionar Estoque\n");
        printf(" 7. Cadastrar Novo Usuario (Admin)\n");
        printf(" 0. Voltar ao Menu Principal\n");
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sDigite a opcao:%s ", VAMPIRE_BITE, RESET);

        if (scanf("%d", &opcao) != 1) {
            limparBuffer(); 
            opcao = -1; 
        } else {
            limparBuffer();
        }

        switch (opcao) {
            case 1: cadastrarProduto(); break;
            case 2: cadastrarCliente(); break;
            case 3: excluirProduto(); break;
            case 4: excluirCliente(); break;
            case 5: cadastrarCategoria(); break;
            case 6: adicionarEstoque(); break;
            case 7: if (validarUsuarioAdmin()) cadastrarUsuario(); break;
            case 0: break; 
            default: printf("%sOpcao invalida!%s\n", vermelhoErro, RESET); pausar(); break; 
        } 
    } while (opcao != 0); 
}

void menuVendas() {
    int opcao;
    
    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|                  MENU DE VENDAS                |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" 1. Iniciar Nova Venda\n");
        printf(" 2. Processar Pagamento (Venda Aberta)\n");
        printf(" 3. Listar Vendas Abertas\n");
        printf(" 4. Historico de Vendas\n");
        printf(" 0. Voltar\n");
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sOpcao:%s ", VAMPIRE_BITE, RESET);

        if (scanf("%d", &opcao) != 1) {
            limparBuffer();
            opcao = -1;
        }
        limparBuffer();
        
        switch (opcao) {
            case 1: novaVenda(); break;
            case 2: pagamento(); break;
            case 3: listarVendasEmAberto(); pausar(); break;
            case 4: listarTodasVendas(); break;
            case 0: break;
            default: printf("%sOpcao invalida.%s\n", vermelhoErro, RESET); pausar(); break;
        }
    } while (opcao != 0);
}

void menuAberturaCaixa() {
    if (!validarUsuarioAdmin()) return; 

    if (caixaAberto) { 
        printf("%s\n >> O caixa ja esta aberto!%s\n", PUMPKIN_PUNCH, RESET);
        pausar();
        return;
    }

    limparTela(); 
    printf("%s--- ABERTURA DE CAIXA ---%s\n", CIANO, RESET);
    printf(" %sValor de abertura (Troco): R$%s ", VAMPIRE_BITE, RESET);
    
    if (scanf("%f", &saldoAbertura) != 1) {
        limparBuffer();
        saldoAbertura = -1.0; 
    } else {
        limparBuffer();
    }

    if (saldoAbertura >= 0) {
        caixaAberto = 1;
        totalRetiradas = 0.0;
        printf("\n%s >> Caixa aberto com R$ %.2f%s\n", verdeSuave, saldoAbertura, RESET); 
    } else { 
        printf("%s\n[ERRO] Valor invalido!\n%s", vermelhoErro, RESET); 
    }
    pausar(); 
}

void menuFechamentoCaixa() {
    int opcao;  

    if (itensNoCarrinho > 0) {  
        limparTela();
        printf("%s[ERRO] Venda em andamento no carrinho.%s\n", vermelhoErro, RESET);
        pausar(); 
        return;
    }

    do { 
        limparTela(); 
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|               FECHAMENTO DE CAIXA              |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" 1. Fechar o Caixa\n"); 
        printf(" 2. Voltar\n"); 
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sOpcao:%s ", VAMPIRE_BITE, RESET);
        
        if (scanf("%d", &opcao) != 1) {
            limparBuffer();
            opcao = 0;
        } else {
            limparBuffer();
        }

        switch (opcao) { 
            case 1: fecharCaixa(); break; 
            case 2: break; 
            default: printf("%s\nOpcao invalida!\n%s", vermelhoErro, RESET); pausar(); 
        }
    } while (opcao != 2);
}

void menuRelatorios() {
    int op;
    
    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|               MENU DE RELATORIOS               |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" 1. Relatorios de Clientes\n");
        printf(" 2. Relatorios de Produtos\n");
        printf(" 3. Relatorios de Vendas\n");
        printf(" 4. Voltar\n");
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sOpcao:%s ", VAMPIRE_BITE, RESET);
        
        if (scanf("%d", &op) != 1) {
            limparBuffer();
            op = -1; 
        } else {
            limparBuffer();
        }

        switch (op) {
            case 1: menuRelatoriosClientes(); break;
            case 2: menuRelatoriosProdutos(); break;
            case 3: menuRelatoriosVendas(); break;
            case 4: break;
            default: printf("%sOpcao invalida.%s\n", vermelhoErro, RESET); pausar(); break;
        }
    } while (op != 4);
}

void menuRelatoriosClientes() {
    int op;
    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|            RELATORIOS DE CLIENTES              |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" 1. Listagem de Clientes (A-Z)\n");
        printf(" 2. Clientes que Compraram (Periodo)\n");
        printf(" 3. Voltar\n");
        printf("%s--------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sOpcao:%s ", VAMPIRE_BITE, RESET);

        if (scanf("%d", &op) != 1) { limparBuffer(); op = -1; } else { limparBuffer(); }

        switch (op) {
            case 1: relatorioClientesAlfa(); break;
            case 2: relatorioClientesPorPeriodo(); break;
            case 3: break;
            default: printf("%sOpcao invalida.%s\n", vermelhoErro, RESET); pausar(); break;
        }
    } while (op != 3);
}

void menuRelatoriosProdutos() {
    int op;
    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|             RELATORIOS DE PRODUTOS             |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" 1. Listagem de Produtos (A-Z)\n");
        printf(" 2. Produtos com Estoque Baixo\n");
        printf(" 3. Produtos Mais Vendidos\n");
        printf(" 4. Voltar\n");
        printf("%s--------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sOpcao:%s ", VAMPIRE_BITE, RESET);

        if (scanf("%d", &op) != 1) { limparBuffer(); op = -1; } else { limparBuffer(); }

        switch (op) {
            case 1: relatorioProdutosAlfa(); break;
            case 2: relatorioEstoqueMinimo(); break;
            case 3: relatorioProdutosMaisVendidos(); break;
            case 4: break;
            default: printf("%sOpcao invalida.%s\n", vermelhoErro, RESET); pausar(); break;
        }
    } while (op != 4);
}

void menuRelatoriosVendas() {
    int op;
    do {
        limparTela();
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf("%s|              RELATORIOS DE VENDAS              |%s\n", PURPLE_PHANTOM, RESET);
        printf("%s==================================================%s\n", MIDNIGHT_HOWL, RESET);
        printf(" 1. Listagem das Vendas (Periodo)\n");
        printf(" 2. Faturamento Consolidado (Periodo)\n");
        printf(" 3. Voltar\n");
        printf("%s--------------------------------------------------%s\n", MIDNIGHT_HOWL, RESET);
        printf(" %sOpcao:%s ", VAMPIRE_BITE, RESET);

        if (scanf("%d", &op) != 1) { limparBuffer(); op = -1; } else { limparBuffer(); }

        switch (op) {
            case 1: relatorioVendasPeriodo(); break;
            case 2: relatorioFaturamentoConsolidado(); break; 
            case 3: break;
            default: printf("%sOpcao invalida.%s\n", vermelhoErro, RESET); pausar(); break;
        }
    } while (op != 3);
}
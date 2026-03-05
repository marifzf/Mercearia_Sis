#ifndef BERE_H
#define BERE_H

/*----- INCLUSAO DE BIBLIOTECAS PADRAO -----*/
#include <stdio.h>              
#include <stdlib.h>             
#include <string.h>             
#include <math.h>               
#include <time.h>               
#include "cores.h"              // Inclui o arquivo de cores

/*----- CONSTANTES DE LIMITE DO SISTEMA -----*/
#define MAX_USUARIOS 50         
#define MAX_CLIENTES 100        
#define MAX_PRODUTOS 200        
#define MAX_VENDAS 500          
#define MAX_PAGAMENTOS 1000     
#define MAX_CATEGORIAS 50       // Limite para categorias

/*----- ESTRUTURAS DE DADOS -----*/

/* Estrutura de Categoria de Produto */
typedef struct {
    int id;                     // ID unico da categoria
    char nome[51];              // Nome da categoria (max 50 chars + '\0')
} CategoriaProduto;

/* Estrutura de Usuario (Login) */
typedef struct {
    char login[50];             
    char senha[50];              
    int tipo;                   // 1: Admin, 2: Usuario Padrao
} Usuario;

/* Estrutura de Cliente */
typedef struct {
    int codigo;                 
    char nomeCompleto[100];     
    char nomeSocial[100];       
    char cpf[15];               
    char ruaNumero[100];        
    char bairro[100];           
    char celularWhats[16];      
} Cliente;

/* Estrutura de Produto */
typedef struct {
    int codigo;                 
    char descricao[100];        
    char categoria[50];         // Armazena o nome da categoria
    float precoCompra;          
    float margemLucro;          
    float precoVenda;           
    int quantidadeEstoque;      
    int estoqueMinimo;          
} Produto;

/* Estrutura de Item de Venda (Produto dentro de uma venda/carrinho) */
typedef struct {
    int produtoCodigo;
    char descricao[100];        
    int quantidade;
    float precoUnitario;
    float totalItem;            
} ItemVenda;

/* Estrutura de Pagamento */
typedef struct {
    int idVenda;                // Num. Venda
    float valor;                // Valor pago
    char dataPagamento[11];     // Data do pagamento
    char tipo[3];               // Tipos: d-Dinheiro, c-Cartao, md? Misto Dinheiro, mc? Misto Cartao
} Pagamento;

/* Estrutura de Venda */
typedef struct {
    int idVenda;                // ID unico da venda (contador)
    int clienteCodigo;          // Cliente que efetuou a compra (0 se nao identificado)
    char dataVenda[11];         // Formato DD/MM/AAAA
    float totalVenda;           // Total da venda
    char status;                // 'A'berto, 'F'inalizada, 'C'ancelada
    int numItens;               // Numero de itens comprados
    ItemVenda *itens;           // Array dinamico de itens
} Venda;

/*----- VARIAVEIS GLOBAIS (Ponteiros para alocacao dinamica em bere.c) -----*/
// Todos os arrays dinamicos DEVE ser declarados como ponteiros (Type *)
extern Usuario *usuarios;       
extern int numUsuarios;
extern Usuario usuarioLogado; 
extern int idContadorVenda; 

extern Cliente *clientes;       
extern int numClientes;

extern Produto *produtos;       
extern int numProdutos;

extern ItemVenda* carrinho;
extern int itensNoCarrinho;
extern float totalCarrinho;

extern Venda *vendas;           
extern int numVendas;

extern Pagamento *pagamentos;   
extern int numPagamentos;

// Variaveis de Caixa
extern int caixaAberto;
extern float saldoAbertura;
extern float totalRetiradas;

// Variaveis de Categoria (NOVAS)
extern CategoriaProduto *categorias; // Array dinamico de categorias
extern int numCategorias;
extern int proximoCategoriaId; // Contador de ID para novas categorias


/*----- FUNCOES AUXILIARES / HELPERS -----*/
void limparBuffer();
void limparTela();
void pausar();
void obterDataAtual(char* dataString);
int compararDatas(const char *data1, const char *data2); 
int buscarProdutoPorCodigo(int codigo);     
int buscarClientePorCodigo(int codigo);     
int buscarUsuarioPorLogin(const char* login); 
int buscarVendaEmAberto();
int buscarVendaPorID(int id);               
int validarUsuarioAdmin();                  
void listarCarrinhoAtual();                 
float calcularTotalDinheiro();              
void listarClientesSimples();               
void listarVendasEmAberto();                

// Funcoes Auxiliares de Categoria (NOVAS)
void listarCategorias(); // Lista todas as categorias existentes
int buscarCategoriaPorId(int id); // Busca o ID da categoria no array
void cadastrarCategoriaPadrao(const char* nome); // Para inicializar categorias

/*----- FUNCOES DE SISTEMA E FLUXO -----*/
void inicializarSistema();              
int menuInicial();                      
void executarMenuPrincipal();           
int login();                            
void liberarMemoria();                  
void finalizarSistema();                

// Prototipos das funcoes de Menu
void menuCadastros();
void menuVendas();
void menuAberturaCaixa();               
void menuFechamentoCaixa();             
void menuRelatorios();
void menuRelatoriosClientes();
void menuRelatoriosProdutos();
void menuRelatoriosVendas();

/*----- FUNCOES DE CADASTRO (CRUD) -----*/
void cadastrarUsuario();
void cadastrarProduto();
void cadastrarCliente();
void excluirCliente();
void excluirProduto();
// void alterarProduto();          // << REMOVIDO PARA CORRIGIR ERRO DE LINKAGEM
void adicionarEstoque();
void cadastrarCategoria();


/*----- FUNCOES DE VENDA E CAIXA -----*/
void novaVenda();                   
void retiradaCaixa();               
void pagamento();                   
void registrarPagamento(int idVenda, float valor, const char* tipo); 
void abrirCaixa();                  
void fecharCaixa();                 


/*----- LISTAGENS EM MEMORIA -----*/
void listaProdutos(); 
void listaClientes();
void listaVendas();


/*----- RELATORIOS AVANCADOS (Funcoes de Processamento) -----*/
void relatorioClientesAlfa();
void relatorioClientesPorPeriodo(); 
void relatorioProdutosAlfa();
void relatorioEstoqueMinimo();
void relatorioProdutosMaisVendidos();
void relatorioVendasPeriodo();
void relatorioFaturamentoConsolidado();

// Funcoes de Estoque (Gerais)
void relatorioEstoqueArquivoDat();
void atualizarEstoqueArquivoDat();


/*----- PERSISTENCIA DE DADOS (Arquivos Binarios) -----*/
void carregarDadosPadrao();
void carregarDados();
void salvarDados();

// Funcoes de Persistencia Individual
void carregarUsuarios();
void salvarUsuarios();
void carregarClientes();
void salvarClientes();
void carregarProdutos();
void salvarProdutos();
void carregarVendas();
void salvarVendas();
void carregarPagamentos(); 
void salvarPagamentos();   
void carregarCategorias(); // << CORRIGIDO
void salvarCategorias();   // << CORRIGIDO

#endif
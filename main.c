// main.c
#include "bere.h"

int main() {
    inicializarSistema(); 
    
    // Captura o status: 1 = Sucesso (Login), 0 = Sair
    int statusLogin = menuInicial(); 

    if (statusLogin == 1) { 
        executarMenuPrincipal(); // Chama o menu principal do sistema
    }
    
    liberarMemoria(); 
    finalizarSistema();
    return 0;
}
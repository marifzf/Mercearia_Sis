# 🛒 Sistema de Gestão Dona Berê

Este é um sistema de automação comercial desenvolvido em **linguagem C**. [cite_start]O projeto foi criado para gerenciar operações de vendas, estoque de produtos, cadastro de clientes e controle financeiro de uma microempresa, com foco em robustez e persistência de dados[cite: 10, 11, 14, 15].

---

## 🚀 Funcionalidades

### 📦 Cadastros (CRUD)
- [cite_start]**Produtos**: Registro detalhado com descrição, categoria, preço de custo, margem de lucro, preço de venda automático e controle de estoque mínimo[cite: 308, 312, 315].
- [cite_start]**Clientes**: Cadastro com Nome, CPF e informações de contato[cite: 292, 296, 302].
- [cite_start]**Categorias**: Organização dinâmica de produtos (Ex: Alimento, Limpeza)[cite: 287, 289, 290].
- [cite_start]**Usuários**: Sistema de login com níveis de acesso (Administrador e Padrão)[cite: 284, 286].

### 💰 Vendas e Carrinho
- [cite_start]Carrinho dinâmico de compras com reserva de estoque[cite: 329, 331, 332].
- [cite_start]Vinculação obrigatória de cliente por venda[cite: 334, 337, 338].
- [cite_start]Pagamentos flexíveis: Dinheiro, Cartão ou Misto[cite: 366, 368, 370].

### 🏦 Controle Financeiro
- [cite_start]Abertura e fechamento de caixa com conferência de valores[cite: 403, 405, 421].
- [cite_start]Registro de sangrias (retiradas) com manutenção de fundo de troco[cite: 404].
- [cite_start]Detecção automática de divergências de saldo no fechamento[cite: 360, 361].

### 📊 Relatórios Avançados
- [cite_start]Listagem alfabética de clientes e produtos[cite: 376, 380].
- [cite_start]Ranking de produtos mais vendidos por período[cite: 386, 388].
- [cite_start]Histórico consolidado de faturamento e vendas por categoria[cite: 349, 350, 372].

---

## 🛠️ Tecnologias Utilizadas

- [cite_start]**Linguagem C**: Lógica central e manipulação de memória dinâmica.
- [cite_start]**Persistência em Arquivos Binários (`.dat`)**: Os dados são salvos e carregados automaticamente entre as execuções.
- [cite_start]**Cores ANSI**: Interface de terminal customizada para melhor usabilidade[cite: 1, 268].

---

## ⚙️ Como Executar

1. Certifique-se de ter um compilador C (como o GCC) instalado.
2. Clone o repositório:
   ```bash
   git clone [https://github.com/SEU_USUARIO/dona-bere.git](https://github.com/SEU_USUARIO/dona-bere.git)

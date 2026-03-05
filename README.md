# 🛒 Sistema de Gestão Dona Berê

Este é um sistema de automação comercial desenvolvido em **linguagem C**. O projeto foi criado para gerenciar operações de vendas, estoque de produtos, cadastro de clientes e controle financeiro de uma microempresa, com foco em robustez e persistência de dados.

---

## 🚀 Funcionalidades

### 📦 Cadastros (CRUD)
- **Produtos**: Registro detalhado com descrição, categoria, preço de custo, margem de lucro, preço de venda automático e controle de estoque mínimo
- **Clientes**: Cadastro com Nome, CPF e informações de contato
- **Categorias**: Organização dinâmica de produtos (Ex: Alimento, Limpeza)
- **Usuários**: Sistema de login com níveis de acesso (Administrador e Padrão

### 💰 Vendas e Carrinho
- Carrinho dinâmico de compras com reserva de estoque
- Vinculação obrigatória de cliente por venda
- Pagamentos flexíveis: Dinheiro, Cartão ou Misto

### 🏦 Controle Financeiro
- Abertura e fechamento de caixa com conferência de valores.
- Registro de sangrias (retiradas) com manutenção de fundo de troco
- Detecção automática de divergências de saldo no fechamento

### 📊 Relatórios Avançados
- Listagem alfabética de clientes e produtos
- Ranking de produtos mais vendidos por período
- Histórico consolidado de faturamento e vendas por categoria

---

## 🛠️ Tecnologias Utilizadas

- **Linguagem C**: Lógica central e manipulação de memória dinâmica.
- **Persistência em Arquivos Binários (`.dat`)**: Os dados são salvos e carregados automaticamente entre as execuções.
- **Cores ANSI**: Interface de terminal customizada para melhor usabilidade

---

## 📸 Demonstração
<img width="981" height="246" alt="image" src="https://github.com/user-attachments/assets/d725d394-563c-4afb-8c4a-ae6c9300bed0" />

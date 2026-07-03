# Projeto SO

Sistema cliente-servidor em C para gestão de documentos.

---

## 🗂️ Estrutura do Repositório

* **`/src`**: Ficheiros de código-fonte em C (processos do servidor, cliente e lógicas auxiliares).
* **`/include`**: Ficheiros de cabeçalho (`.h`) com as declarações de estruturas, mensagens e funções.
* **`makefile`**: Ficheiro para a compilação automatizada do projeto.

---

## 🚀 Como Executar o Programa

Este projeto foi desenvolvido em **C** num ambiente Linux/Unix. Certifica-te de que tens o compilador `gcc` e o utilitário `make` instalados.

Abre o teu terminal na pasta raiz do projeto (onde se encontra o `makefile`) e segue os passos:

### 1️⃣ Compilar o código
Antes de executar, compila todos os ficheiros do projeto com o comando:
```bash
make

2️⃣ Iniciar o servidor

Após a compilação concluir sem erros, inicia o processo do servidor no terminal:
Bash

./dserver

3️⃣ Iniciar o cliente

Abre um novo terminal (mantendo o do servidor a correr), navega até à mesma pasta do projeto e executa o cliente:
Bash

./dcliente

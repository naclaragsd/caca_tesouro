# Caça ao Tesouro 1x1

Jogo de caça ao tesouro para dois jogadores em C, com comunicação entre processos por **sockets (winsock2)** e **threads (pthreads)**.

Trabalho 02 da disciplina de **Sistemas Operacionais**, Engenharia de Software (UniCesumar), 2º bimestre de 2026.

> 🚧 Em desenvolvimento

---

## Como funciona o jogo

Cada jogador tem um **mapa secreto 6x6** com tesouros, armadilhas e água espalhados aleatoriamente. Um jogador não vê o mapa do outro.

Na sua vez, o jogador escolhe uma casa do mapa do adversário (ex.: `C4`). O programa do adversário consulta o próprio mapa e responde:

| Resposta | Significado | Efeito |
| --- | --- | --- |
| **T** | Tesouro | O jogador marca um ponto |
| **A** | Água | Vem uma dica: a distância até o tesouro mais próximo |
| **X** | Armadilha | O jogador perde a próxima vez |

Ganha quem encontrar primeiro todos os tesouros do adversário.

### Por que dois processos?

Cada processo guarda **apenas o seu próprio mapa** na memória. Para descobrir o que existe numa casa do mapa adversário, o jogador precisa **enviar uma mensagem** e esperar a resposta do outro processo. A troca de mensagens faz parte da regra do jogo.

---

## Notação das mensagens

Toda mensagem tem tamanho fixo de **64 bytes**. O primeiro caractere é sempre o número do jogador que envia (`1` ou `2`).

| Mensagem | Significado |
| --- | --- |
| `1C4` | Jogador 1 escolheu a casa C4 |
| `2RT` | Jogador 2 responde: tesouro |
| `2RA2` | Jogador 2 responde: água, tesouro mais próximo a 2 casas |
| `2RX` | Jogador 2 responde: armadilha |
| `1V` | Jogador 1 encontrou todos os tesouros e venceu |
| `1S` | Jogador 1 saiu da partida |

A variável `vez` existe nos dois processos e é atualizada pelas mensagens: só quem tem a vez pode enviar uma jogada, e a resposta passa a vez para o outro jogador.

*(A notação pode ser ajustada durante o desenvolvimento.)*

---

## Técnicas de Sistemas Operacionais utilizadas

- **Troca de mensagens:** comunicação cliente/servidor via sockets TCP (`winsock2.h`).
- **Threads:** uma thread fica recebendo mensagens enquanto a thread principal lê o teclado e desenha o mapa (`pthread.h`).
- **Exclusão mútua:** o estado do jogo (vez, mapas, placar) é protegido por `pthread_mutex_t`.
- **Sincronização:** a thread principal espera a resposta com `pthread_cond_t` (dorme até ser acordada) em vez de ficar em espera ocupada.
- **Gerenciamento de memória:** o mapa é alocado com `malloc` e liberado com `free` ao final da partida.

---

## Estrutura do projeto

```
caca_tesouro/
├── main.c      # menu e laço principal do jogo
├── rede.c      # conexão e troca de mensagens (sockets)
├── mapa.c      # criação, sorteio e desenho do mapa
└── README.md
```

---

## Como compilar e executar

**Requisitos:** Windows e Dev-C++ (Embarcadero) ou outro compilador MinGW.

1. Abra o projeto no Dev-C++.
2. Em *Projeto > Opções do Projeto > Parâmetros*, no campo **Linker**, adicione:
   ```
   -lws2_32 -lpthread
   ```
3. Compile (F9).

**Para jogar:**

1. Execute o programa e escolha `1 - Criar partida (servidor)`.
2. Execute o programa novamente (outra janela ou outro computador) e escolha `2 - Entrar em partida (cliente)`.
3. Informe o IP do servidor: `127.0.0.1` no mesmo computador, ou o IP mostrado pelo `ipconfig` em outro computador da mesma rede.

> Na primeira execução, o Firewall do Windows pode pedir permissão. Clique em **Permitir acesso**.

---

## Andamento

- [ ] Conexão entre servidor e cliente
- [ ] Mapa: criação, sorteio e desenho
- [ ] Jogada e resposta pela rede
- [ ] Controle da vez
- [ ] Threads, mutex e variável de condição
- [ ] Dicas, armadilhas, placar e fim de jogo
- [ ] Vídeo de gameplay

---

## Autoras

- Ana Clara Gomes De Andrade 
- Heloysa Fernandes Soares

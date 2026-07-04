# Trabalho de Criptografia — Toro $T_2$ sobre $\mathbb{F}_{p^2}$

Implementação autoral em C++ de dois criptossistemas construídos sobre o mesmo
substrato algébrico: o corpo de extensão quadrática $\mathbb{F}_{p^2}$ e o toro
algébrico $T_2$ (círculo unitário $a^2 + b^2 \equiv 1$), com $p \equiv 3 \pmod 4$.

- **Parte I — Chave pública:** Diffie–Hellman e ElGamal sobre um subgrupo cíclico
  de ordem prima $q \mid (p+1)$ contido em $T_2 \subset \mathbb{F}_{p^2}^{*}$.
- **Parte II — Chave simétrica:** cifra de bloco original (rede SPN) sobre
  $\mathbb{F}_{p^2}$, reaproveitando a aritmética da Parte I. A camada de
  substituição usa inversão de corpo (como o AES) envolvida por rotação no toro,
  dilatação, conjugação (Frobenius) e um mapa afim.

Toda a aritmética de grupo, os protocolos, a cifra e o teste de primalidade
(Miller–Rabin) são de implementação própria. A biblioteca **GMP** é usada
exclusivamente para inteiros de precisão arbitrária.


---

## Estrutura do projeto

| Arquivo                 | Conteúdo                                                                                     |
| ----------------------- | -------------------------------------------------------------------------------------------- |
| `toro.hpp` / `toro.cpp` | Núcleo compartilhado: aritmética modular de inteiros, corpo $\mathbb{F}_{p^2}$ (struct `Elemento`), norma, Miller–Rabin e geração de primos. |
| `cifra_assimetrica.cpp` | **Parte I.** Geração de parâmetros $(p, q, g)$, Diffie–Hellman e ElGamal sobre $T_2$. `main` interativa. |
| `cifra_simetrica.cpp`   | **Parte II.** SPN sobre $\mathbb{F}_{p^2}$: S-box afim, difusão em cascata, key schedule e a bateria de testes. |

Cada `.cpp` com `main` é um executável independente e ambos linkam com `toro.cpp`.

---

## Dependências

- **g++** com suporte a C++17
- **GMP** com a interface C++ (`gmpxx.h`) — pacote de desenvolvimento

Instalação no Ubuntu / WSL2 (Debian-based):

```bash
sudo apt-get update
sudo apt-get install build-essential libgmp-dev
```

> O pacote crucial é o `libgmp-dev`: apenas a `libgmp` de runtime **não basta**,
> pois faltam o cabeçalho `gmpxx.h` e a `libgmpxx`.

---

## Compilação

Compile cada parte ligando o núcleo `toro.cpp` e as bibliotecas da GMP
(`-lgmpxx -lgmp`, nesta ordem):

```bash
# Parte I — Diffie–Hellman + ElGamal
g++ -O2 -std=c++17 toro.cpp cifra_assimetrica.cpp -o parte1 -lgmpxx -lgmp

# Parte II — cifra simétrica SPN
g++ -O2 -std=c++17 toro.cpp cifra_simetrica.cpp -o parte2 -lgmpxx -lgmp
```

A flag `-O2` é recomendada para os benchmarks (os tempos do relatório assumem
`g++ -O2`).


## Execução

### Parte I — `./parte1`

Programa interativo com menu:

```
0: Sair
1: Diffie-Hellman
2: ElGamal
```

- **Diffie–Hellman:** pede os bits de $p$ e de $q$, gera os parâmetros e permite
  escolher as chaves privadas manualmente (`s`) ou sorteá-las (`n`). Ao final,
  verifica se os segredos de Alice e Bob coincidem.
- **ElGamal:** pede os bits de $p$ e de $q$, gera as chaves, cifra uma mensagem
  de teste (uma potência do gerador, garantindo pertencimento ao grupo) e
  confirma a recuperação.

> **Restrição de parâmetros:** é exigido `bits_p > bits_q + 2`.

Exemplo de sessão rápida (parâmetros pequenos, só para demonstração):

```
1        # Diffie-Hellman
256      # bits de p
64       # bits de q
n        # sortear as chaves privadas
```

### Parte II — `./parte2`

Pede apenas os bits do primo $p$ (ele já é gerado com $p \equiv 3 \pmod 4$
automaticamente) e então executa toda a bateria de testes de forma automática:

```
256      # bits do primo p
```

O programa roda, em sequência:

1. **Teste de texto conhecido** — cifra e decifra uma string, mostrando o bloco e
   confirmando a recuperação; inclui também a decifração com chave **errada**.
2. **Ida-e-volta** — 1000 blocos aleatórios cifrados e decifrados (espera-se
   `1000/1000`).
3. **Desempenho** — tempo médio de cifração e decifração por bloco.
4. **Avalanche** — vira 1 bit da entrada e mede a fração de bits alterados na
   saída (200 tentativas; ideal $\approx 50\%$).
5. **Homogeneidade** — verifica se a decifração com chave errada é um múltiplo
   escalar da mensagem (100 tentativas; após o passo afim, espera-se `0`).
6. **Otimização** — compara a decifração antes/depois da pré-computação de
   $c^{-1}$.

---

## Parâmetros recomendados

Os dois primos são dimensionados de forma independente, pois enfrentam ataques
distintos ($q$ contra ataques genéricos; $p$ contra cálculo de índice no corpo):

| Uso                          | bits de $p$ | bits de $q$ |
| ---------------------------- | :---------: | :---------: |
| Demonstração rápida          |     256     |     64      |
| Testes intermediários        |     512     |     128     |
| Nível de segurança realista  |    1536     |     256     |

Na Parte II, o tamanho do bloco acompanha $p$: são $8\lceil \log_2 p \rceil$ bits
por bloco (2048 bits para $p$ de 256 bits).

---

## Ambiente testado

- **SO:** Ubuntu 24.04 (WSL2)
- **Compilador:** g++ 13.3.0, C++17
- **GMP:** 6.3.0
- **Hardware de referência dos benchmarks:** AMD Ryzen 5 7600X

---

## Observações

- A fonte de aleatoriedade (RNG da GMP semeado pelo relógio / pela chave) é
  adequada para testes, mas **não** é criptograficamente segura — vide as
  limitações discutidas no relatório.
- A cifra da Parte II processa um único bloco; o tratamento de mensagens maiores
  exigiria um modo de operação.
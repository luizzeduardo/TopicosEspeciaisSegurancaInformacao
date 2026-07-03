#ifndef TORO_HPP
#define TORO_HPP

#include <gmpxx.h>

using ll = mpz_class;

// Máquina de aleatórios (definida no toro.cpp)
extern gmp_randstate_t sorteador;

//========================================================
//  ARITMÉTICA MODULAR DE INTEIROS
//========================================================

ll mod(ll x, ll p);
ll power_int(ll base, ll exp, ll p);
ll inv_mod_int(ll k, ll p);

//========================================================
//  ELEMENTOS DO CORPO F_{p^2} E SUAS OPERAÇÕES
//========================================================

struct Elemento {
    ll a;
    ll b;
    Elemento();
    Elemento(ll ai, ll bi);
};

Elemento soma(Elemento x, Elemento y, ll p);
Elemento sub(Elemento x, Elemento y, ll p);
Elemento mul(Elemento x, Elemento y, ll p);
Elemento power(Elemento base, ll exp, ll p);
Elemento inv_mod(Elemento x, ll p);
ll norma(Elemento x, ll p);

//========================================================
//  PRIMALIDADE E GERAÇÃO
//========================================================


bool miller_rabin(ll n, ll rodadas);
ll gera_primo(int bits);
ll gera_primo_3mod4(int bits);

#endif
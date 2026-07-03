#include "toro.hpp"

gmp_randstate_t sorteador; 

//========================================================
//  ARITMÉTICA MODULAR
//========================================================


ll mod(ll x, ll p){
    ll r = x % p;
    if (r < 0) r += p;
    return r;
}


ll power_int(ll base, ll exp, ll p){
    ll resultado = 1;
    base = mod(base, p);
    while(exp > 0){
        if(exp % 2 == 1) resultado = mod(resultado * base, p);
        base = mod(base * base, p);
        exp /= 2;
    }
    return resultado;
}


ll inv_mod_int(ll k, ll p) {
    return power_int(k, p - 2, p);
}


//========================================================
//  ELEMENTOS DO CORPO F_{p^2} E SUAS OPERAÇÕES
//========================================================

Elemento::Elemento() : a(0), b(0) {}
Elemento::Elemento(ll ai, ll bi) : a(ai), b(bi) {}


Elemento soma(Elemento x, Elemento y, ll p) {
    return Elemento(mod(x.a + y.a, p), mod(x.b + y.b, p));
}


Elemento sub(Elemento x, Elemento y, ll p) {
    return Elemento(mod(x.a - y.a, p), mod(x.b - y.b, p));
}


Elemento mul(Elemento x, Elemento y, ll p){
    ll real = mod(x.a * y.a - x.b * y.b, p);
    ll imag = mod(x.a * y.b + x.b * y.a, p);
    return Elemento(real, imag);
}


Elemento power(Elemento base, ll exp, ll p) {
    Elemento resultado(1, 0);
    while (exp > 0) {
        if (exp % 2 == 1) resultado = mul(resultado, base, p);
        base = mul(base, base, p);
        exp >>= 1;
    }
    return resultado;
}


Elemento inv_mod(Elemento x, ll p) {
    return Elemento(x.a, mod(-x.b, p));
}


ll norma(Elemento x, ll p){
    return mod(x.a * x.a + x.b * x.b, p);
}


//========================================================
//  PRIMALIDADE E GERAÇÃO
//========================================================


bool miller_rabin(ll n, ll rodadas){
    if(n < 2) return false;
    if(n == 2 || n == 3) return true;
    if(n % 2 == 0) return false;

    ll d = n - 1, s = 0;
    while(d % 2 == 0){ d /= 2; s++; }

    for (int i = 0; i < rodadas; i++) {
        ll a, x;
        ll limite = n - 3;
        mpz_urandomm(a.get_mpz_t(), sorteador, limite.get_mpz_t());
        a += 2;
        x = power_int(a, d, n);
        if(x == 1 || x == n - 1) continue;

        bool encontrou = false;
        for (ll r = 1; r < s; r++) {
            x = mod(x * x, n);
            if(x == n - 1){ encontrou = true; break; }
        }
        if(!encontrou) return false;
    }
    return true;
}


ll gera_primo(int bits){
    ll candidato;
    while(true){
        mpz_urandomb(candidato.get_mpz_t(), sorteador, (mp_bitcnt_t) bits);
        mpz_setbit(candidato.get_mpz_t(), (mp_bitcnt_t)(bits - 1));
        mpz_setbit(candidato.get_mpz_t(), 0);
        if(miller_rabin(candidato, 40)) return candidato;
    }
}


ll gera_primo_3mod4(int bits){
    ll candidato;
    while(true){
        mpz_urandomb(candidato.get_mpz_t(), sorteador, (mp_bitcnt_t) bits);
        mpz_setbit(candidato.get_mpz_t(), (mp_bitcnt_t)(bits - 1)); 
        mpz_setbit(candidato.get_mpz_t(), 0);
        mpz_setbit(candidato.get_mpz_t(), 1);  
        if(miller_rabin(candidato, 40)) return candidato;
    }
}
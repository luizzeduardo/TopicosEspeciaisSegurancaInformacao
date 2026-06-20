#include <bits/stdc++.h>
#include <gmpxx.h>
using ll = mpz_class; //inteiro de precisão arbitraria

//======================================================================================
//-=-==-=-=-=-=-=-=-=- Funções das Operações Modulares e Complexas -=-=-=-=-=-=-=-=-=-=-
//======================================================================================

ll mod(ll x, ll p){
    ll r = x % p;
    if (r < 0){
        r += p;
    }
    return r;
}

struct Elemento {
    ll a;
    ll b;

    Elemento() : a(0), b(0) {} // valor padrão
    Elemento(ll ai, ll bi) : a(ai), b(bi) {} // passagem de parametros
};

Elemento mul(Elemento x, Elemento y, ll p){
    ll real = mod(x.a * y.a - x.b * y.b, p);
    ll imag = mod(x.a * y.b + x.b * y.a, p);
    return Elemento(real, imag);
}


// usando exponenciação binaria
Elemento power(Elemento base, ll exp, ll p) {
    Elemento resultado(1, 0); // neutro

    while (exp > 0) {
        if (exp%2 == 1) {
            resultado = mul(resultado, base, p);
        }

        base = mul(base, base, p);
        exp >>= 1;
    }
    return resultado;
}


Elemento inv_mod(Elemento x, ll p) {
    return Elemento(x.a, mod(-x.b, p));
}

// (subgrupo do toro) se, e somente se, sua norma é 1.
ll norma(Elemento x, ll p){
    return mod(x.a*x.a + x.b*x.b, p);
}



int main() {

}
#include <bits/stdc++.h>
#include <gmpxx.h>
using namespace std;
using ll = mpz_class; //inteiro de precisão arbitraria
gmp_randstate_t gerador; // gerador de nums 'aleatorios'


//=======================================================================================
//=-=-=-=-=-=-=-=-=-=-= Funções das Operações Modulares e Complexas -=-=-=-=-=-=-=-=-=-=-
//=======================================================================================
 

ll mod(ll x, ll p){
    ll r = x % p;
    if (r < 0){
        r += p;
    }
    return r;
}


// usando exponenciação binaria
ll power_int(ll base, ll exp, ll p){
    ll resultado = 1;
    base = mod(base, p);
    while(exp>0){
        if(exp%2==1){
            resultado = mod(resultado*base, p);
        }
        base = mod(base*base, p);
        exp/=2;
    }
    return resultado;
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


// mesma coisa do power_int
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


/**
 * Algortimo Miller Rabin para verificar se o numero é primo ou não
 */
bool miller_rabin(ll n, ll rodadas){
    //casos triviais
    if(n<2) return false;
    if( n==2 || n==3) return true;
    if(n%2==0) return false;

    //fatorar n-1, no fator
    ll d = n-1;
    ll s = 0;
    while(d%2==0){
        d/=2;
        s++;
    }
    // n-1 = 2^s . d


    // testar rodadas testemunhas
    for (int i = 0; i < rodadas; i++) {
        ll a, x;

        // sortear a em [2, n-2]
        ll limite = n - 3;
        mpz_urandomm(a.get_mpz_t(), gerador, limite.get_mpz_t());
        a += 2;

        // x = a^d mod n
        x = power_int(a, d, n);

        if(x==1 || x==n-1){
            continue;
        }

        bool encontrou = false;

        for (ll r = 1; r < s; r++) {
            x = mod(x * x, n); 
            if(x == n-1){
                encontrou = true;
                break;
            }
        }

        if(!encontrou){
            return false;
        }

    }

    // passou em todas as testemunhas
    return true;
}




int main() {

    gmp_randinit_default(gerador); // instancia o gerador 'aleatorio'
    gmp_randseed_ui(gerador, time(nullptr));


    int operacao = 1;
    do {
        cout << "Escolha:\n";
        cout << "0: Sair\n";
        cout << "1: Diffie Helman\n";
        cout << "2: ElGammal\n";
        cin >> operacao;
    
        if (operacao == 1) {
            cout << "em desenvolvimento..." ;
        } else if (operacao == 2) {
            cout << "em desenvolvimento...";
        }
        cout << endl << "só o 0 funciona kakakakkakak";
    } while (operacao);

}
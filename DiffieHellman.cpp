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


/**
 * Gerador de primos com quant de bits parametrizável
 */
ll gera_primo(int bits){
    ll candidato;
    while(true){
        mpz_urandomb(candidato.get_mpz_t(), gerador, (mp_bitcnt_t) bits);
        mpz_setbit(candidato.get_mpz_t(), (mp_bitcnt_t) (bits - 1)); // garantir bit mais relevante 1
        mpz_setbit(candidato.get_mpz_t(), 0); //garantir numero impar

        if(miller_rabin(candidato, 40)) return candidato;
    }
}


/**
 * Gera os paramtros p e q, recebendo o numero de bits de ambos respectivamente
 */
pair<ll, ll> gera_parametros(int bits_p, int bits_q) {

    if (bits_p <= bits_q + 2) {
        cerr << "Erro: bits_p deve ser maior que bits_q + 2\n";
        exit(1);
    }

    ll q = gera_primo(bits_q);


    int bits_m = bits_p - bits_q - 2;
    ll m, p;
    while (true) {
        mpz_urandomb(m.get_mpz_t(), gerador, (mp_bitcnt_t)bits_m);
        mpz_setbit(m.get_mpz_t(), (mp_bitcnt_t)(bits_m - 1));   // garantir o tamanho do m

        p = 4 * m * q - 1; // p = 3 mod 4
        if(miller_rabin(p, 40)) break;
    }

    return {p, q};
}


// qualquer gerador é igualmente bom, basta achar um, o q garante isso
Elemento acha_gerador(ll p, ll q) {
    ll expo = (p * p - 1) / q;
    while (true) {
        Elemento z;
        mpz_urandomm(z.a.get_mpz_t(), gerador, p.get_mpz_t());
        mpz_urandomm(z.b.get_mpz_t(), gerador, p.get_mpz_t());

        Elemento g = power(z, expo, p);

        if (!(g.a == 1 && g.b == 0)) return g;
    }
}


ll gera_privada(ll q) {
    ll privada;
    ll limite = q - 1;
    mpz_urandomm(privada.get_mpz_t(), gerador, limite.get_mpz_t());
    privada += 1; // [1, q)
    return privada;
}


ll chave_valida(const string& nome, ll q) {
    ll chave;
    while (true) {
        cout << "Chave privada de " << nome << " [1, " << (q-1) << "]: ";
        cin >> chave;

        if (chave >= 1 && chave < q) {
            return chave; // válida, retorna
        }
        cout << "  Invalida! Deve estar entre 1 e " << (q-1) << ".\n";
    }
}



pair<Elemento, Elemento> elgamal_cifra(Elemento M, Elemento H, Elemento g, ll p, ll q) {
    ll k = gera_privada(q); // nonce aleatorio
    Elemento c1 = power(g, k, p);
    Elemento mascara = power(H, k, p);
    Elemento c2 = mul(M, mascara, p);
    return {c1, c2};
}

Elemento elgamal_decifra(Elemento c1, Elemento c2, ll d, ll p) {
    Elemento mascara = power(c1, d, p);
    Elemento inversa = inv_mod(mascara, p);
    Elemento m =mul(c2, inversa, p);
    return m;
}



void diffie_hellman() {
    cout << "\n=== Diffie-Hellman sobre o toro T2 ===\n\n";

    int bits_p, bits_q;
    cout << "Bits de p: ";
    cin >> bits_p;
    cout << "Bits de q: ";
    cin >> bits_q;

    auto [p, q] = gera_parametros(bits_p, bits_q);
    Elemento g = acha_gerador(p, q);
    cout << "\nParametros gerados:\n";
    cout << "p = " << p << "\n";
    cout << "q = " << q << "\n";
    cout << "g = (" << g.a << ", " << g.b << ")\n\n";

    // escolher as chaves
    char escolher;
    cout << "Escolher chaves privadas manualmente? (s/n): ";
    cin >> escolher;

    ll a_priv, b_priv;
    if (escolher == 's') {
        a_priv = chave_valida("Alice", q);
        b_priv = chave_valida("Bob", q);
    } else {
        a_priv = gera_privada(q);
        b_priv = gera_privada(q);
        cout << "Chave privada de Alice: " << a_priv << endl;
        cout << "Chave privada de Bob: " << b_priv << endl;
    }

    Elemento a_pub = power(g, a_priv, p);
    Elemento b_pub = power(g, b_priv, p);
    Elemento seg_alice = power(b_pub, a_priv, p);
    Elemento seg_bob   = power(a_pub, b_priv, p);

    bool igual = (seg_alice.a == seg_bob.a && seg_alice.b == seg_bob.b);
    cout << "\nSegredos batem? " << (igual ? "SIM" : "NAO") << "\n";
    cout << "Segredo de Alice: " << seg_alice.a << " " << seg_alice.b << endl;
    cout << "Segredo de Bob: " << seg_bob.a << " " << seg_bob.b << endl;
}



void elgamal() {
    cout << "\n=== ElGamal sobre o toro T2 ===\n\n";

    int bits_p, bits_q;
    cout << "Bits de p: ";
    cin >> bits_p;
    cout << "Bits de q: ";
    cin >> bits_q;

    auto [p, q] = gera_parametros(bits_p, bits_q);
    Elemento g = acha_gerador(p, q);

    // Alice gera chaves
    ll d = gera_privada(q);          // privada
    Elemento H = power(g, d, p);     // pública

    // mensagem de teste: uma potência de g (garante que está no grupo)
    Elemento M = power(g, 42, p);

    cout << "\nMensagem original: (" << M.a << ", " << M.b << ")\n";

    auto [c1, c2] = elgamal_cifra(M, H, g, p, q);
    Elemento M_dec = elgamal_decifra(c1, c2, d, p);

    cout << "Mensagem decifrada: (" << M_dec.a << ", " << M_dec.b << ")\n";

    bool ok = (M.a == M_dec.a && M.b == M_dec.b);
    cout << "Recuperada? " << (ok ? "SIM" : "NAO") << "\n";
}


int main() {

    gmp_randinit_default(gerador); // instancia o gerador 'aleatorio'
    gmp_randseed_ui(gerador, (unsigned long)time(nullptr));

    int operacao = 1;
    do {
        cout << "\n==========================\n";
        cout << " Criptografia - Toro T2\n";
        cout << "==========================\n";
        cout << "Escolha:\n";
        cout << "0: Sair\n";
        cout << "1: Diffie Helman\n";
        cout << "2: ElGammal\n";
        cin >> operacao;
    
        if (operacao == 1) {
            diffie_hellman();
        } else if (operacao == 2) {
            elgamal();
        }
        else{
            cout << "Opção Invalida";
        }
    } while (operacao);

}
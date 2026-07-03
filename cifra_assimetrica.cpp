#include <bits/stdc++.h>
#include <gmpxx.h>
#include "toro.hpp"
using namespace std;

 

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
        mpz_urandomb(m.get_mpz_t(), sorteador, (mp_bitcnt_t)bits_m);
        mpz_setbit(m.get_mpz_t(), (mp_bitcnt_t)(bits_m - 1));   // garantir o tamanho do m

        p = 4 * m * q - 1; // p = 3 mod 4 e alem disso q|p
        if(miller_rabin(p, 40)) break;
    }

    return {p, q};
}


// qualquer gerador é igualmente bom, basta achar um, o q garante isso
Elemento acha_gerador(ll p, ll q) {
    ll expo = (p * p - 1) / q;
    while (true) {
        Elemento z;
        mpz_urandomm(z.a.get_mpz_t(), sorteador, p.get_mpz_t());
        mpz_urandomm(z.b.get_mpz_t(), sorteador, p.get_mpz_t());

        Elemento g = power(z, expo, p);

        if (!(g.a == 1 && g.b == 0)) return g;
    }
}


ll gera_privada(ll q) {
    ll privada;
    ll limite = q - 1;
    mpz_urandomm(privada.get_mpz_t(), sorteador, limite.get_mpz_t());
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

    cout << "\nMensagem encriptada: c1 = (" << c1.a << ", " << c1.b << "), c2 = (" << c2.a << ", " << c2.b << ")\n";

    Elemento M_dec = elgamal_decifra(c1, c2, d, p);

    cout << "\nMensagem decifrada: (" << M_dec.a << ", " << M_dec.b << ")\n";

    bool ok = (M.a == M_dec.a && M.b == M_dec.b);
    cout << "\nRecuperada? " << (ok ? "SIM" : "NAO") << "\n";
}


int main() {

    gmp_randinit_default(sorteador); // instancia o sorteador
    gmp_randseed_ui(sorteador, (unsigned long)time(nullptr));

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
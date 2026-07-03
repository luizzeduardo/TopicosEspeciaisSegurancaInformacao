#include <bits/stdc++.h>
#include <gmpxx.h>
#include "toro.hpp"
using namespace std;


gmp_randstate_t sorteador_chave;  // gerador do key schedule (semeado pela chave)


//=======================================================
// Aritmética do corpo F_{p^2} (reusada da Parte I)
//=======================================================


struct SubChave {
    Elemento k_theta;
    ll c;
};


Elemento rotacao(Elemento z, Elemento k_theta, ll p) {
    return mul(z, k_theta, p);
}


Elemento dilatacao(Elemento z, ll c, ll p) {
    ll nova_a = mod(z.a * c, p);
    ll nova_b = mod(z.b * c, p);
    return Elemento(nova_a, nova_b);
}


Elemento rotacao_inversa(Elemento z, Elemento k_theta, ll p) {
    Elemento conj(k_theta.a, mod(-k_theta.b, p));  // conjugado
    return mul(z, conj, p);
}


Elemento dilatacao_inversa(Elemento z, ll c, ll p) {
    ll c_inv = inv_mod_int(c, p);
    return dilatacao(z, c_inv, p);  // reusa a dilatação com c_inv
}


Elemento inverso(Elemento z, ll p) {
    ll N = norma(z, p);
    if (N == 0) return Elemento(0, 0);
    ll N_inv = inv_mod_int(N, p);
    ll nova_a = mod(z.a * N_inv, p);
    ll nova_b = mod(mod(-z.b, p) * N_inv, p);
    return Elemento(nova_a, nova_b);
}


Elemento f_mistura(Elemento z, ll p) {
    // multiplica por uma constante fixa pra embaralhar
    Elemento constante(12345, 67890);  // valor arbitrário fixo
    return mul(z, constante, p);
}


vector<Elemento> difusao_frente(vector<Elemento> bloco, ll p) {
    for (int i = 1; i < 4; i++) {
        bloco[i] = soma(bloco[i], f_mistura(bloco[i-1], p), p);
    }
    return bloco;
}


vector<Elemento> difusao_frente_inversa(vector<Elemento> bloco, ll p) {
    for (int i = 3; i >= 1; i--) {
        bloco[i] = sub(bloco[i], f_mistura(bloco[i-1], p), p);
    }
    return bloco;
}

vector<Elemento> difusao_tras(vector<Elemento> bloco, ll p) {
    for (int i = 2; i >= 0; i--) {
        bloco[i] = soma(bloco[i], f_mistura(bloco[i+1], p), p);
    }
    return bloco;
}

vector<Elemento> difusao_tras_inversa(vector<Elemento> bloco, ll p) {
    for (int i = 0; i <= 2; i++) {
        bloco[i] = sub(bloco[i], f_mistura(bloco[i+1], p), p);
    }
    return bloco;
}


vector<Elemento> uma_rodada(vector<Elemento> bloco, Elemento k_theta, ll c, ll p, int r) {
    for (int i = 0; i < 4; i++) {
        bloco[i] = rotacao(bloco[i], k_theta, p);
        bloco[i] = dilatacao(bloco[i], c, p);
        bloco[i] = inverso(bloco[i], p);
    }
    // alterna a direção da difusão conforme a rodada
    if (r % 2 == 0)
        bloco = difusao_frente(bloco, p);
    else
        bloco = difusao_tras(bloco, p);
    return bloco;
}

vector<Elemento> uma_rodada_inversa(vector<Elemento> bloco, Elemento k_theta, ll c, ll p, int r) {
    // desfaz a difusão da direção que foi usada nesta rodada
    if (r % 2 == 0)
        bloco = difusao_frente_inversa(bloco, p);
    else
        bloco = difusao_tras_inversa(bloco, p);
    for (int i = 0; i < 4; i++) {
        bloco[i] = inverso(bloco[i], p);
        bloco[i] = dilatacao_inversa(bloco[i], c, p);
        bloco[i] = rotacao_inversa(bloco[i], k_theta, p);
    }
    return bloco;
}

const int NUM_RODADAS = 10;


Elemento gera_norma1(Elemento w, ll p) {
    Elemento conj(w.a, mod(-w.b, p));   // conjugado de w
    Elemento conj_inv = inverso(conj, p);
    return mul(w, conj_inv, p);          // w / conj
}


vector<SubChave> key_schedule(ll chave, ll p) {
    // semeia o gerador do key schedule com a chave (DETERMINÍSTICO)
    gmp_randseed(sorteador_chave, chave.get_mpz_t());

    vector<SubChave> chaves;
    for (int r = 0; r < NUM_RODADAS; r++) {
        // gera um ponto w aleatório (do gerador semeado pela chave)
        Elemento w;
        mpz_urandomm(w.a.get_mpz_t(), sorteador_chave, p.get_mpz_t());
        mpz_urandomm(w.b.get_mpz_t(), sorteador_chave, p.get_mpz_t());

        // k_theta = ponto de norma 1
        Elemento k_theta = gera_norma1(w, p);

        // c: constante != 0
        ll c;
        mpz_urandomm(c.get_mpz_t(), sorteador_chave, p.get_mpz_t());
        if (c == 0) c = 1;   // garante c != 0

        chaves.push_back({k_theta, c});
    }
    return chaves;
}


vector<Elemento> cifrar(vector<Elemento> bloco, vector<SubChave> chaves, ll p) {
    for (int r = 0; r < NUM_RODADAS; r++) {
        bloco = uma_rodada(bloco, chaves[r].k_theta, chaves[r].c, p, r);
    }
    return bloco;
}


vector<Elemento> decifrar(vector<Elemento> bloco, vector<SubChave> chaves, ll p) {
    for (int r = NUM_RODADAS - 1; r >= 0; r--) {   // ordem reversa
        bloco = uma_rodada_inversa(bloco, chaves[r].k_theta, chaves[r].c, p, r);
    }
    return bloco;
}


// ---------- auxiliares ----------

void imprime_bloco(const string& rotulo, const vector<Elemento>& b) {
    cout << rotulo << ":\n";
    for (int i = 0; i < 4; i++)
        cout << "  m" << i << " = (" << b[i].a << ", " << b[i].b << ")\n";
}

// empacota uma string em 4 elementos de F_{p^2} (8 componentes < p)
vector<Elemento> texto_para_bloco(const string& s, ll p) {
    size_t nbits = mpz_sizeinbase(p.get_mpz_t(), 2);
    size_t cap = (nbits - 1) / 8; if (cap < 1) cap = 1;   // bytes por componente
    size_t capacidade = 8 * cap;
    vector<unsigned char> buf(capacidade, 0);
    size_t n = min(s.size(), capacidade);
    for (size_t i = 0; i < n; i++) buf[i] = (unsigned char)s[i];
    vector<Elemento> bloco(4);
    for (int comp = 0; comp < 8; comp++) {
        mpz_class v;
        mpz_import(v.get_mpz_t(), cap, 1, 1, 1, 0, &buf[comp * cap]);
        if (comp % 2 == 0) bloco[comp / 2].a = v; else bloco[comp / 2].b = v;
    }
    return bloco;
}

string bloco_para_texto(const vector<Elemento>& b, size_t len, ll p) {
    size_t nbits = mpz_sizeinbase(p.get_mpz_t(), 2);
    size_t cap = (nbits - 1) / 8; if (cap < 1) cap = 1;
    size_t maxb = (nbits + 7) / 8 + 1;               // cabe qualquer v < p
    vector<unsigned char> buf(8 * cap, 0), tmp(maxb, 0);
    for (int comp = 0; comp < 8; comp++) {
        const mpz_class& v = (comp % 2 == 0) ? b[comp/2].a : b[comp/2].b;
        size_t escritos = 0;
        if (v != 0) mpz_export(tmp.data(), &escritos, 1, 1, 1, 0, v.get_mpz_t());
        if (escritos <= cap)                          // normal: alinha à direita
            for (size_t j = 0; j < escritos; j++)
                buf[comp*cap + (cap - escritos) + j] = tmp[j];
        else                                          // maior que o slot: usa os bytes baixos
            for (size_t j = 0; j < cap; j++)
                buf[comp*cap + j] = tmp[escritos - cap + j];
    }
    string out((char*)buf.data(), 8 * cap);
    if (len < out.size()) out.resize(len);
    return out;
}

bool blocos_iguais(const vector<Elemento>& x, const vector<Elemento>& y) {
    for (int i = 0; i < 4; i++)
        if (!(x[i].a == y[i].a && x[i].b == y[i].b)) return false;
    return true;
}

int main() {
    gmp_randinit_default(sorteador);
    gmp_randinit_default(sorteador_chave);
    gmp_randseed_ui(sorteador, (unsigned long)time(nullptr));

    cout << "=== Cifra Simetrica SPN sobre F_{p^2} ===\n\n";
    int bits_p;
    cout << "Bits do primo p (ex: 256): ";
    cin >> bits_p;

    ll p = gera_primo_3mod4(bits_p);
    cout << "\np = " << p << "\n(p mod 4 = " << mod(p, 4) << ")\n\n";

    // chave e key schedule
    ll chave;
    mpz_urandomb(chave.get_mpz_t(), sorteador, (mp_bitcnt_t)bits_p);
    auto chaves = key_schedule(chave, p);
    cout << "Chave K = " << chave << "\n";
    cout << "Subchaves geradas: " << chaves.size() << " (uma por rodada)\n\n";

    // mensagem -> bloco
    string texto = "Cifra SPN autoral sobre o toro T2";
    cout << "--- Mensagem ---\nTexto: \"" << texto << "\"\n";
    auto M = texto_para_bloco(texto, p);
    imprime_bloco("Bloco original", M);

    // cifracao
    cout << "\n--- Cifracao ---\n";
    auto C = cifrar(M, chaves, p);
    imprime_bloco("Bloco cifrado", C);

    // decifracao com a chave correta
    cout << "\n--- Decifracao (chave correta) ---\n";
    auto D = decifrar(C, chaves, p);
    imprime_bloco("Bloco recuperado", D);
    cout << "Texto recuperado: \"" << bloco_para_texto(D, texto.size(), p) << "\"\n";
    cout << "Recuperou? " << (blocos_iguais(M, D) ? "SIM" : "NAO") << "\n";

    // decifracao com chave errada (mostra a dependencia da chave)
    cout << "\n--- Decifracao (chave ERRADA) ---\n";
    ll chave_errada;
    mpz_urandomb(chave_errada.get_mpz_t(), sorteador, (mp_bitcnt_t)bits_p);
    cout << "Chave K' = " << chave_errada << "\n";
    auto D2 = decifrar(C, key_schedule(chave_errada, p), p);
    imprime_bloco("Bloco recuperado", D2);
    string lixo = bloco_para_texto(D2, texto.size(), p);
    for (char& ch : lixo) if ((unsigned char)ch < 32 || (unsigned char)ch > 126) ch = '.';
    cout << "Texto recuperado: \"" << lixo << "\"\n";
    cout << "Recuperou? " << (blocos_iguais(M, D2) ? "SIM" : "NAO") << "\n";

    // verificacao em massa
    cout << "\n--- Verificacao (blocos aleatorios) ---\n";
    int N_test = 1000, ok = 0;
    for (int t = 0; t < N_test; t++) {
        vector<Elemento> R(4);
        for (auto& e : R) {
            mpz_urandomm(e.a.get_mpz_t(), sorteador, p.get_mpz_t());
            mpz_urandomm(e.b.get_mpz_t(), sorteador, p.get_mpz_t());
        }
        if (blocos_iguais(R, decifrar(cifrar(R, chaves, p), chaves, p))) ok++;
    }
    cout << ok << "/" << N_test << " round-trips corretos\n";

    // desempenho
    cout << "\n--- Desempenho (p = " << bits_p << " bits, bloco = " << 8*bits_p << " bits) ---\n";
    int N = bits_p <= 128 ? 3000 : (bits_p <= 256 ? 1000 : 300);
    auto t0 = chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { volatile auto x = cifrar(M, chaves, p); (void)x; }
    auto t1 = chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) { volatile auto x = decifrar(C, chaves, p); (void)x; }
    auto t2 = chrono::high_resolution_clock::now();
    double us_cif = chrono::duration_cast<chrono::nanoseconds>(t1 - t0).count() / 1000.0 / N;
    double us_dec = chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count() / 1000.0 / N;
    cout << fixed << setprecision(2);
    cout << "Cifracao  : " << us_cif << " us/bloco\n";
    cout << "Decifracao: " << us_dec << " us/bloco\n";
    return 0;
}
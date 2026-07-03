#include <bits/stdc++.h>
#include <gmpxx.h>
#include "toro.hpp"
using namespace std;


gmp_randstate_t sorteador_chave;  // gerador do key schedule (semeado pela chave)


//=======================================================
// Aritmetica do corpo F_{p^2} (reusada da Parte I)
//=======================================================


// Cada subchave de rodada: rotacao (k_theta, norma 1), dilatacao (c) e
// a chave aditiva rk (delta da S-box afim), um elemento por lane.
struct SubChave {
    Elemento k_theta;
    ll c;
    vector<Elemento> rk;
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
    return dilatacao(z, c_inv, p);  // reusa a dilatacao com c_inv
}


Elemento inverso(Elemento z, ll p) {
    ll N = norma(z, p);
    if (N == 0) return Elemento(0, 0);
    ll N_inv = inv_mod_int(N, p);
    ll nova_a = mod(z.a * N_inv, p);
    ll nova_b = mod(mod(-z.b, p) * N_inv, p);
    return Elemento(nova_a, nova_b);
}


// conjugado a + bi -> a - bi. Com p = 3 (mod 4) isto e o Frobenius (z^p)
// e coincide com a inversao no toro T_2.
Elemento conjugado(Elemento z, ll p) {
    return Elemento(z.a, mod(-z.b, p));
}


//=======================================================
// S-box afim (inversao + conjugacao + mapa afim alpha*(.)+delta)
//=======================================================
// ALPHA e uma constante PUBLICA fixa (papel algebrico, como a matriz afim
// do AES); o segredo mora em k_theta, c e no delta aditivo (rk). ALPHA_INV
// e pre-computado uma unica vez em init_cifra().
Elemento ALPHA, ALPHA_INV;

void init_cifra(ll p) {
    // "nothing-up-my-sleeve": digitos de pi e de e reduzidos mod p
    ALPHA = Elemento(mod(mpz_class("314159265358979"), p),
                     mod(mpz_class("271828182845905"), p));
    if (norma(ALPHA, p) == 0) ALPHA = Elemento(1, 0);  // garante invertibilidade
    ALPHA_INV = inverso(ALPHA, p);
}


// S(z) = alpha * conj(z^-1) + delta, com a mistura multiplicativa da chave (k_theta, c)
// aplicada antes da inversao.
Elemento sbox(Elemento z, Elemento k_theta, ll c, Elemento delta, ll p) {
    Elemento t = dilatacao(rotacao(z, k_theta, p), c, p);  // z * k_theta * c
    Elemento u = inverso(t, p);                            // inversao de corpo
    Elemento v = conjugado(u, p);                          // conjugacao (Frobenius)
    return soma(mul(ALPHA, v, p), delta, p);               // alpha*v + delta
}

// Inversa: (y-delta)*alpha^-1 -> conj -> inverso -> desfaz k_theta,c
Elemento sbox_inversa(Elemento y, Elemento k_theta, ll c, Elemento delta, ll p) {
    Elemento v = mul(sub(y, delta, p), ALPHA_INV, p);      // (y - delta)*alpha^-1
    Elemento u = conjugado(v, p);                          // conj desfaz conj
    Elemento t = inverso(u, p);                            // inversao desfaz inversao
    Elemento z = dilatacao_inversa(t, c, p);               // * c^-1
    return rotacao_inversa(z, k_theta, p);                 // * conj(k_theta)
}


Elemento f_mistura(Elemento z, ll p) {
    // multiplica por uma constante fixa pra embaralhar
    Elemento constante(12345, 67890);  // valor arbitrario fixo
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


vector<Elemento> uma_rodada(vector<Elemento> bloco, const SubChave& sk, ll p, int r) {
    // camada de substituicao: S-box afim por elemento
    for (int i = 0; i < 4; i++) {
        bloco[i] = sbox(bloco[i], sk.k_theta, sk.c, sk.rk[i], p);
    }
    // camada de difusao: alterna a direcao conforme a rodada
    if (r % 2 == 0)
        bloco = difusao_frente(bloco, p);
    else
        bloco = difusao_tras(bloco, p);
    return bloco;
}

vector<Elemento> uma_rodada_inversa(vector<Elemento> bloco, const SubChave& sk, ll p, int r) {
    // desfaz a difusao da direcao usada nesta rodada
    if (r % 2 == 0)
        bloco = difusao_frente_inversa(bloco, p);
    else
        bloco = difusao_tras_inversa(bloco, p);
    // desfaz a substituicao
    for (int i = 0; i < 4; i++) {
        bloco[i] = sbox_inversa(bloco[i], sk.k_theta, sk.c, sk.rk[i], p);
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
    // semeia o gerador do key schedule com a chave (DETERMINISTICO)
    gmp_randseed(sorteador_chave, chave.get_mpz_t());

    vector<SubChave> chaves;
    for (int r = 0; r < NUM_RODADAS; r++) {
        // gera um ponto w aleatorio (do gerador semeado pela chave)
        Elemento w;
        mpz_urandomm(w.a.get_mpz_t(), sorteador_chave, p.get_mpz_t());
        mpz_urandomm(w.b.get_mpz_t(), sorteador_chave, p.get_mpz_t());

        // k_theta = ponto de norma 1
        Elemento k_theta = gera_norma1(w, p);

        // c: constante != 0
        ll c;
        mpz_urandomm(c.get_mpz_t(), sorteador_chave, p.get_mpz_t());
        if (c == 0) c = 1;   // garante c != 0

        // rk: delta aditivo da S-box, um elemento por lane
        vector<Elemento> rk(4);
        for (int i = 0; i < 4; i++) {
            mpz_urandomm(rk[i].a.get_mpz_t(), sorteador_chave, p.get_mpz_t());
            mpz_urandomm(rk[i].b.get_mpz_t(), sorteador_chave, p.get_mpz_t());
        }

        chaves.push_back({k_theta, c, rk});
    }
    return chaves;
}


vector<Elemento> cifrar(vector<Elemento> bloco, vector<SubChave> chaves, ll p) {
    for (int r = 0; r < NUM_RODADAS; r++) {
        bloco = uma_rodada(bloco, chaves[r], p, r);
    }
    return bloco;
}


vector<Elemento> decifrar(vector<Elemento> bloco, vector<SubChave> chaves, ll p) {
    for (int r = NUM_RODADAS - 1; r >= 0; r--) {   // ordem reversa
        bloco = uma_rodada_inversa(bloco, chaves[r], p, r);
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
        if (escritos <= cap)                          // normal: alinha a direita
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


// Criterio de avalanche: altera 1 bit do bloco de entrada e mede a fracao
// de bits do bloco cifrado que mudam. Ideal 50%.
double avalanche(ll p, int tentativas) {
    size_t nbits = mpz_sizeinbase(p.get_mpz_t(), 2);
    long total_bits = 8L * (long)nbits;   // 8 componentes de nbits bits

    ll chave;                              // uma chave fixa para todo o teste
    mpz_urandomb(chave.get_mpz_t(), sorteador, (mp_bitcnt_t)nbits);
    auto chaves = key_schedule(chave, p);

    double soma = 0.0;
    for (int t = 0; t < tentativas; t++) {
        vector<Elemento> M(4);             // bloco aleatorio
        for (auto& e : M) {
            mpz_urandomm(e.a.get_mpz_t(), sorteador, p.get_mpz_t());
            mpz_urandomm(e.b.get_mpz_t(), sorteador, p.get_mpz_t());
        }
        auto C = cifrar(M, chaves, p);

        vector<Elemento> M2 = M;           // vira 1 bit aleatorio do plaintext
        int comp = rand() % 8;
        int bit  = rand() % (int)nbits;
        mpz_class* alvo = (comp % 2 == 0) ? &M2[comp/2].a : &M2[comp/2].b;
        mpz_combit(alvo->get_mpz_t(), bit);
        auto C2 = cifrar(M2, chaves, p);

        long dif = 0; mpz_class x;         // conta bits diferentes
        for (int i = 0; i < 4; i++) {
            x = C[i].a ^ C2[i].a; dif += mpz_popcount(x.get_mpz_t());
            x = C[i].b ^ C2[i].b; dif += mpz_popcount(x.get_mpz_t());
        }
        soma += (double)dif / total_bits;
    }
    return 100.0 * soma / tentativas;
}


// Teste de homogeneidade: decifra com chave errada e verifica se o resultado
// e um multiplo escalar da mensagem (D2[i] = lambda*M[i] com o MESMO lambda).
// Retorna quantas vezes a propriedade ocorreu (esperado 0, apos o passo afim).
int homogeneidade(ll p, int tentativas) {
    size_t nbits = mpz_sizeinbase(p.get_mpz_t(), 2);
    int ocorrencias = 0;
    for (int t = 0; t < tentativas; t++) {
        ll kc, kw;                         // duas chaves distintas
        mpz_urandomb(kc.get_mpz_t(), sorteador, (mp_bitcnt_t)nbits);
        mpz_urandomb(kw.get_mpz_t(), sorteador, (mp_bitcnt_t)nbits);
        auto Kc = key_schedule(kc, p);
        auto Kw = key_schedule(kw, p);

        vector<Elemento> M(4);             // bloco aleatorio
        for (auto& e : M) {
            mpz_urandomm(e.a.get_mpz_t(), sorteador, p.get_mpz_t());
            mpz_urandomm(e.b.get_mpz_t(), sorteador, p.get_mpz_t());
        }

        auto C  = cifrar(M, Kc, p);
        auto D2 = decifrar(C, Kw, p);

        // lambda_i = D2[i] * M[i]^-1 ; se todos iguais, e multiplo escalar
        Elemento lam0 = mul(D2[0], inverso(M[0], p), p);
        bool escalar = true;
        for (int i = 1; i < 4; i++) {
            Elemento lami = mul(D2[i], inverso(M[i], p), p);
            if (!(lami.a == lam0.a && lami.b == lam0.b)) { escalar = false; break; }
        }
        if (escalar) ocorrencias++;
    }
    return ocorrencias;
}



// =============================================
//            Funções dos testes
// =============================================


ll configurar_primo() {
    int bits;
    cout << "Bits do primo p (ex: 256): ";
    cin >> bits;
    ll p = gera_primo_3mod4(bits);
    cout << "\np = " << p << "\n(p mod 4 = " << mod(p, 4) << ")\n\n";
    return p;
}


ll gerar_chave_aleatoria(int bits) {
    ll chave;
    mpz_urandomb(chave.get_mpz_t(), sorteador, (mp_bitcnt_t)bits);
    return chave;
}

// parametrizar dps
void teste_texto_conhecido(ll p) {
    size_t nbits = mpz_sizeinbase(p.get_mpz_t(), 2);
    ll chave = gerar_chave_aleatoria(nbits);
    auto chaves = key_schedule(chave, p);
    cout << "Chave K = " << chave << "\n";
    cout << "Subchaves geradas: " << chaves.size() << "\n\n";

    string texto = "Cifra SPN sobre o toro T2";
    cout << "--- Mensagem ---\nTexto: \"" << texto << "\"\n";
    auto M = texto_para_bloco(texto, p);
    imprime_bloco("Bloco original", M);

    // Cifração
    auto C = cifrar(M, chaves, p);
    cout << "\n--- Cifracao ---\n";
    imprime_bloco("Bloco cifrado", C);

    // Decifração correta
    auto D = decifrar(C, chaves, p);
    cout << "\n--- Decifracao (chave correta) ---\n";
    imprime_bloco("Bloco recuperado", D);
    cout << "Texto recuperado: \"" << bloco_para_texto(D, texto.size(), p) << "\"\n";
    cout << "Recuperou? " << (blocos_iguais(M, D) ? "SIM" : "NAO") << "\n";

    // Decifração com chave errada
    cout << "\n--- Decifracao (chave ERRADA) ---\n";
    ll chave_errada = gerar_chave_aleatoria(nbits);
    cout << "Chave K' = " << chave_errada << "\n";
    auto D2 = decifrar(C, key_schedule(chave_errada, p), p);
    imprime_bloco("Bloco recuperado", D2);
    string lixo = bloco_para_texto(D2, texto.size(), p);
    for (char& ch : lixo)
        if ((unsigned char)ch < 32 || (unsigned char)ch > 126) ch = '.';
    cout << "Texto recuperado: \"" << lixo << "\"\n";
    cout << "Recuperou? " << (blocos_iguais(M, D2) ? "SIM" : "NAO") << "\n";
}


void verificar_blocos_aleatorios(ll p, int N_test) {
    size_t nbits = mpz_sizeinbase(p.get_mpz_t(), 2);
    ll chave = gerar_chave_aleatoria(nbits);
    auto chaves = key_schedule(chave, p);

    int ok = 0;
    for (int t = 0; t < N_test; t++) {
        vector<Elemento> R(4);
        for (auto& e : R) {
            mpz_urandomm(e.a.get_mpz_t(), sorteador, p.get_mpz_t());
            mpz_urandomm(e.b.get_mpz_t(), sorteador, p.get_mpz_t());
        }
        if (blocos_iguais(R, decifrar(cifrar(R, chaves, p), chaves, p)))
            ok++;
    }
    cout << "\n--- Verificacao (blocos aleatorios) ---\n";
    cout << ok << "/" << N_test << " testes corretos\n";
}


void medir_desempenho(ll p) {
    size_t nbits = mpz_sizeinbase(p.get_mpz_t(), 2);
    ll chave = gerar_chave_aleatoria(nbits);
    auto chaves = key_schedule(chave, p);
    vector<Elemento> M(4);   // bloco de amostra
    for (auto& e : M) {
        mpz_urandomm(e.a.get_mpz_t(), sorteador, p.get_mpz_t());
        mpz_urandomm(e.b.get_mpz_t(), sorteador, p.get_mpz_t());
    }
    auto C = cifrar(M, chaves, p);

    int N = nbits <= 128 ? 3000 : (nbits <= 256 ? 1000 : 300);
    cout << "\n--- Desempenho (p = " << nbits << " bits, bloco = "
         << 8 * nbits << " bits) ---\n";

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
}


void teste_avalanche(ll p, int tentativas) {
    cout << "\n--- Avalanche ---\n";
    double percent = avalanche(p, tentativas);
    cout << "Avalanche (1 bit flip, " << tentativas << " tentativas): "
         << percent << "%\n";
}


void teste_homogeneidade(ll p, int tentativas) {
    cout << "\n--- Homogeneidade ---\n";
    int ocorr = homogeneidade(p, tentativas);
    cout << "Ocorrencias de homogeneidade em " << tentativas
         << " tentativas: " << ocorr << "\n";
}


int main() {
    gmp_randinit_default(sorteador);
    gmp_randinit_default(sorteador_chave);
    gmp_randseed_ui(sorteador, (unsigned long)time(nullptr));

    cout << "=== Cifra Simetrica SPN sobre F_{p^2} ===\n\n";

    // Configuração do primo e da cifra
    ll p = configurar_primo();
    init_cifra(p);

    // Teste funcional com texto conhecido
    teste_texto_conhecido(p);

    // Verificação de ida‑e‑volta com blocos aleatórios
    verificar_blocos_aleatorios(p, 1000);

    // Medição de desempenho
    medir_desempenho(p);

    // Teste de avalanche
    teste_avalanche(p, 200);

    // teste de homogeneidade
    teste_homogeneidade(p, 100);

    return 0;
}
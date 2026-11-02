# T9.1b: lista da passada de escuta e jogo

O lead não ouve. Esta lista diz o que abrir, o que esperar e o que anotar. Para cada item, responda "ok" ou descreva o defeito
(onde, o que se ouviu ou viu, o que o Remix faz).

## Pronto antes de você começar

Capturas de 20 s de áudio de oito estágios (Kirby e Mario CPU, sem entrada), todas com som em todos os segundos (RMS entre 1500 e
8700), estão em `build/agent-shots/vs/t91b_stage_<id>/audio.wav`. Isso prova que a música toca; quem diz se é a faixa certa é você.

| Estágio | Id | Pasta |
|---|---|---|
| Dream Land | 6 | `t91b_stage_6` |
| Deku Tree | 41 | `t91b_stage_41` |
| Pokémon Stadium | 45 | `t91b_stage_45` |
| Taltal | 46 | `t91b_stage_46` |
| Glacial | 47 | `t91b_stage_47` |
| WarioWare | 48 | `t91b_stage_48` |
| Battlefield | 49 | `t91b_stage_49` |
| Dr. Mario | 51 | `t91b_stage_51` |

Uma partida do Taltal caiu uma vez (rc 139, sem marca do flake do Metal) numa execução com quatro jogos em paralelo; passou nas
cinco execuções seguintes (uma sozinha e quatro em paralelo). Fica registrada como não reproduzida.

## Como abrir o jogo

Dentro de `build/`, com `BattleShip-Remix.o2r` e `f3d.o2r` presentes:

```
cd build && ./BattleShip-Remix
```

Atalhos para ir direto ao ponto (variáveis de ambiente antes do comando):

| Objetivo | Comando |
|---|---|
| Partida VS num estágio | `SSB64_START_SCENE=22 SSB64_VS_FKINDS=0,8 SSB64_VS_STAGE=<id> ./BattleShip-Remix` (P1 humano) |
| Sound Test | `SSB64_START_SCENE=59 ./BattleShip-Remix` |
| Training sem CSS | `SSB64_TRAIN_FKINDS=<humano>,<cpu> SSB64_VS_STAGE=<id> ./BattleShip-Remix` |
| Training com opções do menu | acrescente `SSB64_REMIX_TRAINING="ShieldBreak=1;Oos=3;Tech=4"` |
| DK Mode | `SSB64_DK_MODE=<modo> ./BattleShip-Remix` (o easter egg do CSS ainda não existe) |

Teclado (padrão do libultraship, igual ao BattleShip original): **WASD é o analógico e as setas são os C-buttons**, e um toque
em C-button faz o lutador pular. Para andar use WASD (X = A, C = B, Z = Z, E = L, R = R, Espaço = Start).

Ids de lutador do Remix são iguais ao fkind do port (Crash 72, Marina 63, Young Link 31, Lanky 76, Marth 58).

## 1. Música e estágios

- [ ] Para cada estágio da tabela: a faixa é a do Remix para aquele estágio e não pula nem engasga nos primeiros 20 s.
- [ ] Peach's Castle II (id 68): abre como o Peach's Castle original (sem Banzai Bill, é código morto no Remix).
- [ ] Música de vitória de um lutador Remix (termine uma partida com Crash, Marina ou Marth) e a tela de resultados.
- [ ] Sound Test: navegar, tocar uma faixa Remix e uma vanilla, voltar à primeira ao dar a volta na lista.

## 2. Vozes e SFX

- [ ] Golpes de lutadores Remix (Crash, Marina, Marth e outros à escolha): o som acompanha o golpe, sem silêncio nem som repetido.
- [ ] Vozes do J (versão japonesa) no estágio e nos lutadores onde o Remix troca o som.
- [ ] Conhecido aberto: sons J da bomba e do bumerangue do Link ainda não existem; anote se incomodar.

## 3. Menus e gráficos

- [ ] Título, CSS (grade de 24 retratos do 12CB não existe; confira o CSS padrão do port) e seleção de estágio.
- [ ] Pausa: legendas "R: Legend" na posição certa e sem sobrepor "1P PAUSE".
- [ ] Resultados do KOTH e do Smashketball: linhas extras na tela de fim.

## 4. Training

- [ ] Menu do port, Remix > Training: cada entrada muda o comportamento (Shield Break Infinite segura a CPU no escudo; Spam Practice
  repete o golpe no intervalo; Percent aplica o valor ao criar a partida; Stage BG troca o fundo).
- [x] OOS contra escudo real: visto em 2026-11-02 por script (Grab saindo do escudo, ver T9.1 do ROADMAP).
- [ ] Ainda não visto em jogo: Tech do menu (spike a CPU no chão com `Tech=4`, ela rola para a esquerda),
  DI da CPU (`DiType=3;DiStrength=1;DiDirection=1;DiFirstHit=1`).

## 5. Conferências do T9.1 que ainda são só minhas

Esteira de Surface, bury e plunge, Game Speed no Training, volta do Lightning ao tamanho do modo, Full Results e Salty Runback:
feitas em 2026-11-01 (detalhes no T9.1 do ROADMAP). O Salty Runback tinha um defeito, já corrigido: ao testar, segure exatamente
A+B+Z+R+START (ou A+B+Z+R+D-Left com a opção alternativa) enquanto a partida termina, sem outros botões.

## Como devolver

Uma lista curta por seção, no formato "seção, item, ok ou defeito". Cada defeito vira uma linha no ROADMAP e uma correção com
teste, antes do T9.3.

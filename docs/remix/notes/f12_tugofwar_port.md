# F12 Tug of War: notas do port (M6, T6.1 a T6.3)

Fonte: `.toolchain/pkgs/smashremix-src/src/TugOfWar.asm` (lido 223-545), nota de pesquisa `docs/remix/notes/f11_mode_tugofwar.md`.

Nota ai-memory: as regras de `_global` foram lidas (`_rules/` integral). O escopo do projeto não foi resolvido: não há `.ai-memory.toml` e o remote `origin` (`JRickey/BattleShip`) não retornou páginas. As regras do projeto não foram carregadas.

## Step P1 and P3: credit logic, last-hit tracking

### Arquivos

- Novo: `decomp/src/gm/gmremixtugofwar.h` e `decomp/src/gm/gmremixtugofwar.c`. Estado (`last_hit_by_port[4]`, `team_by_port[4]`, `is_1v1`), `gmRemixTugOfWarSetup`, recorders de dano e de quebra de escudo, `gmRemixTugOfWarGetLastHit`, a escolha pura `gmRemixTugOfWarPickCreditPort` e `gmRemixTugOfWarShouldRevive`. Os 1v1 ports ficam só no `setup` (cruzamento dos últimos acertos), não guardados em estado.
- `decomp/src/sc/sccommon/scvsbattle.c`: include e `gmRemixTugOfWarSetup();` logo após `gmRemixStaminaResetDefeated();` (em `scVSBattleStartBattle`, antes de `gmRemixModeLogMatch`).
- `decomp/src/ft/ftcommon/ftcommondamage.c`: include (bloco `#if defined(PORT) && defined(REGION_REMIX)`) e chamada `gmRemixTugOfWarRecordDamage(this_fp, attacker_fp)` dentro do bloco `if (attacker_gobj != NULL)`, antes de `attack_count++`.
- `decomp/src/ft/ftcommon/ftcommonshieldbreakfly.c`: include e chamada `gmRemixTugOfWarRecordShieldBreak(fp)` logo antes de `ftParamUpdate1PGameDamageStats(...)`.
- Não editado: `ftcommondead.c` (P4).

### Hooks (asm -> decomp)

| asm | decomp | observação |
|---|---|---|
| `setup_` (TugOfWar.asm:223-277), chamado por Render.asm:2303 | `scVSBattleStartBattle` -> `gmRemixTugOfWarSetup` | o loop de jogadores do asm (`0x800466FC`) virou a lista `gGCCommonLinks[nGCCommonLinkIDFighter]` (mesma de `ftParamGetPlayerNumGObj`); zera `shield_player` de cada fighter e grava o time por porta |
| `update_last_hit_by_._damage` (TugOfWar.asm:434-464) | `ftCommonDamageInitDamageVars` (`ftcommondamage.c`, tail `attacker_fp->attack_count++`) | RAM 0x80141534 cai dentro dessa função (antes de 0x80141560). Não é unmatched no sentido do `gameplay_m_z.md`: o trecho é o bloco do `attacker_fp` |
| `update_last_hit_by_._shield` (TugOfWar.asm:473-496) | `ftCommonShieldBreakFlyCommonSetStatus` (`ftcommonshieldbreakfly.c`) | o `sw r0, 0x14(sp)` do patch é o argumento de pilha da chamada `ftParamUpdate1PGameDamageStats(fp, shield, ..., 0, 0, 0)`; por isso o hook fica logo antes dessa chamada |

### Desvios do asm (intencional)

- **R1 corrigido (shield break).** O asm sobrescrevia `t1` (time do jogador) com `li t1, last_hit_by_port` e comparava o time do atacante com o endereço. Intenção: pular quando o atacante está no mesmo time do alvo. Implementado: `sGMRemixTugOfWarTeam[attacker_port] == victim->team` -> retorna.
- **R2 corrigido (crédito de mesmo time).** O asm indexava `last_hit_by_port` pelo time do alvo (`t2`). Intenção: pela porta do alvo. Implementado no acesso: `gmRemixTugOfWarGetLastHit(victim_port)`. P4 deve chamar com `fp->player` (0x0D), nunca com `fp->team`.
- **`is_1v1` não entra na escolha de crédito.** O asm usa `is_1v1` só para limpar `last_hit_by_port` depois do crédito (ftcommondead, P4) e para pular os hooks de dano e de escudo. A escolha pura de crédito recebe (time da vítima, time do KOer, porta do KOer, último acerto). Em 1v1 o `setup` já cruza os últimos acertos, então a escolha coincide com o asm sem o parâmetro.
- **Revive.** O asm testa `t9 == 0` em `0x2B` (`bnez t9` pula o revive), mas o comentário diz "increase to 1". Não resolvido porque `0x2B` não confere com nenhuma struct do decomp (R3). A escolha implementada usa o contador de fighter (`FTStruct.stock_count`, -1 = eliminado, fttypes.h): revive quando o contador era -1 antes do crédito. Confirmar no P4 contra o jogo.
- Hooks checam o modo a cada chamada (`gmRemixModeIs`); fora de Tug of War a função sai na primeira linha. Vanilla não muda.

### Escolha de crédito (P1)

`gmRemixTugOfWarPickCreditPort(victim_team, koer_team, koer_port, last_hit_port)`:
- sem KOer válido -> `last_hit_port` (pode ser -1: ninguém).
- KOer de outro time -> `koer_port`.
- KOer do mesmo time -> `last_hit_port`.

Sem `-1` no retorno, o chamador não credita. Casos do asm `_not_hit` (SD sem acerto, TODO no asm) ficam em `-1`: sem crédito, igual ao asm. R9 mantido.

### Verificação

- `scripts/remix/syntax-check.sh` OK em `gmremixtugofwar.c`, `ftcommondamage.c`, `ftcommonshieldbreakfly.c` e `scvsbattle.c`. O aviso de `ftcommondamage.c:685` é anterior e não é do hook.
- Não rodei build nem jogo (regra do brief). Não há harness offline de teste para `gm/` neste passo.

### O que falta

- P4: em `ftcommondead.c` (fora do escopo deste passo), chamar `gmRemixTugOfWarPickCreditPort` com `gmRemixTugOfWarGetLastHit(fp->player)` e aplicar crédito e revive. Limpar `last_hit` de não-1v1 depois do crédito (asm 340-347 e 377-381).
- Riscos abertos: R3 (`0x2B`), R4 (`0x24(sp)` depois de `OS.save_registers`), R5 (`t0` / `a2` no caminho sem KOer), R6 (FFA com todos os times iguais: nunca credita por time, depende do VS; agora o caso de mesmo time credita o último acerto, o que pode dar stock a quem bateu sem ser KOer), R9 (SD sem acerto).

### O que o lead deve ver

- Aceite do P1/P3 é invisível: o efeito aparece no P4. Para checar que o hook roda sem quebrar, partida Tug of War com `SSB64_REMIX_MODE=tugofwar` e dois jogadores, e um log de `remix mode:` (já existente em `gmRemixModeLogMatch`).
- Vanilla sem `REGION_REMIX` ou fora de Tug of War: sem efeito (todas as chamadas estão em `#if defined(PORT) && defined(REGION_REMIX)` ou passam pelo `gmRemixModeIs`).

### Decisão registrada (R1 e R2)

- R1: a checagem de mesmo time no escudo compara o time do atacante (`team_by_port[shield_player]`) com o time da vítima. Mesmo time: não registra.
- R2: o crédito de mesmo time lê `last_hit_by_port` pela porta da vítima. Não pelo time.
- Ambas são correções do asm, sem `#ifdef PORT` extra: vivem só no código Remix.
- `docs/bugs/` não atualizado nesta etapa (P9 do plano de f11, depois de P4 validar no jogo).

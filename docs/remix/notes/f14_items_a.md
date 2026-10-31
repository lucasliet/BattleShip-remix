# F14 notas do escopo items_a (REMIX-TODO de itens do Remix)

Arquivos: `decomp/src/it/itremix/` itremixklaptrap.c, itremixbulletbill.c, itremixrobotbee.c, itremixscuttleminion.c,
itremixturnip.c, itremixcar.c, itremixpitfall.c, itremixcacodemon.c.

## Mapeamento de offsets usado (N64 -> struct_offsets.txt)

- N64 0x398 = `proc_dead` (raw 0x394). Registrado nos itens que o asm registra no blast zone.
- N64 0x33E = `multi` (u16, raw 0x33A). Asm: `sh` de 0xB4 no spawn (pirhana flag), exceto pitfall, que zera.
- N64 0x2D3 bit 0x08 = `is_allow_knockback` (raw 0x2CF:4-4). Bit 0x04 = `is_unused_item_bool` (raw 0x2CF:5-5).
- N64 0x158 bit 0x10 = `attack_coll.can_rehit_shield`. Clang = `can_setoff` (bit 0x80 do byte 0x158).
- N64 0x2CE mask 0xCF = `times_landed` (raw 0x2CA:2-3). N64 0x2CF bit 0x40 = `is_attach_surface` (raw 0x2CB:1-1).
- N64 0x224 + 8*i = `attack_coll.attack_records[i]` (victim_gobj, victim_flags).
- N64 0x1D0 e 0x1D4 ficam dentro de `attack_coll.attack_pos[1]` (0x1C4 + 0xC e + 0x10, pos_prev.x/y).
  Leitores no asm do Remix: Basketball.asm e Boomerang.asm (outros itens). Nenhum leitor no asm dos itens deste escopo.
  A leitura vanilla de attack_pos[1] ocorre somente para `i < attack_count` (itprocess.c:31, itdisplay.c:31).
  O attack_count destes itens nao foi verificado (vem do arquivo de itens do Remix), por isso nao e DEVIATION.

## Gancho de blast zone (proc_dead)

`itProcessProcItemMain` (itprocess.c:195): `if (proc_dead == NULL || proc_dead(item) != FALSE)` destroi o item.
O `j 0x8016F8C0` no fim dos `*_blast_zone_` do Hazards.asm cai depois desse teste, logo o callback que retorna FALSE
nao destroi o item.

| Item | Asm blast zone (Hazards.asm) | Retorno no port |
|------|------------------------------|-----------------|
| KlapTrap | 1798 (jump 0x8016F8C0) | FALSE |
| Car | 6870 (jump 0x8016F8C0) | FALSE |
| Robot Bee | 4968 (`v0 = 1`, destroi) | TRUE |
| Scuttle Minion | 7723 (`v0 = 1`, destroi) | TRUE |
| Banzai Bill | 8376 (`v0 = 1`, destroi) | TRUE |
| Cacodemon | `cacodemon_blast_zone_` (ja registrado antes deste escopo) | ja existente |
| Turnip | zera 0x398 no stage_setting_ (Turnip.asm 168) | `proc_dead = NULL` |

## Estado de cada linha

| Arquivo | TODO original | Estado | Motivo |
|---------|---------------|--------|--------|
| klaptrap | 0x33E / 0x158 / 0x2D3 no spawn | implementado | `multi`, `can_rehit_shield`, `is_allow_knockback` |
| klaptrap | 0x398 blast zone | implementado | `proc_dead = itRemixKlapTrapProcDead` (FALSE) |
| klaptrap | 0x1D0 / 0x1D4 | pendente | alias de attack_pos[1].pos_prev, attack_count nao verificado |
| klaptrap | blast zone hide (GObj 0x38) | implementado | `itRemixKlapTrapSetVisible(FALSE)` (camera_tag) |
| bulletbill | 0x33E / 0x158 / 0x2D3 / 0x398 | implementado | mesmos campos, callback retorna TRUE |
| bulletbill | 0x1D0 / 0x1D4 / 0x35C | pendente | mesma razao do klaptrap |
| robotbee | 0x158 / 0x33E / 0x2D3 / 0x398 | implementado | callback retorna TRUE |
| robotbee | 0x1D0 / 0x1D4 / 0x35C | pendente | mesma razao |
| scuttleminion | 0x158 / 0x33E / 0x2D3 / 0x398 | implementado | callback retorna TRUE |
| scuttleminion | 0x1D0 / 0x1D4 / 0x35C | pendente | mesma razao |
| turnip | 0x398 / 0x33E | implementado | `proc_dead = NULL`, `multi = 0` |
| turnip | 0x2D3 bit 0x04 | corrigido | o TODO estava errado: Turnip.asm nao escreve 0x2D3 nem 0x158 |
| turnip | 0x2CE mask 0x02 (despawn) | pendente | bit cai em times_thrown, significado vanilla nao confirmado |
| car | 0x33E / 0x158 / 0x2D3 | implementado | `multi`, `can_rehit_shield`, `is_allow_knockback` |
| car | 0x354 spawn y | DEVIATION | ver abaixo |
| car | 0x1D0 / 0x1D4 / 0x35C | pendente | mesma razao |
| pitfall | 0x33E zero / 0x2D3 bit 0x04 no stage_setting | implementado | `multi = 0`, `is_unused_item_bool = TRUE` |
| pitfall | 0x2CE mask 0xCF / clang em begin_buried | implementado | `times_landed = 0`, `can_setoff = FALSE` |
| pitfall | clang em throw_initial | implementado | `can_setoff = FALSE` (asm throw_initial_, Pitfall.asm 269-271) |
| pitfall | 0x2CF bit 0x40 em stick_to_surface | implementado | `is_attach_surface = TRUE` (escrita incondicional, delay slot do beq) |
| pitfall | hazard (Surface clipping) | pulado | subsistema fora do port (helper de surface clipping) |
| pitfall | rumble (ftParamMakeRumble) | pulado | campo match_info do jogador nao nomeado |
| cacodemon | 0x158 (sb r0) / 0x33E / 0x2D3 | implementado | 7 flags = FALSE, `motion_attack_id &= 0x1F`, `multi`, `is_allow_knockback` |
| cacodemon | hit records 0x224..0x243 | implementado | `itRemixCacodemonResetHitRecords`: victim_gobj = NULL, group_id = 7, timer_rehit top bit cleared |
| cacodemon | 0x1D4 / 0x35C / 0x11C | pendente | sem campo nomeado (0x11C e knockback effect, fora deste escopo) |

## DEVIATION

- Car `car_blast_zone_` (Hazards.asm 6874-6876): grava o spawn y em item_vars+4 (N64 0x354). Nenhuma rotina do
  car le esse valor de volta (grep no intervalo 6692-6900 mostra so essa escrita). O port guarda a posicao de spawn em
  `sITRemixCarSpawnPos`.

## Observacoes para o lead

- `itremixklaptrap.h` linha 20 ainda declara `void itRemixKlapTrapBlastZone(GObj *item_gobj);`. A funcao foi renomeada
  para `static sb32 itRemixKlapTrapProcDead` (conflito de assinatura se o header fosse editado no mesmo TU). Remover a
  linha do header (fora do escopo deste worker).
- O callback do car ja retorna FALSE (linha 223). O `return item_gobj` citado no TODO inicial e de
  `itRemixCarMakeItem`, nao do callback. Nenhuma correcao necessaria.
- O cacodemon hit record usa o byte 0x0E (group_id 7) conforme o mapa BE do GMHitFlags. O port e LE, mas a escrita e
  por nome de campo, logo independe de endianness. A semantica de group_id no vanilla e marcada como "feels abandoned"
  em gmtypes.h, por isso vale validar em jogo (cacodemon, reset do loop).
- A escrita `is_attach_surface` do pitfall foi colocada no inicio de `itRemixPitfallStickToSurface`; no asm ela ocorre
  depois de algumas escritas de hitbox. Nenhuma delas le esse bit, mas vale conferir ao testar o pitfall enterrado.
- Registro de `proc_dead` (klaptrap, car, cacodemon) confirmado no asm como ponteiro escrito em 0x398. Nao ha build
  nem teste neste worker: validar em jogo no spawn de cada item e no blast zone (klaptrap deve voltar a posicao de
  spawn sem ser destruido; robot bee, scuttle e bill devem ser destruidos).
- ai-memory: o escopo do projeto nao foi resolvido (sem `.ai-memory.toml` na raiz; `project: "BattleShip"` nao existe
  no workspace `default`). So a regra `_global` foi lida.

# Smash Remix: extração da tabela de fighters a partir da ROM

Protótipo: `tools/remix_dump_fighters.py` (stdlib, Python 3, sem `uv`). Gera JSON em `build/remix-dump/` (git-ignored). Nenhum dado derivado da ROM é versionado.

Comando: `python3 tools/remix_dump_fighters.py [--out DIR] [--remix ROM] [--vanilla ROM]`

Saídas: `remix_fighters.json` (97 fighters), `vanilla_fighters.json` (27), `validation.json`.

## 1. Onde a tabela realmente está

Verificado nos bytes das ROMs, não na documentação.

- **Struct table de FTData: ROM `0x92610` / RAM `0x80116E10`, in place.** A tabela não foi realocada. Ela cresce no lugar: as 27 entradas vanilla são seguidas de 70 entradas Remix, terminadas por NULL no índice 97. Dois fatos conferidos: a entrada 27 (RANDOM) aponta para `0x8049F470` (segmento expandido) e a entrada 28 (NONE) aponta para o struct de Mario (`0x80117810`), como indicado em `fighters.md` 1.1.
- **Mapeamento RAM -> ROM confirmado:**
  - dados vanilla `0x80084800`..: `ROM = RAM - 0x80084800` (confirmado pelo struct table e pelos arrays de param).
  - segmento expandido `0x80400000`+: `ROM = RAM - 0x7D800000`.
  - janela de overlay `0x80390000`..`0x80400000` (arrays de menu/submotion): `ROM = RAM - 0x80288A20`. Não estava na documentação. Foi encontrado porque o padrão do primeiro entry de `dFTMarioSubMotionDescs` aparece em ROM `0x108480`, e a mesma relação vale para todos os 27 fighters vanilla com contagem consistente.
- **ACTION_ARRAY_TABLE foi movida no Remix.** `fighters.md` 2.1 cita ROM `0xA6F40`, que é a tabela vanilla. No Remix ela está em RAM `0x804A05D0` (ROM `0x2CA05D0`), com 97 entradas. Os 27 primeiros slots de `0xA6F40` ficaram intactos, mas os demais foram sobrescritos por dados do Remix. O endereço foi decodificado do hook `move_action_array_table_` (ROM `0x62BD0`: `lui t9` + `lw` com offset assinado), não do código-fonte.
- **Layout de `FTData` (0x78 bytes):** confirmado contra `decomp/src/ft/fttypes.h`. Os 9 file ids ficam em `0x00..0x20`. `file_main_size` em `0x24`, `o_attributes` em `0x60`, `mainmotion` em `0x64` (count em `0x6C`), `submotion` em `0x68` com ponteiro para o count em `0x70`, e `file_anim_size` em `0x74`. Os ponteiros `0x28..0x48` são de runtime e não foram comparados.
- **Entradas de motion:** 12 bytes, `(anim_file_id u32, command_offset u32, flags u32)`. Os 0x68/0x70 são o array de submotion no decomp, e o Remix usa esse campo para o array de menu de 0xF entradas (conferido: `ptr70 - ptr68 == count * 0xC` para 97 - 3 fighters).
- **Fighters registrados:** 97 no Remix, 27 no vanilla. Os 97 foram lidos com sucesso.

## 2. Validação contra o decomp

Fonte: `decomp/src/ft/ftdata.c`, ramo `#ifdef PORT`, com símbolos resolvidos por `include/reloc_data.us.h`. A ordem fkind vem do array `dFTManagerDataFiles`.

Resultado para fkinds 0..26 (vanilla ROM x decomp):

- **Campos escalares: 0 divergências** em 27 fighters (9 file ids, `file_main_size`, `o_attributes`, contagem de mainmotion, `file_anim_size`).
- **Arrays de motion: 0 divergências.** 5825 entradas de mainmotion e 386 de menu/submotion comparadas (anim id e flags; offsets são simbólicos no decomp e não foram comparados). Contagens batem em todos.

Os valores `o_attributes` pedidos para MMario, NMario, NFox e NKirby batem com a expectativa:

| Fighter (fkind) | Vanilla | Remix |
|---|---|---|
| MMario (13) | 0x2A8 | 0x2BC |
| NMario (14) | 0x298 | 0x2B0 |
| NFox (15) | 0x2A4 | 0x2BC |
| NKirby (22) | 0x2C0 | 0x2F8 |

Todos os 13 fighters polygon/novos de 13 a 25 mudam `o_attributes` (vanilla -> Remix): NDK (16) 0x298 -> 0x2B0, NSamus (17) 0x3BC -> 0x3D4, NLuigi (18) 0x298 -> 0x2B8, NLink (19) 0x2D8 -> 0x2F0, NYoshi (20) 0x2B8 -> 0x2D0, NCaptain (21) 0x29C -> 0x2B4, NPikachu (23) 0x2A8 -> 0x2C0, NPurin (24) 0x2A0 -> 0x2E8, NNess (25) 0x2F0 -> 0x308. Mario (0), Fox (1), DK (2), Samus (3), Luigi (4), Yoshi (6), Captain (7), Pikachu (9), Purin (10), Ness (11), Boss (12) e GDonkey (26, 0x3C8) não mudam.

### Diferenças Remix x vanilla por fkind (0..26)

| fkind | Fighter | Campos diferentes |
|---|---|---|
| 0 | MARIO | só ponteiro do array de param (`0x64`) mudou; entradas idênticas (array movido) |
| 5 | LINK | `action_array` (ver ressalva 3) |
| 8 | KIRBY | contagem de mainmotion (`0x6C`), ponteiro e entradas de mainmotion, `action_array` (extensão de 155 ações, esperada) |
| 13-25 | MMario..NNess | `o_attributes` (`0x60`) e conteúdo do array de menu |
| 18, 24 | NLuigi, NPurin | também o ponteiro de submotion/menu; o ponteiro do count (`0x70`) continua apontando para o valor vanilla |
| 26 | GDONKEY | ponteiro de submotion/menu e conteúdo do menu |

Idênticos ao vanilla: 1 FOX, 2 DONKEY, 3 SAMUS, 4 LUIGI, 6 YOSHI, 7 CAPTAIN, 9 PIKACHU, 10 PURIN, 11 NESS, 12 BOSS. Dos 27 fighters vanilla, 17 diferem.

## 3. Ressalvas e não resolvido

1. **Contagem de action arrays não é validada.** Não há contagem armazenada na struct e o decomp não tem essas tabelas. O script infere o tamanho pelo gap entre ponteiros distintos (`count_method: inferred_gap_unverified`). Isso falha: o Link (5) tem 17 entradas na vanilla e 100 no Remix a partir do mesmo ponteiro `0x8012A5AC`. Os conteúdos (`entries`) devem ser tratados como não confiáveis até haver uma contagem independente. Os dados brutos de cada entrada de 0x14 bytes são preservados.
2. **Fighters sem action array próprio:** RANDOM (27) e NONE (28) têm ponteiro 0 na tabela Remix. O script registra `count: 0`.
3. **Menu do Remix x vanilla:** para 13..26, o conteúdo do array de menu no Remix difere do vanilla. Na vanilla, a maioria das entradas é zero (`anim 0`, `cmd 0x80000000`). Não está claro se é Remix sobrescrevendo overlay ou população em runtime; não foi investigado.
4. **Inconsistência de ponteiro em NLuigi (18), NPurin (24) e GDonkey (26):** o ponteiro do array de submotion mudou, mas o ponteiro do count continua o vanilla. O campo `consistent_with_count_pointer` do JSON marca isso como `false`.
5. **Não comparados com o decomp:** offsets de motion (simbólicos no decomp), ponteiros de runtime e fighters Remix (97 - 27), que não têm referência no decomp.
6. **Nomes de fighters Remix:** o JSON só tem ids numéricos; nomes estão em `src/Character.asm` do Remix (`define_character`), fora deste protótipo.
7. **Consulta ao ai-memory:** o projeto não foi encontrado no ai-memory (workspace `JRickey` inexistente), então as regras de projeto não puderam ser carregadas. Regras globais foram lidas.

# Paridade do Smash Remix 2.0.1 (gerado por tools/gen_remix_parity.py; não editar à mão)

Uma linha por `.asm` do Remix. Atualize `STATUS` no script quando uma task do `ROADMAP.md` fechar.

| Área | ported | partial | todo | n/a | progresso |
|---|---|---|---|---|---|
| fighters | 74 | 42 | 12 | 0 | 74% |
| items | 5 | 25 | 3 | 0 | 53% |
| stages | 1 | 2 | 4 | 0 | 29% |
| audio | 0 | 3 | 2 | 0 | 30% |
| menus_modes | 0 | 2 | 69 | 0 | 1% |
| ai | 0 | 0 | 32 | 0 | 0% |
| infra | 1 | 0 | 0 | 10 | 100% |

| Arquivo | Área | Status | Port |
|---|---|---|---|
| `AA.asm` | menus_modes | todo |  |
| `AI.asm` | ai | todo | ROADMAP T7.1 |
| `Accessibility.asm` | menus_modes | todo |  |
| `Action.asm` | fighters | ported | ftremixvanillaprocs.c, remix_action_layout.h |
| `AerialAttackFastFall.asm` | menus_modes | todo |  |
| `AirDodge.asm` | menus_modes | todo |  |
| `BGM.asm` | audio | partial | grRemixGetStageBGM |
| `Banjo/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Banjo/Banjo.asm` | fighters | partial | decomp/src/ft/ftremix/banjo |
| `Banjo/BanjoSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/banjo |
| `Blastzone.asm` | stages | todo | ROADMAP T2.7 |
| `Bonus.asm` | menus_modes | todo |  |
| `Boot.asm` | infra | n/a | N64 boot |
| `Bowser/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Bowser/BowserSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/bowser |
| `Bowser/bowser.asm` | fighters | partial | decomp/src/ft/ftremix/bowser |
| `Camera.asm` | stages | todo | ROADMAP T2.7 |
| `CharEnvColor.asm` | fighters | todo | ROADMAP T2.2 |
| `Character.asm` | fighters | ported | port/remix/remix_roster.cpp, ftremix* |
| `CharacterDataScreen.asm` | menus_modes | todo |  |
| `CharacterSelect.asm` | menus_modes | partial | CSS port (port/css_*, mnplayers*) |
| `CharacterSelectDebugMenu.asm` | menus_modes | todo |  |
| `ChargeSmashAttacks.asm` | menus_modes | todo |  |
| `Cheats.asm` | menus_modes | todo |  |
| `Color.asm` | infra | n/a | constants |
| `Combo.asm` | menus_modes | todo |  |
| `ComboMeter.asm` | menus_modes | todo |  |
| `Conker/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Conker/Conker.asm` | fighters | ported | decomp/src/ft/ftremix/conker |
| `Conker/ConkerSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/conker |
| `Costumes.asm` | menus_modes | partial | CSS port (costume tables) |
| `Crash/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Crash/Crash.asm` | fighters | partial | decomp/src/ft/ftremix/crash |
| `Crash/CrashSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/crash |
| `Crash.asm` | menus_modes | todo |  |
| `Credits.asm` | menus_modes | todo |  |
| `CustomGrabAction.asm` | fighters | partial | ftremixpianoshared.c, fighter_registry custom_capture_* |
| `DI.asm` | menus_modes | todo |  |
| `DKMode.asm` | menus_modes | todo |  |
| `DSamus/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `DSamus/DSamus.asm` | fighters | ported | decomp/src/ft/ftremix/dsamus |
| `DSamus/DsamusSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/dsamus |
| `Damage.asm` | fighters | todo | ROADMAP T2.7 |
| `Dedede/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Dedede/Dedede.asm` | fighters | partial | decomp/src/ft/ftremix/dedede |
| `Dedede/DededeSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/dedede |
| `DrLuigi/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `DrLuigi/DrLuigi.asm` | fighters | ported | data-only clone (roster) |
| `DrMario/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `DrMario/DrMario.asm` | fighters | ported | data-only clone (roster) |
| `DragonKing/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `DragonKing/DragonKing.asm` | fighters | partial | decomp/src/ft/ftremix/dragonking |
| `DragonKing/DragonKingSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/dragonking |
| `DragonKingHUD.asm` | fighters | partial | ftremixdragonkingdrainflash.c |
| `ELink/ELink.asm` | fighters | ported | data-only clone (roster) |
| `EPika/EPika.asm` | fighters | ported | data-only clone (roster) |
| `EPuff/EPuff.asm` | fighters | ported | data-only clone (roster) |
| `ESamus/ESamus.asm` | fighters | ported | data-only clone (roster) |
| `Ebi/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Ebi/Ebi.asm` | fighters | ported | decomp/src/ft/ftremix/ebi |
| `Ebi/EbiSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/ebi |
| `FD.asm` | stages | ported | scvsbattle.c (FD music), port stage page |
| `FGM.asm` | audio | partial | Torch RemixAudioFactory |
| `FPS.asm` | infra | n/a | N64 frame counter |
| `Falco/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Falco/Falco.asm` | fighters | ported | decomp/src/ft/ftremix/falco |
| `Falco/Phantasm.asm` | fighters | ported | decomp/src/ft/ftremix/falco |
| `File.asm` | infra | ported | Torch yamls/remix, RelocFactory |
| `FootStool.asm` | menus_modes | todo |  |
| `GBowser/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `GBowser/gbowser.asm` | fighters | ported | data-only clone (roster) |
| `GFX.asm` | fighters | todo | ROADMAP T2.1 |
| `GFXRoutine.asm` | fighters | todo | ROADMAP T2.1 |
| `Gallery.asm` | menus_modes | todo |  |
| `GameEnd.asm` | menus_modes | todo |  |
| `Ganondorf/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Ganondorf/Ganondorf.asm` | fighters | ported | data-only clone (roster) |
| `Global.asm` | infra | n/a | addresses and constants |
| `Goemon/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Goemon/Goemon.asm` | fighters | partial | decomp/src/ft/ftremix/goemon |
| `Goemon/GoemonSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/goemon |
| `HRC.asm` | menus_modes | todo |  |
| `Handicap.asm` | menus_modes | todo |  |
| `Hazards.asm` | stages | partial | grremixhazards_*.c, grremixgating.c |
| `Hitbox.asm` | fighters | todo | ROADMAP T2.7 |
| `Hitstun.asm` | menus_modes | todo |  |
| `InputDelay.asm` | menus_modes | todo |  |
| `InputDisplay.asm` | menus_modes | todo |  |
| `Item.asm` | items | partial | itremixspawn.c, itmanager.c, mnvsitemswitch.c |
| `JDK/JDK.asm` | fighters | ported | data-only clone (roster) |
| `JFalcon/JFalcon.asm` | fighters | ported | data-only clone (roster) |
| `JFox/JFox.asm` | fighters | ported | data-only clone (roster) |
| `JKirby/JKirby.asm` | fighters | ported | decomp/src/ft/ftremix/jkirby |
| `JLink/JLink.asm` | fighters | ported | data-only clone (roster) |
| `JLuigi/JLuigi.asm` | fighters | ported | data-only clone (roster) |
| `JMario/JMario.asm` | fighters | ported | data-only clone (roster) |
| `JNess/JNess.asm` | fighters | ported | data-only clone (roster) |
| `JPika/JPika.asm` | fighters | ported | decomp/src/ft/ftremix/jpika |
| `JPika/JPikaSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/jpika |
| `JPuff/JPuff.asm` | fighters | ported | data-only clone (roster) |
| `JSamus/JSamus.asm` | fighters | ported | data-only clone (roster) |
| `JYoshi/JYoshi.asm` | fighters | ported | data-only clone (roster) |
| `JabLock.asm` | menus_modes | todo |  |
| `Japan.asm` | menus_modes | todo |  |
| `Joypad.asm` | infra | n/a | replaced by the port's controller code |
| `KingOfTheHill.asm` | menus_modes | todo |  |
| `Kirby/Kirby.asm` | fighters | ported | decomp/src/ft/ftremix/kirby |
| `KirbyHats.asm` | fighters | todo | ROADMAP T2.6 |
| `Knockback.asm` | fighters | todo | ROADMAP T2.7 |
| `Lanky/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Lanky/Lanky.asm` | fighters | partial | decomp/src/ft/ftremix/lanky |
| `Lanky/LankySpecial.asm` | fighters | partial | decomp/src/ft/ftremix/lanky |
| `LedgeJump.asm` | menus_modes | todo |  |
| `LedgeTrump.asm` | menus_modes | todo |  |
| `Lucas/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Lucas/Lucas.asm` | fighters | ported | decomp/src/ft/ftremix/lucas |
| `Lucas/LucasSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/lucas |
| `MLuigi/MLuigi.asm` | fighters | ported | data-only clone (roster) |
| `MagnifyingGlass.asm` | items | todo |  |
| `Marina/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Marina/Marina.asm` | fighters | partial | decomp/src/ft/ftremix/marina |
| `Marina/MarinaSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/marina |
| `Marth/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Marth/Marth.asm` | fighters | partial | decomp/src/ft/ftremix/marth |
| `Marth/MarthSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/marth |
| `Menu.asm` | menus_modes | todo |  |
| `MetalMario/MetalMario.asm` | fighters | ported | data-only clone (roster) |
| `Mewtwo/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Mewtwo/Mewtwo.asm` | fighters | partial | decomp/src/ft/ftremix/mewtwo |
| `Mewtwo/MewtwoSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/mewtwo |
| `NBanjo/NBanjo.asm` | fighters | ported | data-only clone (roster) |
| `NBowser/nbowser.asm` | fighters | ported | data-only clone (roster) |
| `NConker/NConker.asm` | fighters | ported | data-only clone (roster) |
| `NCrash/NCrash.asm` | fighters | ported | data-only clone (roster) |
| `NDSamus/NDSamus.asm` | fighters | ported | data-only clone (roster) |
| `NDedede/NDedede.asm` | fighters | ported | data-only clone (roster) |
| `NDrMario/NDrMario.asm` | fighters | ported | data-only clone (roster) |
| `NFalco/NFalco.asm` | fighters | ported | data-only clone (roster) |
| `NGanondorf/NGanondorf.asm` | fighters | ported | data-only clone (roster) |
| `NGoemon/NGoemon.asm` | fighters | ported | data-only clone (roster) |
| `NLucas/NLucas.asm` | fighters | ported | data-only clone (roster) |
| `NMarina/NMarina.asm` | fighters | ported | data-only clone (roster) |
| `NMarth/NMarth.asm` | fighters | ported | data-only clone (roster) |
| `NMewtwo/NMewtwo.asm` | fighters | ported | data-only clone (roster) |
| `NPeach/NPeach.asm` | fighters | ported | data-only clone (roster) |
| `NSheik/NSheik.asm` | fighters | ported | data-only clone (roster) |
| `NSonic/NSonic.asm` | fighters | ported | data-only clone (roster) |
| `NWario/NWario.asm` | fighters | ported | data-only clone (roster) |
| `NWolf/NWolf.asm` | fighters | ported | data-only clone (roster) |
| `NYoungLink/NYoungLink.asm` | fighters | ported | data-only clone (roster) |
| `OnHit.asm` | fighters | todo | ROADMAP T2.7 |
| `PAL.asm` | infra | n/a | N64 TV type |
| `Parry.asm` | menus_modes | todo |  |
| `Pause.asm` | menus_modes | todo |  |
| `Peach/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Peach/Peach.asm` | fighters | ported | decomp/src/ft/ftremix/peach |
| `Peach/PeachSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/peach |
| `Peppy/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Peppy/Peppy.asm` | fighters | ported | decomp/src/ft/ftremix/peppy |
| `Peppy/PeppySpecial.asm` | fighters | ported | decomp/src/ft/ftremix/peppy |
| `PerfectShield.asm` | menus_modes | todo |  |
| `Piano/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Piano/Piano.asm` | fighters | ported | decomp/src/ft/ftremix/piano |
| `Piano/PianoSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/piano |
| `PlayerTag.asm` | menus_modes | todo |  |
| `Poison.asm` | fighters | todo | ROADMAP T2.7 |
| `PokemonAnnouncer.asm` | fighters | todo | ROADMAP T2.5 |
| `Practice.asm` | menus_modes | todo |  |
| `Practice_1P.asm` | menus_modes | todo |  |
| `Projectile.asm` | fighters | ported | decomp/src/wp/*remix* |
| `Rage.asm` | menus_modes | todo |  |
| `Reflect.asm` | fighters | todo | ROADMAP T2.3 |
| `Render.asm` | infra | n/a | replaced by the port's GObj/SObj calls |
| `Roy/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Roy/Roy.asm` | fighters | ported | decomp/src/ft/ftremix/roy |
| `Roy/RoySpecial.asm` | fighters | ported | decomp/src/ft/ftremix/roy |
| `SFXReplace.asm` | audio | todo | ROADMAP T3.2 |
| `SRAM.asm` | menus_modes | todo | ROADMAP T4.1 |
| `SSonic/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `SSonic/SSonic.asm` | fighters | partial | decomp/src/ft/ftremix/ssonic |
| `Sandbag/Sandbag.asm` | fighters | ported | data-only clone (roster) |
| `Settings.asm` | menus_modes | todo |  |
| `Sheik/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Sheik/Sheik.asm` | fighters | partial | decomp/src/ft/ftremix/sheik |
| `Sheik/SheikSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/sheik |
| `Shield.asm` | menus_modes | todo |  |
| `SingleButtonMode.asm` | menus_modes | todo |  |
| `SinglePlayer.asm` | menus_modes | todo |  |
| `SinglePlayerEnemy.asm` | menus_modes | todo |  |
| `SinglePlayerMenus.asm` | menus_modes | todo |  |
| `SinglePlayerModes.asm` | menus_modes | todo |  |
| `Size.asm` | fighters | partial | ftremixsize.c (phase a) |
| `Skeleton.asm` | menus_modes | todo |  |
| `Slippy/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Slippy/Slippy.asm` | fighters | ported | decomp/src/ft/ftremix/slippy |
| `Slippy/SlippyHipTranslation.asm` | fighters | ported | decomp/src/ft/ftremix/slippy |
| `Slippy/SlippySpecial.asm` | fighters | ported | decomp/src/ft/ftremix/slippy |
| `Smashketball.asm` | menus_modes | todo | ROADMAP T6.1 |
| `Sonic/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Sonic/Sonic.asm` | fighters | ported | decomp/src/ft/ftremix/sonic |
| `Sonic/SonicSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/sonic |
| `Spawn.asm` | stages | todo | ROADMAP T2.7 |
| `SpecialZoom.asm` | menus_modes | todo |  |
| `Speed.asm` | menus_modes | todo |  |
| `SpotDodge.asm` | menus_modes | todo |  |
| `Stages.asm` | stages | partial | grremixstage.c, mnmaps.c |
| `StaleMoves.asm` | menus_modes | todo |  |
| `Stamina.asm` | menus_modes | todo |  |
| `Stereo.asm` | audio | todo | ROADMAP T3.2 |
| `StockMode.asm` | menus_modes | todo |  |
| `String.asm` | infra | n/a | assembler helpers |
| `Surface.asm` | stages | todo | ROADMAP T2.7 |
| `SwordTrail.asm` | fighters | todo | ROADMAP T8.1 |
| `TagTeam.asm` | menus_modes | todo |  |
| `Teams.asm` | menus_modes | todo |  |
| `TimedStock.asm` | menus_modes | todo |  |
| `Toggles.asm` | menus_modes | todo |  |
| `Training.asm` | menus_modes | todo |  |
| `Transitions.asm` | menus_modes | todo |  |
| `Tripping.asm` | menus_modes | todo |  |
| `TugOfWar.asm` | menus_modes | todo |  |
| `TwelveCharBattle.asm` | menus_modes | todo |  |
| `VanillaLv10Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `VsDemo.asm` | menus_modes | todo |  |
| `VsRemixMenu.asm` | menus_modes | todo |  |
| `VsStats.asm` | menus_modes | todo |  |
| `WallTeching.asm` | menus_modes | todo |  |
| `Walljump.asm` | menus_modes | todo |  |
| `Wario/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Wario/Wario.asm` | fighters | ported | decomp/src/ft/ftremix/wario |
| `Wario/WarioSpecial.asm` | fighters | ported | decomp/src/ft/ftremix/wario |
| `Widescreen.asm` | infra | n/a | out of scope (4:3) |
| `Wolf/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `Wolf/Wolf.asm` | fighters | partial | decomp/src/ft/ftremix/wolf |
| `Wolf/WolfSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/wolf |
| `YoungLink/AI/Attacks.asm` | ai | todo | ROADMAP T7.1 |
| `YoungLink/YoungLink.asm` | fighters | partial | decomp/src/ft/ftremix/younglink |
| `YoungLink/YoungLinkSpecial.asm` | fighters | partial | decomp/src/ft/ftremix/younglink |
| `ZCancel.asm` | menus_modes | todo |  |
| `captainshared.asm` | fighters | partial | ftremixdkcaptainshared.c, ftremixentry.c |
| `command.asm` | fighters | partial | ftremixcommand.c |
| `dkshared.asm` | fighters | partial | ftremixdkcaptainshared.c, ftremixentry.c |
| `fireball.asm` | fighters | partial | wpmarioremixfireball.c, wpmariofireball.c |
| `foxshared.asm` | fighters | partial | ftremix/falco, wolf, peppy, slippy |
| `items/Basketball.asm` | items | partial | decomp/src/it/itremix/itremixbasketball.c |
| `items/BlueShell.asm` | items | partial | decomp/src/it/itremix/itremixblueshell.c |
| `items/Boomerang.asm` | items | partial | decomp/src/it/itremix/itremixboomerang.c |
| `items/BulletBill.asm` | items | todo |  |
| `items/Cacodemon.asm` | items | partial | decomp/src/it/itremix/itremixcacodemon.c |
| `items/Car.asm` | items | partial | decomp/src/it/itremix/itremixcar.c |
| `items/ClanBomb.asm` | items | ported | decomp/src/it/itremix/itremixclanbomb.c |
| `items/CloakingDevice.asm` | items | partial | decomp/src/it/itremix/itremixcloakingdevice.c |
| `items/Cloud.asm` | items | partial | decomp/src/it/itremix/itremixcloud.c |
| `items/Dango.asm` | items | ported | decomp/src/it/itremix/itremixdango.c |
| `items/DekuNut.asm` | items | partial | decomp/src/it/itremix/itremixdekunut.c |
| `items/Flashbang.asm` | items | partial | decomp/src/it/itremix/itremixflashbang.c |
| `items/FranklinBadge.asm` | items | partial | decomp/src/it/itremix/itremixfranklinbadge.c |
| `items/Gem.asm` | items | ported | decomp/src/it/itremix/itremixgem.c |
| `items/GoldenGun.asm` | items | partial | decomp/src/it/itremix/itremixgoldengun.c |
| `items/Gordo.asm` | items | partial | decomp/src/it/itremix/itremixgordo.c |
| `items/KlapTrap.asm` | items | partial | decomp/src/it/itremix/itremixklaptrap.c |
| `items/Lightning.asm` | items | partial | decomp/src/it/itremix/itremixlightning.c |
| `items/MrSaturn.asm` | items | partial | decomp/src/it/itremix/itremixmrsaturn.c |
| `items/Pitfall.asm` | items | partial | decomp/src/it/itremix/itremixpitfall.c |
| `items/PoisonMushroom.asm` | items | ported | decomp/src/it/itremix/itremixpoisonmushroom.c |
| `items/Pokemon/Psyduck.asm` | items | partial | decomp/src/it/itremix/itremixpsyduck.c |
| `items/Pwing.asm` | items | partial | decomp/src/it/itremix/itremixpwing.c |
| `items/RobotBee.asm` | items | partial | decomp/src/it/itremix/itremixrobotbee.c |
| `items/ScuttleMinion.asm` | items | todo |  |
| `items/Shuriken.asm` | items | ported | decomp/src/it/itremix/itremixshuriken.c |
| `items/Stopwatch.asm` | items | partial | decomp/src/it/itremix/itremixstopwatch.c |
| `items/SuperMushroom.asm` | items | partial | decomp/src/it/itremix/itremixsupermushroom.c |
| `items/Turnip.asm` | items | partial | decomp/src/it/itremix/itremixturnip.c |
| `items/WaddleDee.asm` | items | partial | decomp/src/it/itremix/itremixwaddledee.c |
| `items/WaddleDoo.asm` | items | partial | decomp/src/it/itremix/itremixwaddledoo.c |
| `jigglypuffkirbyshared.asm` | fighters | partial | ftremixpuffkirbyshared.c |
| `linkshared.asm` | fighters | partial | ftremixlinkshared.c |
| `marioshared.asm` | fighters | partial | ftremixentry.c, wpmario |
| `midi.asm` | audio | partial | Torch RemixAudioFactory, audio_bridge.cpp |
| `moveset.asm` | fighters | ported | ftremixcommand.c |
| `nessshared.asm` | fighters | partial | ftremix/lucas |
| `os.asm` | infra | n/a | assembler helpers |
| `pikashared.asm` | fighters | partial | ftremix/jpika |
| `resultsscreen.asm` | menus_modes | todo |  |
| `samusshared.asm` | fighters | partial | ftremixsamusshared.c, ftremixsamusshared2.c |
| `timeouts.asm` | menus_modes | todo |  |
| `yoshishared.asm` | fighters | partial | ftremixyoshishared.c |

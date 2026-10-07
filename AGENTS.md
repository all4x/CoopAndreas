# AGENTS.md — contexto para IAs/devs que forem mexer neste fork

> Fork de [Tornamic/CoopAndreas](https://github.com/Tornamic/CoopAndreas) (GPL-3.0) mantido em `all4x/CoopAndreas`, branch **`coop-lan`**.
> Dono: Alex (fala português; respostas em PT-BR, linguagem simples, ele não é programador C++).
> Objetivo: **GTA San Andreas modo história em coop para 2 jogadores, em 2 PCs, via LAN/Radmin VPN**, com controle via GInput.

---

## 1. Regras de trabalho (combinadas com o dono)

- **Não invente.** Endereço de memória, opcode, hook ou comportamento do SCM só com evidência no código/binário. Se não souber, escreva **UNKNOWN** e investigue.
- **Antes de criar algo, procure se já existe** no CoopAndreas (completo, quebrado ou desconectado). Nada de código duplicado.
- **Commits pequenos**, cada um compilando. Mensagem em inglês, explicando *por quê*.
- Separe **ENGINE SYNC** (player, veículo, armas, interiores) de **MISSION SYNC** (scripts, checkpoints, falha/sucesso).
- **Nunca altere arquivos de mods/jogo com o jogo aberto.** O ModLoader recarregava mods em tempo real e isso crashou o jogo. Hoje o `AutoRefresh` está desligado, mas mantenha a regra.

## 2. Como compila e como chega no PC do dono

- Só compila com **MSVC + xmake (x86)**: há asm inline `__declspec(naked)`. Não dá pra compilar no Linux.
- **CI (`.github/workflows/ci.yml`)**: a cada push em `coop-lan`, o GitHub Actions compila client, server, proxy e launcher e:
  - publica um artifact `CoopAndreas-<sha>`;
  - **força um commit na branch `builds`** com o pacote pronto. Para baixar sem login: `git fetch origin +refs/heads/builds:refs/remotes/origin/builds && git archive origin/builds | tar -x -C <dir>`.
- Conteúdo do pacote: `GTA-SA/` (`CoopAndreasSA.dll`, `eax.dll` = proxy, `LaunchCoopAndreas.exe`, `Jogar-Coop.bat`, `CoopAndreas/main.scm` + `script.img`) e `server/server.exe`.

## 3. Instalação real no PC do dono (Windows)

```
Área de Trabalho\GTA\
├─ JOGAR - Host (PC 1).bat   abre Radmin (se instalado), fecha servidor antigo DESTA pasta,
│                            aplica Servidor\CoopAndreasServer.new.exe se existir, abre servidor e jogo
├─ Fechar tudo.bat           mata CoopAndreasServer/server/gta_sa só desta pasta (por caminho)
├─ Diagnostico.bat           gera diagnostico.txt: quem usa UDP 6767 + processos
├─ LEIA-ME.txt
├─ Servidor\                 CoopAndreasServer.exe (nome único de propósito), server-config.ini
│                            (port=6767, maxplayers=2), coopandreas-server.log
├─ GTA San Andreas COOP\     jogo 1.0 US + Essentials Pack + GInput + CoopAndreas
│   ├─ coopandreas.log       log do client
│   ├─ CLEO.asi.desativado   ← ver seção 5
│   └─ modloader\ (GInputSA, _ESSENTIALS, Cheat Menu by Grinch_)
├─ Para o PC 2\              GTA San Andreas COOP.zip + Atualizacao\ (DLL mais recente + LEIA-ME)
└─ Extras\                   GTA com 2 Player Deluxe (coop local, NÃO usar com CoopAndreas)
```

- Exe: **GTA SA 1.0 US Hoodlum + Largeaddress**, MD5 `2b5066bd4097ac2944ce6a9cf8fe5677`, entry point `0x824570`.
- Jogo sempre aberto com `gta_sa.exe --coop`. O proxy `eax.dll` só carrega o CoopAndreas com essa flag; o `eax.dll` original vira `eax_orig.dll`.
- IP do PC 1 no Radmin: `26.153.235.155`, porta `6767`. O PC 1 conecta em `127.0.0.1`.
- Config do client: `Documentos\GTA San Andreas User Files\coopandreas.ini` (nickname, ip, port).
- **Os dois PCs precisam da MESMA `CoopAndreasSA.dll`.** A versão do protocolo continua `0.3.0-alpha` e não detecta diferença de build.

## 4. O que este fork mudou (sobre upstream `89aafba`)

| Commit | O quê |
|---|---|
| `631e7db`, `dec6def` | CI empacota e publica na branch `builds` |
| `54598b6` | serial key do launcher virou opt-in (`COOP_REQUIRE_SERIAL`). O bot do Discord não gera mais keys |
| `a7026ce` | **crash no boot `0x0074872E` "Privileged instruction"**: o CoopAndreas dava NOP na call de `IsAlreadyRunning` em `0x74872D` e depois o SilentPatch/GInput reescreviam só o rel32. Agora só faz `PutRetn0(0x7468E0)` |
| `71f064e` | server respeita `maxplayers` (padrão 2) e loga eventos LAN |
| `ae3cbf8` | `shared/logger.h` grava em arquivo (`coopandreas.log` / `coopandreas-server.log`); chat espelhado no log; logs de missão, checkpoint, EnEx, veículo, respawn |
| `a34e7e1` | bug upstream: a limpeza de fim de missão no não-host nunca rodava (`PacketHandlers/scripts.cpp`, ON_MISSION_FLAG_SYNC) |
| `4be9e93` | server desliga QuickEdit do console (clicar na janela congelava o server) |
| `486d7cf` | server: heartbeat `[diag]` a cada 10s (laço vivo, estado de cada peer ENet, erros WSA) |
| `a4f7af7` | `CCoopTeleport`: `/tp` e renascer perto do parceiro (fora de missão/interior) |
| `4a625a0` | `CCoopCommands`: comandos no chat (F6). Comandos "dos dois" são executados localmente e reenviados como mensagem de chat; o outro client reconhece e executa |
| `d265309` | `UI/CCoopMenu`: menu clicável no **F7** (ImGui) que chama `CCoopCommands::Run` |
| `9acbc5e`/`e8d6fee` | mouse liberado com menu aberto (`0x6194A0` psSetMousePos → `ret`, NOP em `0x541DD7` call `CPad::UpdateMouse`), restaurado ao fechar; menu com veículos por categoria (helicópteros, aviões/jatos, barcos, motos, off-road) |

## 5. Problemas já resolvidos — não repetir

1. **`CLEO.asi` quebra a rede.** O CLEO 4.4.4 sobrescreve a call `0x53E981` (CGame::Process), que é onde o plugin-sdk pendura o `gameProcessEvent`. O CoopAndreas atende a rede nesse evento. Sintoma: o client diz "connected", mas o server fica com o peer em `state=2` (ACKNOWLEDGING_CONNECT), não há handshake, ficam **0 NPCs** (pedestres e trânsito só ligam depois do handshake, via `CPatch::RevertTemporaryPatches`) e cada um fica sozinho no mundo. **Solução: CLEO desativado** (não há mais scripts CLEO nessa cópia). TODO: detectar isso em código e avisar.
2. **CLEO+ e 2 Player Deluxe são incompatíveis.** O CoopAndreas já avisa do CLEO+ (`CCompatibilityChecker`), e o 2PDX usa `CWorld::Players[1]`, o mesmo slot dos jogadores remotos.
3. **ModLoader `AutoRefresh`** recarregou mods com o jogo aberto e crashou ao entrar num carro. Desligado em `modloader\.data\config.ini`.
4. **Janela do server em modo "Selecionar"** congelava o servidor. Corrigido no `4be9e93`.

## 6. Arquitetura (resumo; análise completa no doc `ANALISE-COOPANDREAS.md` do projeto)

- `server/`: relay ENet (bind `0.0.0.0`). Não simula o jogo. O primeiro jogador conectado vira **host** (`AssignHostToFirstPlayer`).
- O **host roda as missões**. Os outros recebem efeitos via **opcode sync** (`client/src/COpCodeSync.cpp`) dos scripts marcados com `Coop.EnableSyncingThisScript`.
- `scm/main.txt` + `scm/scripts/*.txt` = `main.scm` próprio com opcodes `Coop.*` (`0x1D00–0x1D1C`, `client/src/Commands/`). Compilado com Sanny Builder 4 + `sdk/`.
- Missões com coop real: **Big Smoke (INTRO1), Ryder (INTRO2), Tagging Up Turf (SWEET1), Cleaning the Hood (SWEET1B)**; Intro parcial. Outras 8 do começo exibem "unsupported"; o resto não tem adaptação.
- **GInput**: o CoopAndreas substitui `CPad::GetLookLeft/Right` (`0x53FDD0`/`0x53FE10`, `CDriveBy.cpp`), e o GInput também referencia esses endereços. Risco de conflito no drive-by. UNKNOWN se quebra na prática. O plano é encadear em vez de substituir.
- Debug in-game: cheat `D1212` abre o menu de debug com **Missions** (o host inicia qualquer missão). Digitar o nome de um veículo como cheat (ex. `INFERNUS`) faz spawn.

## 7. Como diagnosticar

- Logs: `GTA San Andreas COOP\coopandreas.log`, `Servidor\coopandreas-server.log`, `modloader\modloader.log` (traz o stack dos crashes), `CoopAndreas_crashes\` (se existir).
- Conexão ok no server: `Peer connected` → `Player N 'nome' joined` → `Host ... is now player 0`. No client: `Authenticated, playerid N`.
- `[diag] ... state=2` repetindo = o client não está atendendo a rede (ver seção 5.1).

## 8. Próximos passos combinados

1. Testar **Tagging Up Turf** em coop e depois **Ryder** (veículo, interior, falha).
2. Compatibilidade GInput (`GetLookLeft/Right` encadeado).
3. Avisar em código quando outro mod sobrescrever `0x53E981`.
4. Adaptar mais missões de Los Santos, uma por vez, seguindo o padrão de `SWEET1.txt`.
5. Pendências do próprio CoopAndreas que o dono quer: dinheiro e nível de procurado sincronizados, coletáveis, gamepad no assento de passageiro.

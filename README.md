# Connect Four

A modular C implementation of Connect Four with pluggable bots. The gameplay loop lives in `main.c` and delegates bot turns to a central manager that picks from individual strategy files.

## Project layout

```
.
├── board.c / board.h          # Board representation helpers
├── bot_manager.c / .h         # Dispatches bot moves based on difficulty
├── bots/                      # Individual bot implementations
│   ├── bot_easy.c/.h
│   ├── bot_medium.c/.h
│   ├── bot_hard.c/.h
│   ├── bot_utils.c/.h         # Shared helpers (random move, simulation)
│   └── bot.h                  # Bot API contracts
├── main.c                     # Game loop and CLI
└── Makefile                   # Build automation
```

## Building

Requires a C11-compatible compiler (tested with `gcc`). From the project root:

```bash
make
```

This produces the `connect4` executable. Clean artifacts with `make clean`.

## Playing

Run the game and follow the prompts:

```bash
./connect4
```

- Choose human vs. human or human vs. bot.
- Provide player tokens (single characters); when playing vs. the bot, the bot token defaults to `B`.
- If playing vs. the bot, pick a difficulty:
  - **Easy** – random legal moves.
  - **Medium** – immediate win/block search plus positional heuristics.
  - **Hard** – adds board evaluation for deeper tactical play.

On each turn, enter the column number (1–7). The board redraws after every move. The game ends on the first connect-four or when the board fills.

## Extending bots

To add a new strategy:

1. Create `bots/bot_<name>.c` + `.h` implementing `BotStrategy`.
2. Add the strategy to the static array inside `bot_manager_init`.
3. Rebuild with `make`.

Each strategy receives a `BotRequest` containing the current board state, player tokens, and the opponent’s last move, enabling sophisticated decisions.

# Connect 4 (Hard-Impenetrable-Agent)

This project is a console-based Connect 4 game in C with:

- A **hard bot** that uses a Bitboard + Negamax + Alpha-Beta search.
- Optional **networking experiments** (separate from the main game loop).
- A lightweight **Alpine-based Docker image** to build and run the game.

## Agent
Explained in the PDF attached.

## Significant update to the Agent: DP

We introduced a top-down dp layer that caches negamax evaluations in a 12-ply transposition table keyed on both player bitboards. Each position stores the score along with whether it represents an exact value, a lower bound, or an upper bound, so repeated subtrees can be pruned or skipped entirely. Because many move sequences in Connect Four transpose into the same board, this cache dramatically cuts the number of nodes searched and keeps depth-12 searches within reasonable response times. After each move the table is exported to `dp_export.csv`, making it easy to confirm the cached states and verify that the DP layer was exercised during play.


## Networking (experimental)

The networking layer is a simple text-based TCP protocol on port 8080 where a stateful server (`networking/server.c`) keeps all users and games in memory and accepts short commands like `login`, `join`, `move`, `board`, and `logout` over one-off connections, while an interactive client (`networking/client.c`) repeatedly opens a TCP connection, sends a single command string (for example, `login <username> <password>` or `move <cert> <col>`), receives a single textual response (such as `OK win`, `OK next`, `OK board`, or `ERR`), and then closes the socket; the shared game logic in `networking/network_game_logic.c` manages a 6×7 board array, placing coins and checking for 4-in-a-row on the server side for each `move` command so both players see a consistent board when they request `board <cert>`.

Problems that might occur with the server:

* Race condition if too many players connected at once.


## Running with Docker (Alpine)

A `Dockerfile` in the project root builds and runs the game inside an Alpine Linux container.

From the project root:

```bash
# Build the image
docker build -t connect4 .

# Run the game in an interactive container
docker run -it --rm connect4
```

Inside the container, the game starts immediately and you can interact with it via standard input
and output (the same prompts you see when running `./app` locally).


## Multithreading

Summary:

Good for bot x human matches on one level and for bot-bot matches on another.

Explaination:

How we believe multithreading would help here is by simultaneously doing expensive bot compution (that takes approximately 0.13 seconds on an M3 Macbook Air) while taking user input, improving `perceived responsiveness.` This is important because this way, since the user is taking time to enter their move, it doesn't seem like the computation took place.

Multithreading can further help by parallelizing search (root move splitting, speculative parallel alpha-beta) or running multiple matches concurrently.


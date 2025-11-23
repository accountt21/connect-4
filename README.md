# Connect 4 (Hard-Impenetrable-Agent)

This project is a console-based Connect 4 game in C with:

- A **hard bot** that uses a Bitboard + Negamax + Alpha-Beta search.
- Optional **networking experiments** (separate from the main game loop).
- A lightweight **Alpine-based Docker image** to build and run the game.

## Agent
Explained in the PDF attached in the appendix.

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


## Appendix: 

`Algorithmic Analylsis.pdf`.

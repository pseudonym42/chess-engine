all:
	gcc vice.c init.c bitboards.c hashkeys.c data.c board.c attack.c -o chess_engine

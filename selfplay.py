'''
A tournament script for othello engines compiled with protocol.cpp.
Functionality similar to cutechess-cli.
'''

import random
import subprocess
import sys
import threading
import time

BLACK = 0
WHITE = 1
TOTAL_TIME = 4000
ENGINE_DIR = "C:\\Users\\Jeffrey\\Documents\\GitHub\\othello_engine\\"
BOOK = "perft6.txt"

count = 0
score = [0, 0, 0]
mutex = threading.Lock()


# Sends a formatted command to the engine
def put(engine, command):
    engine.stdin.write(command + '\n')

# Wait for ready signal
def wait(engine):
    engine.stdin.write("isready\n")
    while engine.stdout.readline().strip() != "ready":
        pass

# Formats a move for printing
def format_move(x, y, time_left):
    return str(x) + " " + str(y) + " " + str(time_left)

# Reads and parses a move from an engine
def get_move(engine):
    x = 0
    y = 0
    ct = [0, 0]
    while True:
        text = engine.stdout.readline().strip().split()
        if len(text) != 4:
            continue
        else:
            x = int(text[0])
            y = int(text[1])
            ct[BLACK] = int(text[2])
            ct[WHITE] = int(text[3])
            break

    return x, y, ct[BLACK], ct[WHITE]

# Plays a single match and returns the result and movelist
def play_match(engine_names, opening):
    # Start engines
    engines = []
    color_input = "black"
    bits = opening.split(' ')
    for name in engine_names:
        e = subprocess.Popen(
            [ENGINE_DIR + name, color_input, bits[0], bits[1]],
            # [ENGINE_DIR + name, color_input],
            bufsize = 0,
            universal_newlines = True,
            stdin = subprocess.PIPE,
            stdout = subprocess.PIPE,
            stderr = subprocess.DEVNULL
        )
        engines.append(e)
        color_input = "white"
        sys.stdout.flush()

    # Wait for both programs to initialize
    for e in engines:
        wait(e)

    # Play engine match
    movelist = "" # in 0-63 format, separated by spaces
    movenumber = 0 # black's turn if even, white's turn if odd
    last_x = -1
    last_y = -1
    piece_cts = [0, 0]
    passed_last = False
    engine_times = [TOTAL_TIME, TOTAL_TIME]

    while True:
        color = movenumber % 2
        e = engines[color]

        start_time = time.time() * 1000

        # Send last move and receive new move
        put(e, format_move(last_x, last_y, engine_times[color]))
        last_x, last_y, piece_cts[BLACK], piece_cts[WHITE] = get_move(e)
        # sys.stdout.write(str(last_x + 8*last_y) + " " + str(sum(piece_cts)) + "\n")

        end_time = time.time() * 1000
        time_used = int(end_time - start_time)
        engine_times[color] -= time_used
        # TODO terminate if an engine goes over time

        movelist += str(last_x + 8 * last_y) + " "
        movenumber += 1

        # Determine game end
        if last_x == -1 and last_y == -1:
            if passed_last:
                break
            passed_last = True
        else:
            passed_last = False

    global mutex
    mutex.acquire()
    # TODO record games somehow
    # sys.stdout.write(movelist + "\n")
    sys.stdout.write(str(piece_cts[BLACK]) + "-" + str(piece_cts[WHITE]) + "\n")
    sys.stdout.flush()
    mutex.release()

    # Close both programs
    for e in engines:
        # Current behavior is to end programs when anything other than a move
        # is input
        put(e, "quit")

    return piece_cts[0], piece_cts[1]

def play_matches(engine_names, book, start, end):
    global count
    global score
    global mutex

    opns = end - start + 1
    perm = list(range(opns))
    random.shuffle(perm)
    for i in range(0, opns):
        mutex.acquire()
        sys.stderr.write("Starting game " + str(count+1) + "\n")
        mutex.release()
        b, w = play_match(engine_names, book[start+perm[i]])
        mutex.acquire()
        count += 1
        # Determine who won based on piece counts
        if b > w:
            score[0] += 1
        elif b < w:
            score[1] += 1
        else:
            score[2] += 1
        sys.stderr.write("Game " + str(count) + " finished: " + str(score[0]) \
            + "-" + str(score[1]) + "-" + str(score[2]) + "\n")
        mutex.release()
        engine_names.reverse()

        # Replay with opposite colors?
        mutex.acquire()
        sys.stderr.write("Starting game " + str(count+1) + "\n")
        mutex.release()
        w, b = play_match(engine_names, book[start+perm[i]])
        mutex.acquire()
        count += 1
        # Determine who won based on piece counts
        if b > w:
            score[0] += 1
        elif b < w:
            score[1] += 1
        else:
            score[2] += 1
        sys.stderr.write("Game " + str(count) + " finished: " + str(score[0]) \
            + "-" + str(score[1]) + "-" + str(score[2]) + "\n")
        mutex.release()
        engine_names.reverse()

# Read command line arguments
# Argument 1: engine 1 name
# Argument 2: engine 2 name
engine_names = [sys.argv[1], sys.argv[2]]
if len(sys.argv) == 3:
    pos_book = []
    with open(BOOK) as f:
        for line in f:
            pos_book.append(line.strip())

    num_threads = 1
    div = len(pos_book) // num_threads
    thread_list = []
    # TODO set up a queue for games so they go roughly in order
    for i in range(num_threads):
        end_val = (i+1)*div if (i+1) < num_threads else len(pos_book)
        thread_list.append(threading.Thread(target=play_matches, args=(engine_names, pos_book, i*div, end_val)))
    for i in range(num_threads):
        thread_list[i].start()
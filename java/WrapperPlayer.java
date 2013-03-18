import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;

/**
 * A wrapper class for the CS 2 Othello tournament. This should 
 * enable students to use a non-Java Othello player by handing the moves
 * back and forth using stdin/stdout.
 */
public class WrapperPlayer implements OthelloPlayer {
    private final static int MAX_MEMORY_KB = 786432;
    private long startTime;
    private Process p;
    private BufferedReader br;
    private BufferedReader stderr;    
    private BufferedWriter bw;
    private String name;
    private String test;
    public WrapperPlayer(String programName) {
        this.name = programName;
    }
    
    /**
     * Makes the next move in the game.
     * 
     * @param opponentsMove the last move made by the other player. If
     * it is null, then that player passed, of this player is the first
     * player to make a move.
     * 
     * @param millisLeft the number of milliseconds that this player
     * has remaining in the entire game, before going overtime and
     * being disqualified.
     * 
     * @return Move this player's move. May be null only if there are
     * no legal moves to make.
     */
    public Move doMove(Move opponentsMove, long millisLeft) {    
        // Print out everything from stderr.
        try {
            while (stderr.ready()) {
                System.out.println(stderr.readLine());
            }
        } catch (Exception e) {   
            e.printStackTrace();
        }

        startTime = System.currentTimeMillis();
        String line;
        try {
            if (opponentsMove == null) {
                bw.write("-1 -1 " + millisLeft + "\n");
            } else {
                bw.write(opponentsMove.getX() + " " + opponentsMove.getY()
                    + " " + millisLeft + "\n");
            }
            bw.flush();
            
            while (!br.ready()) {
                // Print out everything from stderr again.
                try {
                    while (stderr.ready()) {
                        System.out.println(stderr.readLine());
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                try {
                    int exitvalue = p.exitValue();
                    // If we got an exit value (versus exception), it means
                    // the program has ended; don't wait for input.
                    break;
                } catch (IllegalThreadStateException e) {
                    // Program still running, don't do anything.
                }                                
                Thread.yield();
                Thread.sleep(100);
            }
            line = br.readLine();
            if (line == null || line.equals("-1 -1")) {
                return null;
            } else {
                String[] parts = line.split(" ");
                return new Move(Integer.parseInt(parts[0]),
                    Integer.parseInt(parts[1]));
            }            
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /* (non-Javadoc)
     * @see cs2ai.OthelloPlayer#init(cs2ai.OthelloSide)
     */
    public void init(OthelloSide side) {
        try {
            String cmd = "ulimit -m " + MAX_MEMORY_KB + " -v " + MAX_MEMORY_KB + ";";
            cmd += "./" + name + " " + side;
            ProcessBuilder pb = new ProcessBuilder("bash", "-c", cmd);
            p = pb.start();            
            
            br = new BufferedReader(new InputStreamReader(p.getInputStream()));
            stderr = new BufferedReader(new InputStreamReader(p.getErrorStream()));                    
            bw = new BufferedWriter(new OutputStreamWriter(p.getOutputStream()));
              
            // Wait for message that program is done initialization.                       
            br.readLine();
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }
}

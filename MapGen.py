import random

rows = 10
cols = 10
graph = [['W' for k in range(cols)] for j in range(rows)]
  
def printgraph(g):
    print("\n".join(map(lambda x: "".join(x), g)) + "\n")

def hasthree(g, i, j):
    count = 0;
    if (g[i+1][j+1] == 'W'):
        count = count + 1
    if (g[i+1][j-1] == 'W'):
        count = count + 1
    if (g[i-1][j+1] == 'W'):
        count = count + 1
    if (g[i-1][j-1] == 'W'):
        count = count + 1
    return count >= 3 and g[i][j] == 'W'

def dfsMaze(g, i, j, fr, fc):
    if(i != fr and j != fc):
        attempts = 0
        g[i][j] = '0'
        printgraph(g)
        while(attempts < 5000):
            attempts = attempts + 1
            movei = random.randint(0,1)
            positive = random.randint(-1,1)
            di = int(movei) * positive
            dj = int(not movei) * positive
            notedge = i+di>0 and j+dj>0 and i+di<rows and j+dj<cols
            if (notedge and hasthree(g, i+di, j+dj)):
                dfsMaze(g, i+di, j+dj, fr, fc)
                return
    elif (i == fr and j == fc):
        g[i][j] = 'F'

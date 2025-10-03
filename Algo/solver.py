"""
The main function is solver(grid, word).

Algorithm :
    for each (i,j) case in the grid :
        for each of the 8 directions, search the word :
            - if the word was found :
                return coordinates (startX,startY),(endX,endY)
    return None
"""

def solver(grid : list[list], word : str):
    """
    Parameters : Grid (A matrix of letters) and non-empty word (A string)
    Matrix dimensions are strictly positives, according to the algrebric definition.
    The function finds a word in a grid and returns a tuple of two couples of integers.
    Returns (START,END) with START = (x1,y1) and END = (x2,y2) if the word was found and None in other cases.
    """
    height = len(grid)   # rows count
    width = len(grid[0]) # columns count
    word_len = len(word) # word lenght

    directions = [
        (0, 1),   # → E  (East)
        (0, -1),  # ← W  (West)
        (1, 0),   # ↓ S  (South)
        (-1, 0),  # ↑ N  (North)
        (1, 1),   # ↘ SE (South-East)
        (-1, -1), # ↖ NW (North-West)
        (1, -1),  # ↙ SW (South-West)
        (-1, 1)   # ↗ NE (North-East)
    ]

    for i in range(height):
        for j in range(width):
            if grid[i][j] == word[0]: # to avoid searching if we already know it's not here
                for (abscisse, ordonnee) in directions:
                    # (x,y) are moving from the first letter in (i,j) in the direction (abscisse, ordonnee)
                    x = i 
                    y = j

                    k = 0
                    # while the letters in the current directions works with the word's letters.
                    while (k < word_len) and (0 <= x < height and 0 <= y < width) and (grid[x][y] == word[k]):
                        x += abscisse
                        y += ordonnee
                        k += 1 # current letter index in the word
                    if k == word_len:
                        # we found the word from (i,j) to (i2, j2).

                        # Note that the variables (x,y) have gone one box too far
                        # That's why we desincrement them in the opposite direction
                        i2 = x - abscisse
                        j2 = y - ordonnee
                        return ((j, i), (j2, i2)) # Python's indexes (i,j) are (j,i) humans coordinates.
    



# ------------------------------------ TESTS ------------------------------------
if __name__ == "__main__":
    matrix = [
        ['H','O','R','I','Z','O','N','T','A','L'],
        ['D','X','R','A','H','C','L','B','G','A'],
        ['D','I','K','C','I','L','E','O','K','C'],
        ['I','G','A','J','H','Y','L','Y','H','I'],
        ['H','G','F','G','O','D','T','I','O','T'],
        ['G','D','L','R','O','W','K','B','F','R'],
        ['P','L','N','R','D','N','E','R','G','E'],
        ['J','H','A','I','D','U','A','J','G','V'],
        ['U','K','G','F','F','O','L','L','E','H']
    ]

    def unit_test():
        print("-"*30)
        result = solver(matrix,"HORIZONTAL")
        print(f"{result}", end=" ")
        print("🟢" if result==((0,0),(9,0)) else "🔴")
        
        result = solver(matrix,"VERTICAL")  
        print(f"{result}", end=" ")
        print("🟢" if result==((9,7),(9,0)) else "🔴")
        
        result = solver(matrix,"DIAGONAL")  
        print(f"{result}", end=" ")
        print("🟢" if result==((0,1),(7,8)) else "🔴")
        
        result = solver(matrix,"FIND")      
        print(f"{result}", end=" ")
        print("🟢" if result==((4,8),(1,5)) else "🔴")
        
        result = solver(matrix,"HELLO")     
        print(f"{result}", end=" ")
        print("🟢" if result==((9,8),(5,8)) else "🔴")
        
        result = solver(matrix,"WORLD")     
        print(f"{result}", end=" ")
        print("🟢" if result==((5,5),(1,5)) else "🔴")
        
        result = solver(matrix,"GOLDORAK")  
        print(f"{result}", end=" ")
        print("🟢" if result==((8,1),(1,8)) else "🔴")
        
        result = solver(matrix,"EPITA")     
        print(f"{result}", end=" ")
        print("🟢" if result==(None)        else "🔴")

    unit_test()
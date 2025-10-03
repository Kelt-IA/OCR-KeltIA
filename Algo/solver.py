"""
The main function is solver(grid, word).

Algorithm :
    for each (i,j) in the grid :
        - search for the word in all (8) directions.
        - if the word was found :
            return coordinates
    return "no results".
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
    
    for i in range(height):
        for j in range(width):
            result = search(grid, word, i, j, width, height, len(word))
            if (result != None):
                return result

    return None


def search_simple(grid, word, x1, y1, width, height, wordLength):
    """
    Searchs a word from Start = (x1,y1) in the four standard directions.
    Returns None if the word was not found.
    """
    # Horizontal (West -> East)
    if y1 + wordLength <= width:
        match = True
        i = 0
        while i < wordLength and match:
            if grid[x1][y1 + i] != word[i]:
                match = False
            i += 1
        if match:
            return ((y1, x1), (y1 + wordLength - 1, x1))
    
    # Vertical (North -> South)
    if x1 + wordLength <= height:
        match = True
        i = 0
        while i < wordLength and match:
            if grid[x1 + i][y1] != word[i]:
                match = False
            i += 1
        if match:
            return ((y1, x1), (y1, x1 + wordLength - 1))
        
    # Diagonal Principale (NW -> SE)
    if x1 + wordLength <= height and y1 + wordLength <= width:
        match = True
        i = 0
        while i < wordLength and match:
            if grid[x1 + i][y1 + i] != word[i]:
                match = False
            i += 1
        if match:
            return ((y1, x1), (y1 + wordLength - 1, x1 + wordLength - 1))
        
    # Diagonale Secondaire (NE -> SW)
    if x1 + wordLength <= height and y1 - wordLength + 1 >= 0:
        match = True
        i = 0
        while i < wordLength and match:
            if grid[x1 + i][y1 - i] != word[i]:
                match = False
            i += 1
        if match:
            return ((y1, x1), (y1 - wordLength + 1, x1 + wordLength - 1))


 
    return None

def search(grid, word, x1, y1, width, height, wordLength):
    """
    Searchs a word from Start = (x1,y1)
    Calls the function search_simple twice to search in 8 directions.
    Returns None if the word was not found.
    """
    result = search_simple(grid, word, x1, y1, width, height, wordLength)
    if result != None:
        return result
    result = search_simple(grid, mirror(word), x1, y1, width, height, wordLength)
    if result != None:
        return (result[1], result[0])

    
    

def mirror(s: str) -> str:
    return s[::-1] # I know this is a Pythonnerie. We will have to do it in C.
















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
def partition(MatrixPicture, n, seuil):
    """
    MatrixPicture : Contains only 0 (white) or 1 (black) values.
    n : Number of rows and columns there should be in the grid.
    seuil : Real number in [0, 1]. Determines the level the square is considered white or black.
    """

    # Note: seuil = 0.0 => set to 1 if there is at least one black pixel.
    # Note: seuil = 0.5 => set to 1 if the majority of pixels are black.
    # Note: seuil = 1.0 => never set to 1.

    num_rows = len(MatrixPicture)
    num_cols = len(MatrixPicture[0])


    if (n > num_rows or n > num_cols):
        raise ValueError(f"{n=} est trop grand.")

    # The term "square" refers to the portion of the MatrixPicture corresponding to a single grid cell.

    # grid : n*n matrix initially filled with -1.
    grid = [[-1 for _ in range(n)] for _ in range(n)]

    # block : the width or height of the rows or columns corresponding to a single grid cell.
    block_h = num_rows // n
    reste_h = num_rows % n
    block_w = num_cols // n
    reste_w = num_cols % n

    x = 0
    for a in range(n):
        h = block_h + (a < reste_h)
        y = 0
        for b in range(n):
            w = block_w + (b < reste_w)

            a2 = min(x + h, num_rows)
            b2 = min(y + w, num_cols)

            # Checks if current square contains more black pixels than white according to the seuil.
            total = 0
            nb = 0
            for i in range(x, a2):
                for j in range(y, b2):
                    total += MatrixPicture[i][j]
                    nb += 1
            moyenne = total / nb

            contains_word = int(moyenne > seuil)
            grid[a][b] = contains_word

            y = b2  # avancer horizontalement
        x = a2  # avancer verticalement

    return grid


def find_rectangles(matrix):
    """
    Takes as parameter a matrix.
    Returns the location of the two main rectangles.
    We advise to apply partition() before applying find_rectangles().
    """
    rows = len(matrix)
    cols = len(matrix[0])

    visited = [[0]*cols for _ in range(rows)] # matrice de booléens
    rectangles = []

    for i in range(rows):
        for j in range(cols):
            if matrix[i][j] == 1 and not visited[i][j]:
                stack = [(i, j)]
                min_r = max_r = i
                min_c = max_c = j
                while len(stack) > 0:
                    x, y = stack.pop()
                    if visited[x][y]:
                        continue
                    visited[x][y] = 1
                    min_r = min(min_r, x)
                    max_r = max(max_r, x)
                    min_c = min(min_c, y)
                    max_c = max(max_c, y)

                    (dx, dy) = (-1,0)
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < rows and 0 <= ny < cols and matrix[nx][ny] == 1 and not visited[nx][ny]:
                        stack.append((nx, ny))

                    (dx, dy) = (1,0)
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < rows and 0 <= ny < cols and matrix[nx][ny] == 1 and not visited[nx][ny]:
                        stack.append((nx, ny))

                    (dx, dy) = (0,-1)
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < rows and 0 <= ny < cols and matrix[nx][ny] == 1 and not visited[nx][ny]:
                        stack.append((nx, ny))

                    (dx, dy) = (0,1)
                    nx, ny = x + dx, y + dy
                    if 0 <= nx < rows and 0 <= ny < cols and matrix[nx][ny] == 1 and not visited[nx][ny]:
                        stack.append((nx, ny))

                rectangles.append(((min_r, min_c), (max_r, max_c)))

    return rectangles

def main(matrix, n, seuil):
    """
    Takes 3 parameters and returns the coordinates of the grid, then the coordinates of the words list
    Parameters :
        - matrix : A matrix of booleans (0 or 1). (0 is white, 1 is black). We are looking for "1" pixels.
        - n : number of subdivisions. The grid will be a n*n matrix.
        - seuil : tolerance for 1 pixels proportion per grid square to consider it "1".
    """

    width = len(matrix)
    height = len(matrix[0])
    
    # Average squares dimensions
    w = width // n
    h = height // n

    # Finds where the rectangles are located
    smallerMatrix = partition(matrix, n, seuil)
    rectangles = find_rectangles(smallerMatrix)

    if len(rectangles) < 2:
        raise ValueError("Not enough rectangles found. Check threshold or input matrix.")

    # Find witch one is the grid, witch one is the words list
    rectangleA = rectangles[0]
    rectangleB = rectangles[1]
    areaA = (rectangleA[1][0]-rectangleA[0][0]) * (rectangleA[1][1] - rectangleA[0][1])
    areaB = (rectangleB[1][0]-rectangleB[0][0]) * (rectangleB[1][1] - rectangleB[0][1])
    if areaA > areaB:
         grid = rectangleA
         words = rectangleB
    else:
         grid = rectangleB
         words = rectangleA
    
    # (grid, words) are coordinates on the grid

    # We convert to make (grid, words) coordinates on original matrix
    grid = ((grid[0][0] * w, grid[0][1] * h), (grid[1][0] * w, (grid[1][1]+1) * h))
    words = ((words[0][0] * w, words[0][1] * h), (words[1][0] * w, (words[1][1]+1) * h))
    return (grid, words)

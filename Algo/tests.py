"""
HOW TO TEST :
    - Create "M" a matrix of 0 and 1.
    - execute one_function_to_rule_them_all(M, n=10, seuil=0.01).
        n : number of subdivisions
        seuil : level where black pixel concentration is identify as a letter.
"""

# I - Import the File
from detect_zones import *

# II - Fonctions Annexes
def print_matrice(M):
        print(f"=========== MatrixPicture ===========")
        for line in M:
            for val in line:
                print(f"{val}", end="  ")
            print()
        print("===============================")

def convert_to_emoji(char):
    D = {0 : "⬛", 1 : "⬜"}
    if char in D:
         return D[char]
    else:
         return char

def print_matrice_emoji(M):
        print(f"============ MatrixPicture ============")
        for line in M:
            for val in line:
                print(f"{convert_to_emoji(val)}", end="  ")
            print()
        print("=======================================")

def file_to_matrix(nom_fichier):
        """
        Lit un fichier texte et retourne une matrice d'entiers.
        Chaque ligne du fichier correspond à une ligne de la matrice.
        Ex : '001' devient [0, 0, 1].
        """
        with open(nom_fichier, 'r') as f:
            lignes = f.read().strip().splitlines()

        matrice = [[int(c) for c in ligne.strip()] for ligne in lignes if ligne.strip()]
        return matrice


# III - Main Test Function

def one_function_to_rule_them_all(matrix, n, seuil):
    """
    One function to test them all 💍
    Cette fonction teste trois étapes :
        1. Test de Partition. Elle affiche la matrice obtenue
        2. Test de Find Rectangles. Elle affiche graphiquement et numériquement.
        3. Test de main, numériquement.
    """
    print("\n"*20)
    print(f"▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ ETAPE 1 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬")
    smallerMatrix = partition(matrix, n, seuil)
    print_matrice(smallerMatrix)

    print(f"\n▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ ETAPE 2 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬")
    rectangles = find_rectangles(smallerMatrix)

    rectangleA = rectangles[0]
    rectangleB = rectangles[1]

    smallerMatrix[rectangleA[0][0]][rectangleA[0][1]] = "✅"
    smallerMatrix[rectangleA[1][0]][rectangleA[1][1]] = "🟩"
    smallerMatrix[rectangleB[0][0]][rectangleB[0][1]] = "☑️ "
    smallerMatrix[rectangleB[1][0]][rectangleB[1][1]] = "🟪"

    print_matrice_emoji(smallerMatrix)


    
    areaA = (rectangleA[1][0]-rectangleA[0][0]) * (rectangleA[1][1] - rectangleA[0][1])
    areaB = (rectangleB[1][0]-rectangleB[0][0]) * (rectangleB[1][1] - rectangleB[0][1])
    if areaA > areaB:
         grille = rectangleA
         mots = rectangleB
    else:
         grille = rectangleB
         mots = rectangleA

    print(f"🟢 [grid] Les coordonnées de la grille sont {grille}")
    print(f"🟣 [grid] Les coordonnées de la liste de mots sont {mots}")

    print(f"\n▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ ETAPE 3 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬")
    (grille, mots) = main(matrix, n, seuil)
    print(f"🟢 [matrix] Les coordonnées de la grille sont {grille}")
    print(f"🟣 [matrix] Les coordonnées de la liste de mots sont {mots}")

    print("\n"*2)


if __name__ == "__main__":
    M = file_to_matrix("example.txt")
    one_function_to_rule_them_all(M, n=10, seuil=0.01)
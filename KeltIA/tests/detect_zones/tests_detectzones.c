#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ================================================================
// I - Declarations
// ================================================================
int** partition(int** MatrixPicture, int num_rows, int num_cols, int n, double seuil);
typedef struct {
    int r1, c1;
    int r2, c2;
} Rectangle;
Rectangle* find_rectangles(int** matrix, int rows, int cols, int* rect_count);
void main_function(int** matrix, int width, int height, int n, double seuil);

// ================================================================
// II - Fonctions Annexes
// ================================================================

void print_matrice(int** M, int rows, int cols) {
    printf("=========== MatrixPicture ===========\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d  ", M[i][j]);
        }
        printf("\n");
    }
    printf("=====================================\n");
}

char* convert_to_emoji(int val) {
    if (val == 0) return "⚫";  // black
    if (val == 1) return "⚪";  // white
    if (val == 2) return "🔵";
    if (val == 3) return "🟢";
    if (val == 4) return "🔴";
    if (val == 5) return "🟠";
    return "❓";                // other
}

void print_matrice_emoji(int** M, int rows, int cols) {
    printf("============ MatrixPicture ============\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%s  ", convert_to_emoji(M[i][j]));
        }
        printf("\n");
    }
    printf("=======================================\n");
}

/*
Lit un fichier texte et retourne une matrice d'entiers.
Chaque ligne du fichier correspond à une ligne de la matrice.
Ex : '001' devient [0, 0, 1].
*/
int** file_to_matrix(const char* nom_fichier, int* out_rows, int* out_cols) {
    FILE* f = fopen(nom_fichier, "r");
    if (!f) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", nom_fichier);
        exit(1);
    }

    char ligne[1024];
    int capacity = 1000;
    int** matrice = (int**)malloc(capacity * sizeof(int*));
    int rows = 0;
    int cols = -1;

    while (fgets(ligne, sizeof(ligne), f)) {
        int len = strlen(ligne);
        if (len == 0 || ligne[0] == '\n') continue;
        if (ligne[len - 1] == '\n') ligne[len - 1] = '\0';
        len = strlen(ligne);

        int* ligne_matrice = (int*)malloc(len * sizeof(int));
        for (int i = 0; i < len; i++) {
            ligne_matrice[i] = (ligne[i] == '1') ? 1 : 0;
        }
        if (cols == -1) cols = len;
        matrice[rows++] = ligne_matrice;
    }

    fclose(f);
    *out_rows = rows;
    *out_cols = cols;
    return matrice;
}

// ================================================================
// III - Main Test Function
// ================================================================

/*
One function to test them all 💍
Cette fonction teste trois étapes :
    1. Test de Partition. Elle affiche la matrice obtenue
    2. Test de Find Rectangles. Elle affiche graphiquement et numériquement.
    3. Test de main, numériquement.
*/
void one_function_to_rule_them_all(int** matrix, int width, int height, int n, double seuil) {
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ ETAPE 1 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬\n");
    int** smallerMatrix = partition(matrix, width, height, n, seuil);
    print_matrice(smallerMatrix, n, n);

    printf("\n▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ ETAPE 2 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬\n");
    int rect_count;
    Rectangle* rectangles = find_rectangles(smallerMatrix, n, n, &rect_count);

    if (rect_count < 2) {
        fprintf(stderr, "Pas assez de rectangles trouvés.\n");
        exit(1);
    }

    Rectangle rectangleA = rectangles[0];
    Rectangle rectangleB = rectangles[1];

    // On remplace graphiquement les coins
    smallerMatrix[rectangleA.r1][rectangleA.c1] = 2; // 🔵
    smallerMatrix[rectangleA.r2][rectangleA.c2] = 3; // 🟢
    smallerMatrix[rectangleB.r1][rectangleB.c1] = 4; // 🔴
    smallerMatrix[rectangleB.r2][rectangleB.c2] = 5; // 🟠

    print_matrice_emoji(smallerMatrix, n, n);

    int areaA = (rectangleA.r2 - rectangleA.r1) * (rectangleA.c2 - rectangleA.c1);
    int areaB = (rectangleB.r2 - rectangleB.r1) * (rectangleB.c2 - rectangleB.c1);
    Rectangle grille, mots;
    if (areaA > areaB) {
        grille = rectangleA;
        mots = rectangleB;
    } else {
        grille = rectangleB;
        mots = rectangleA;
    }

    printf("🟢 [grid] Les coordonnées de la grille sont ((%d,%d),(%d,%d))\n",
           grille.r1, grille.c1, grille.r2, grille.c2);
    printf("🟣 [grid] Les coordonnées de la liste de mots sont ((%d,%d),(%d,%d))\n",
           mots.r1, mots.c1, mots.r2, mots.c2);

    printf("\n▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ ETAPE 3 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬\n");
    main_function(matrix, width, height, n, seuil);

    printf("\n\n");
}

// ================================================================
// MAIN
// ================================================================

int main(int argc, char* argv[])
{
    if (argc != 4) {
        printf("Usage: %s <file> <n> <seuil>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    int n = atoi(argv[2]);
    double seuil = atof(argv[3]);

    int width, height;
    int** M = file_to_matrix(filename, &width, &height);
    if (M == NULL) {
        fprintf(stderr, "Erreur lors de la lecture du fichier %s\n", filename);
        return 1;
    }

    one_function_to_rule_them_all(M, width, height, n, seuil);

    // Free memory
    for (int i = 0; i < width; i++) free(M[i]);
    free(M);

    return 0;
}

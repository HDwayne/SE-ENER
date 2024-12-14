# Fractal Generator

## Description
Ce projet est un générateur de fractales écrit en C. Il utilise diverses techniques d'optimisation pour améliorer la vitesse de génération, notamment des optimisations liées à l'utilisation d'instructions SIMD (AVX) et de la parallélisation multicœur. Le programme permet de générer des fractales et d'enregistrer les résultats dans un fichier image BMP.

## Arborescence
```
.
├── algo.h               # Déclarations des fonctions liées à la génération des fractales
├── algo_opti1.c         # Version optimisée 1 (simplification des boucles et des opérations)
├── algo_opti2.c         # Version optimisée 2 (élimination ecriture des pixels noirs inutiles)
├── algo_opti3.c         # Version optimisée 3 (utilisation d'AVX monocœur)
├── algo_opti4.c         # Version optimisée 4 (utilisation d'AVX multicœur)
├── algo_opti5.c         # Version optimisée 5 (utilisation d'AVX-512 monocœur)
├── algo_opti6.c         # Version optimisée 6 (utilisation d'AVX-512 multicœur)
├── algo_simple.c        # Version de base (non optimisée)
├── bmp.c                # Gestion des fichiers BMP (écriture d'images)
├── bmp.h                # Déclarations pour les fonctions de gestion BMP
├── fractal.c            # Programme principal
├── Makefile             # Automatisation de la compilation
└── README.MD            # Ce fichier
```

## Compilation

Vous devez définir l'algorithme à utiliser en décommentant la ligne correspondante (voir fichier `fractal.c`) :

```c
// generateFractal(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX); // version simple
// generateFractal_opti1(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX); // version boucle et operation simplifiee
// generateFractal_opti2(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX); // version ne pas ecrire les pixels noirs
// generateFractal_opti3(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX); // AVX monocoeur
// generateFractal_opti4(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX); // AVX multicore
// generateFractal_opti5(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX); // AVX-512 monocoeur
generateFractal_opti6(pixels, width, height, iteration_max, A, B, XMIN, XMAX, YMIN, YMAX); // AVX-512 multicore
```

> **Note :** Les optimisations AVX et AVX-512 nécessitent un processeur compatible.

### Commandes :

```bash
make          # Compile le projet
make clean    # Supprime les fichiers objets et binaires
```

Le programme s'exécute en ligne de commande avec les options suivantes :

```bash
./fractal [-w width] [-h height] [-i iterations] [-f]
```
- `-w` : Largeur de l'image en pixels (défaut : 200)
- `-h` : Hauteur de l'image en pixels (défaut : 200)
- `-i` : Nombre d'itérations maximales (défaut : 100)
- `-f` : Sauvegarde l'image dans un fichier `fractal.bmp` (défaut : non)

### Exemple :

```bash
./fractal -w 10000 -h 10000 -f
```

Ce qui génère une fractale de 10000x10000 pixels et sauvegarde l'image dans un fichier `fractal.bmp`.
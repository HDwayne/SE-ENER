# Application de Surveillance d'Énergie

## Description

Ce projet est une application en C qui surveille la consommation énergétique d'une commande ou application donnée à l'aide de **MojitO/S**. L'application enregistre les mesures dans un fichier CSV et peut également exporter les résultats sous forme de fichier JSON.

## Fonctionnalités

- Surveille l'exécution d'une commande et enregistre :
  - Le temps total d'exécution.
  - L'énergie totale consommée.
  - La puissance moyenne.
- Génère un fichier CSV contenant les données brutes de MojitO/S.
- Génère un fichier JSON détaillant les résultats en notant la commande et les arguments utilisés. 
- Permet de configurer la fréquence de collecte des données.

## Arborescence

```
.
├── app.c               # Code source principal de l'application
├── Makefile            # Fichier de compilation
├── README.md           # Ce fichier
└── test.sh             # Script de test automatisé
```

## Compilation

Pour compiler l'application, utilisez la commande `make` :

```bash
make
```

Cela génère un exécutable nommé `app`. 

Pour nettoyer les fichiers de compilation et les données générées, utilisez :

```bash
make clean
```

## Utilisation

L'application s'exécute avec les paramètres suivants :

```bash
./app [-f frequency] [--json] <commande> [commande arguments...]
```

- `-f frequency` : Fréquence de collecte des données par MojitO/S (en Hz). Par défaut : `10`.
- `--json` : Ajoute les résultats au fichier JSON `data/results.json`. (si fichier JSON déjà existant, les résultats seront ajoutés à la fin).
- `<commande>` : Commande à surveiller suivie de ses arguments.

### Exemple

```bash
sudo ./app ../fractal/fractal -h 10000 -w 10000 -f
```

Les résultats seront enregistrés dans :
- `data/data.csv` pour les données brutes.
- `data/results.json` pour les résultats agrégés.

## Scripts de Test

Le projet inclut un script de test, `test.sh`, pour générer les données de consommation énergétique de l'application fractale avec différentes dimensions.

### Utilisation :

Pour enregistrer un test avec un nom spécifique :

```bash
make test NAME=<nom_du_test>
```

Les données générées seront déplacées dans un répertoire nommé `/result/data-<nom_du_test>-fractal`.

## Dépendances

- **MojitO/S** : Assurez-vous que MojitO/S est installé et accessible depuis le chemin `../mojitos/bin`.
- **Permissions root** : L'application nécessite des permissions root pour surveiller la consommation énergétique.
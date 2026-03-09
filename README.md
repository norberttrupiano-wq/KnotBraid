# KnotBraid

KnotBraid est une suite logicielle pour la conception et l’étude de tresses et de nœuds topologiques.
Le dépôt est organisé en **monorepo** afin de publier l’ensemble de la suite dans un seul projet.

## Contenu du dépôt

- `LogiKnotting/` : éditeur topologique de nœuds et de rubans.
- `LogiBraiding/` : outil de travail sur les tresses (ABoK et variantes).
## Captures d'ecran

### LogiKnotting
  <img src="docs/images/logiknotting-main.png" alt="LogiKnotting - ecran principal" width="75%">
</p>

### LogiBraiding
  <img src="docs/images/logibraiding-main.png" alt="LogiBraiding - ecran principal" width="75%">
</p>

## Prérequis

- Windows (PowerShell)
- CMake 3.16 ou supérieur
- Qt 6 (exemple local : `C:/Qt/6.10.1/msvc2022_64`)
- Compilateur C++17 (MSVC)

## Compilation rapide

Depuis la racine `E:/KnotBraid` :

### LogiKnotting

```powershell
cmake -S LogiKnotting -B LogiKnotting/build -DCMAKE_PREFIX_PATH="C:/Qt/6.10.1/msvc2022_64"
cmake --build LogiKnotting/build --config Release
```

### LogiBraiding

```powershell
cmake -S LogiBraiding -B LogiBraiding/build -DCMAKE_PREFIX_PATH="C:/Qt/6.10.1/msvc2022_64"
cmake --build LogiBraiding/build --config Release
```

## Nettoyage d’un build bloqué

Si un ancien dossier `build` est verrouillé après déplacement ou renommage :

```powershell
.\cleanup-build-ghost.ps1 -ProjectPath "E:\KnotBraid\LogiKnotting" -BuildDir build
```

Exemple pour `build2` :

```powershell
.\cleanup-build-ghost.ps1 -ProjectPath "E:\KnotBraid\LogiBraiding" -BuildDir build2
```

## Publication GitHub

Le dépôt racine est le **seul dépôt Git publié** :

```powershell
git push -u origin main
```

## Licence

La licence principale du monorepo est disponible dans `LICENSE`
(copie de `LogiKnotting/LICENCE`, GPL v3).

Les sous-projets peuvent conserver des mentions complémentaires dans leurs dossiers respectifs.

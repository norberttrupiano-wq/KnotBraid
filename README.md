# KnotBraid

KnotBraid est une suite logicielle pour la conception et l'etude de tresses et noeuds.
Le depot est organise en monorepo pour publier la suite en un seul morceau.

## Contenu du depot

- `LogiKnotting/` : editeur topologique de noeuds et rubans.
- `LogiBraiding/` : outil de travail sur les tresses (ABoK et variantes).
- `cleanup-build-ghost.ps1` : script utilitaire pour nettoyer des dossiers `build` bloques sous Windows.

Note: `KeyGenerator/` est ignore dans ce depot (`.gitignore`) et n'est pas publie sur GitHub.

## Prerequis

- Windows (PowerShell)
- CMake 3.16+
- Qt 6 (exemple local: `C:/Qt/6.10.1/msvc2022_64`)
- Compilateur C++17 (MSVC)

## Build rapide

Depuis la racine `E:/KnotBraid`:

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

## Nettoyage d'un build bloque

Si un ancien dossier `build` est verrouille apres deplacement/renommage:

```powershell
.\cleanup-build-ghost.ps1 -ProjectPath "E:\KnotBraid\LogiKnotting" -BuildDir build
```

Exemple pour `build2`:

```powershell
.\cleanup-build-ghost.ps1 -ProjectPath "E:\KnotBraid\LogiBraiding" -BuildDir build2
```

## Publication GitHub

Le depot racine est le seul depot Git a publier:

```powershell
git remote add origin https://github.com/<user>/<repo>.git
git push -u origin main
```

## Licence

La licence principale du monorepo est disponible dans `LICENSE` (copie de `LogiKnotting/LICENCE`, GPL v3).
Les sous-projets peuvent conserver des mentions complementaires dans leurs dossiers respectifs.


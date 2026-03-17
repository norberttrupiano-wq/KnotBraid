# KnotBraid - Manuel utilisateur

Version de travail enrichie - 17 mars 2026

## Table des matières

1. [Présentation générale](#1-présentation-générale)
2. [KnotBraid : lancer la suite](#2-knotbraid--lancer-la-suite)
3. [LogiBraiding : exécuter une tresse](#3-logibraiding--exécuter-une-tresse)
4. [LogiKnotting : concevoir et corriger un nœud](#4-logiknotting--concevoir-et-corriger-un-nœud)
5. [Référence des commandes de LogiKnotting](#5-référence-des-commandes-de-logiknotting)

## 1. Présentation générale

KnotBraid est une suite consacrée à la préparation, à la correction et à l'exécution de nœuds et de tressages.
Elle réunit trois outils complémentaires :

- `KnotBraid` : le lanceur général et l'espace de shell.
- `LogiBraiding` : le guidage pas à pas pour le tressage.
- `LogiKnotting` : l'atelier de conception, de correction de croisements et d'impression.

L'idée générale est simple :

1. préparer ou corriger un modèle dans `LogiKnotting` ;
2. passer dans `LogiBraiding` pour suivre l'exécution pratique ;
3. garder `KnotBraid` comme point d'entrée rapide entre les modules.

> **Conseil**  
> Pour constituer une bibliothèque ABoK propre, travailler d'abord en `.lkw`, corriger les croisements, vérifier l'impression, puis valider en `.lkv` seulement quand le modèle est définitif.

### 1.1 À qui s'adresse la suite

KnotBraid s'adresse :

- au tresseur qui veut exécuter une séquence sans quitter sa table ;
- au concepteur qui veut construire ou corriger un nœud ;
- à l'utilisateur qui souhaite préparer une bibliothèque de modèles prêts à imprimer.

### 1.2 Les types de fichiers

- `.lkw` : fichier de travail, modifiable.
- `.lkv` : fichier validé, verrouillé en lecture seule.
- `JSON` : catalogue des tresses de `LogiBraiding`.

> **Attention**  
> Un fichier `.lkv` n'est plus éditable. Pour reprendre un travail à partir d'un modèle validé, utiliser d'abord la commande `Copie`.

### 1.3 Répertoires de travail

Par défaut, `LogiKnotting` enregistre dans `Release/KnotBraid` si ce dossier existe et reste accessible en écriture.
Sinon, l'application bascule vers `Documents/KnotBraid`.

Au démarrage :

- `LogiKnotting` recharge le dernier nœud ouvert avec succès ;
- `LogiBraiding` recharge la dernière tresse utilisée.

> **Capture à prévoir**  
> Page d'accueil ou vue générale de la suite, avec mention visible des trois modules.

## 2. KnotBraid : lancer la suite

`KnotBraid` est le lanceur central.
Il a été allégé pour laisser un maximum de place au shell et conserver une navigation rapide entre les modules.

### 2.1 La palette flottante

La palette flottante contient les actions essentielles :

- `Accueil`
- `Knotting`
- `Braiding`
- `F11`

Elle reste superposée à l'interface, peut être déplacée à la souris, et sa position est mémorisée séparément pour chaque application.

> **Conseil**  
> Place la palette dans une zone qui n'empiète ni sur le shell ni sur les zones de travail visuel. La position choisie sera restaurée au prochain lancement.

### 2.2 Usage conseillé

Utiliser `KnotBraid` comme tableau de bord :

- `Knotting` pour construire ou corriger ;
- `Braiding` pour exécuter ;
- `Accueil` pour revenir à la vue de départ ;
- `F11` pour agrandir ou rétablir l'affichage selon le contexte.

> **Capture à prévoir**  
> Lanceur KnotBraid avec la palette flottante visible au-dessus du shell.

## 3. LogiBraiding : exécuter une tresse

`LogiBraiding` est conçu pour rester lisible pendant le tressage réel.
L'écran doit donner l'information utile au bon moment, sans surcharger la vue.

### 3.1 Principe de fonctionnement

L'utilisateur choisit une tresse, lit l'instruction affichée, exécute le geste sur sa table, puis passe au mouvement suivant.
Le module a été simplifié pour ne garder que les commandes réellement utiles pendant l'exécution.

### 3.2 Organisation de l'interface

La barre du haut regroupe :

- le sélecteur de tresse ;
- le bouton `Séquentiel` ;
- le choix de l'intervalle entre deux pas ;
- le compte à rebours ;
- l'état `AuthorID / Essai` ;
- `Dessin` ;
- `Doc` ;
- `Menu` ;
- le bouton d'affichage des détails.

À gauche, un cadre d'instruction compact affiche le geste courant.
À droite du cadre, deux boutons restent toujours visibles :

- `✓` : mouvement suivant ;
- `✕` : recommencer la séquence.

Au centre, la table de tressage montre la position des fils et le mouvement à venir.

> **Capture à prévoir**  
> Vue principale de LogiBraiding avec le cadre d'instruction, les boutons `✓` et `✕`, la table centrale et la barre supérieure.

### 3.3 Lire l'instruction

Le cadre d'instruction suit volontairement une forme courte :

```text
étape 1/80
prendre le fil intérieur de la case 80
l'amener à l'extérieur de la case 2 ↩
puis recentrer les fils restants de la case 80
```

Le texte doit rester suffisant à lui seul, tandis que la table de tressage confirme visuellement le geste.

> **Conseil**  
> Si l'utilisateur connaît déjà la technique, le texte court et le visuel de la table suffisent. Il n'est pas nécessaire de surcharger l'écran avec des explications longues.

### 3.4 Démarrer une tresse

1. Choisir la tresse dans la liste.
2. Vérifier l'étape affichée dans le cadre.
3. Réaliser le geste sur la table réelle.
4. Cliquer sur `✓` pour afficher le pas suivant.

Le bouton `✕` remet la séquence au début.

### 3.5 Le mode séquentiel

Le mode `Séquentiel` ne ralentit pas le tracé.
Il reproduit automatiquement l'équivalent d'un clic sur `✓` après un délai choisi.

Le cycle de travail est donc :

1. l'étape courante s'affiche ;
2. l'utilisateur exécute le geste sur sa table ;
3. le compte à rebours arrive à zéro ;
4. le pas suivant s'affiche automatiquement.

Le mode séquentiel s'arrête tout seul à la fin du tour.
Après le dernier pas, l'application revient immédiatement à l'étape 1 de la séquence suivante.

Les intervalles disponibles sont :

- `5 s`
- `10 s`
- `15 s`
- `20 s`
- `30 s`
- `40 s`
- `50 s`
- `60 s`

> **Attention**  
> Le mode séquentiel n'est pas un mode de démonstration graphique. Il est pensé pour laisser au tresseur le temps d'exécuter un geste réel avant l'affichage du pas suivant.

> **Capture à prévoir**  
> Barre supérieure de LogiBraiding avec le bouton `Séquentiel`, la liste des secondes et le compte à rebours actif.

### 3.6 Outils complémentaires

`Dessin` permet de charger une image de référence.
`Doc` ouvre le document de référence `Solid Sinnet variations.pdf`.
`Menu` donne accès :

- aux langues ;
- à l'éditeur des tresses ;
- au rechargement du catalogue JSON ;
- à l'aide rapide ;
- à la boîte `À propos`.

Le bouton de détails affiche ou masque la table de mouvements afin de libérer davantage de place pour la table de tressage.

### 3.7 Bonnes pratiques

Pour l'apprentissage d'une nouvelle tresse, travailler en mode manuel avec `✓`.
Pour une tresse déjà connue, choisir un intervalle confortable et laisser le mode séquentiel faire défiler les pas.

> **Conseil**  
> Un intervalle de `20 s` à `30 s` convient souvent mieux qu'un rythme trop court quand le geste réel demande du temps sur la table.

## 4. LogiKnotting : concevoir et corriger un nœud

`LogiKnotting` est l'atelier de conception de la suite.
On y prépare la géométrie du nœud, on ajuste le ruban, on corrige les croisements et on prépare l'impression.

### 4.1 Les trois modes de travail

Le module combine trois approches :

- `Tracage` : dessin réel corde par corde ;
- `Esquisse` : construction ou transformation provisoire ;
- `Insert` : correction explicite des croisements.

### 4.2 Les notions essentielles

#### Ruban

Le nœud est construit sur un ruban de longueur réglable.
Le ruban peut être tourné pour ramener une zone utile dans la vue.

#### Cordes

Jusqu'à cinq cordes peuvent être utilisées.
Une corde active reçoit les nouveaux points de tracé.

#### Croisements

Le mode `Insert` sert à corriger le sens over/under.
Les croisements modifiés pendant la session sont marqués visuellement.
Les inversions sont maintenant sauvegardées et restaurées au rechargement.

#### Validation

Un fichier validé `.lkv` devient non éditable.
Pour repartir d'une version modifiable, il faut d'abord utiliser `Copie`.

> **Attention**  
> La validation n'est pas une simple sauvegarde. Elle correspond à un état final, destiné à la consultation, à l'archivage ou à l'impression stable.

### 4.3 Cycle de travail conseillé

1. Créer ou ouvrir un fichier `.lkw`.
2. Choisir la corde active.
3. Tracer la structure principale.
4. Passer en `Esquisse` si une correction géométrique est nécessaire.
5. Revenir en `Tracage`.
6. Passer en `Insert` pour corriger les croisements.
7. Sauvegarder.
8. Valider le nœud quand la géométrie est définitivement fixée.

> **Capture à prévoir**  
> Vue principale de LogiKnotting avec la règle bleue, le ruban, la corde active et un nœud en cours d'édition.

### 4.4 Mode Tracage

Le mode `Tracage` correspond au dessin réel du nœud.
Chaque clic pose un nouveau point sur la corde active.
Le modèle n'autorise que certaines directions, et un aperçu coloré indique si le segment pressenti reste cohérent.

Les pas de snap sont :

- `1 mm` sans modificateur ;
- `5 mm` avec `Shift` ;
- `10 mm` avec `Ctrl`.

> **Conseil**  
> Pour les premières passes de construction, rester en pas fin. Les pas plus larges deviennent utiles pour les déplacements rapides ou les structures plus régulières.

### 4.5 Mode Esquisse

Le mode `Esquisse` sert à construire ou transformer une géométrie provisoire avant de revenir au tracé réel.

Fonctionnement :

1. le premier clic pose une ancre ;
2. les clics suivants ajoutent des segments à partir de cette ancre ;
3. `Espace` coupe l'esquisse en cours ;
4. `Shift + glisser` crée une sélection ;
5. `Shift + glisser` sur la sélection la déplace ;
6. `Ctrl + Shift + glisser` la duplique puis la déplace.

Les commandes `Rotation de 45° à droite` et `Inversion de sens` s'appliquent à la sélection active.

> **Attention**  
> Le mode `Esquisse` ne remplace pas le tracé final. Il sert à préparer, corriger ou déplacer des segments avant intégration dans la logique du nœud.

> **Capture à prévoir**  
> Exemple de sélection rectangulaire en Esquisse, puis déplacement ou duplication de la sélection.

### 4.6 Mode Insert

Le mode `Insert` est dédié aux croisements.
Il s'active avec la touche `Insert`.

Une fois ce mode actif :

- le survol affiche la nature du croisement ;
- un `clic-droit` inverse le croisement survolé ;
- `Esc` quitte le mode.

L'infobulle reste volontairement courte :

- `Croisement Gauche`
- `Croisement Droite`
- `Clic-droit pour inverser`

> **Conseil**  
> Corriger un croisement dès qu'un doute apparaît évite les incohérences plus loin dans la préparation du nœud ou au moment de l'impression.

> **Capture à prévoir**  
> Mode `Insert` actif avec un croisement survolé et son remplissage rouge translucide lorsqu'il a été modifié pendant la session.

### 4.7 Gestion des fichiers

`Ctrl+O` ouvre un fichier de travail ou un fichier validé.
`Ctrl+S` enregistre le travail courant en `.lkw`.
La commande `Validation` produit un `.lkv` verrouillé.
La commande `Copie` crée une version éditable à partir d'un `.lkv`.

La boîte `Propriétés du fichier` affiche les informations de création, de modification et le temps de travail accumulé.

### 4.8 Impression

Le système d'impression a été recalé pour réduire le nombre de pages.
L'impression se fait sur la largeur utile du ruban, avec orientation paysage et réduction automatique si la hauteur dépasse la zone imprimable.

Le profil d'impression permet de choisir :

- le papier `A4` ou `A3` ;
- le recouvrement ;
- l'impression avec ou sans grille.

> **Attention**  
> Toujours contrôler l'aperçu avant impression avant une sortie définitive, en particulier sur les rubans longs ou les montages destinés à être assemblés sur plusieurs pages.

> **Capture à prévoir**  
> Aperçu avant impression avec un ruban réparti sur peu de pages paysage et repères de coupe/recouvrement.

### 4.9 Bonnes pratiques

Sauvegarder régulièrement en `.lkw` tant que le nœud évolue encore.
Utiliser `Insert` avant la validation finale pour vérifier que les croisements reflètent bien le résultat attendu.
Conserver la validation `.lkv` pour les modèles stables, prêts à archiver, imprimer ou diffuser.

## 5. Référence des commandes de LogiKnotting

### 5.1 Fichier et impression

| Commande | Touche / geste | Contexte | Effet |
|---|---|---|---|
| Nouveau | Menu `Fichier > Nouveau` | général | crée un nouveau document |
| Ouvrir | `Ctrl+O` | général | ouvre un fichier `.lkw` ou `.lkv` |
| Enregistrer | `Ctrl+S` | fichier de travail | enregistre le document courant |
| Imprimer | `Ctrl+P` | général | lance l'impression |
| Mise en page | Menu | général | règle le format papier |
| Aperçu avant impression | Menu | général | affiche l'aperçu |
| Validation | Menu | fichier éditable | verrouille en `.lkv` |
| Copie | Menu | fichier validé | crée une copie éditable |
| Propriétés du fichier | Menu | général | affiche les métadonnées |

### 5.2 Vue et navigation

| Commande | Touche / geste | Contexte | Effet |
|---|---|---|---|
| Zoom avant | `Ctrl++` | général | agrandit la vue |
| Zoom arrière | `Ctrl+-` | général | réduit la vue |
| Zoom 100 % | `Ctrl+0` | général | remet le zoom par défaut |
| Retour origine | `Home` | général | remet l'offset du ruban à `0` |

### 5.3 Modes

| Commande | Touche / geste | Contexte | Effet |
|---|---|---|---|
| Mode Esquisse | `Ctrl+E` | général | passe en esquisse |
| Mode Tracage | `Ctrl+B` | général | passe en traçage |
| Mode Insert | `Insert` | général | entre ou sort du mode croisements |
| Quitter Insert | `Esc` | mode Insert | ferme le mode croisements |

### 5.4 Ruban

| Commande | Touche / geste | Contexte | Effet |
|---|---|---|---|
| Tourner à gauche | `←` | général | rotation du ruban de `1 mm` |
| Tourner à droite | `→` | général | rotation du ruban de `1 mm` |
| Tourner à gauche pas 5 | `Shift+←` | général | rotation du ruban de `5 mm` |
| Tourner à droite pas 5 | `Shift+→` | général | rotation du ruban de `5 mm` |
| Tourner à gauche pas 10 | `Ctrl+←` | général | rotation du ruban de `10 mm` |
| Tourner à droite pas 10 | `Ctrl+→` | général | rotation du ruban de `10 mm` |
| Raccourcir le ruban | `Alt+←` | avant franchissement | réduit la longueur de `1 mm` |
| Allonger le ruban | `Alt+→` | avant franchissement | augmente la longueur de `1 mm` |
| Raccourcir pas 5 | `Alt+Ctrl+←` | avant franchissement | réduit la longueur de `5 mm` |
| Allonger pas 5 | `Alt+Ctrl+→` | avant franchissement | augmente la longueur de `5 mm` |
| Raccourcir pas 10 | `Alt+Ctrl+Shift+←` | avant franchissement | réduit la longueur de `10 mm` |
| Allonger pas 10 | `Alt+Ctrl+Shift+→` | avant franchissement | augmente la longueur de `10 mm` |

### 5.5 Cordes

| Commande | Touche / geste | Contexte | Effet |
|---|---|---|---|
| Corde 1 | `Ctrl+1` | général | active la corde 1 |
| Corde 2 | `Ctrl+2` | général | active la corde 2 |
| Corde 3 | `Ctrl+3` | général | active la corde 3 |
| Corde 4 | `Ctrl+4` | général | active la corde 4 |
| Corde 5 | `Ctrl+5` | général | active la corde 5 |

### 5.6 Esquisse

| Commande | Touche / geste | Contexte | Effet |
|---|---|---|---|
| Sectionner l'esquisse | `Espace` | mode Esquisse | coupe l'esquisse en cours |
| Supprimer la sélection | `Delete` ou `Backspace` | mode Esquisse | efface la sélection |
| Copier la sélection | `Ctrl+C` | mode Esquisse | copie les segments sélectionnés |
| Coller la sélection | `Ctrl+V` | mode Esquisse | colle la sélection |
| Annuler | `Ctrl+Z` | mode Esquisse | annule sur l'esquisse |
| Rétablir | `Ctrl+Y` | mode Esquisse | rétablit sur l'esquisse |
| Rotation de 45° à droite | Menu `Esquisse` | sélection en esquisse | tourne la sélection |
| Inversion de sens | Menu `Esquisse` | sélection en esquisse | inverse la sélection |

### 5.7 Croisements

| Commande | Touche / geste | Contexte | Effet |
|---|---|---|---|
| Activer édition croisements | `Insert` | général | affiche le mode Insert |
| Inverser un croisement | `Clic-droit` | mode Insert | inverse le croisement survolé |
| Quitter le mode | `Esc` | mode Insert | ferme le mode Insert |

### 5.8 Souris et pas de snap

| Commande | Touche / geste | Contexte | Effet |
|---|---|---|---|
| Poser un point | `Clic gauche` | mode Tracage | ajoute le point suivant |
| Poser ancre / segment | `Clic gauche` | mode Esquisse | crée l'ancre puis les segments |
| Snap fin | `Clic gauche` | tracé / esquisse | pas de `1 mm` |
| Snap moyen | `Shift + clic gauche` | hors sélection mobile | pas de `5 mm` |
| Snap large | `Ctrl + clic gauche` | hors sélection mobile | pas de `10 mm` |
| Sélection rectangulaire | `Shift + glisser gauche` | mode Esquisse | crée une sélection |
| Déplacer sélection | `Shift + glisser gauche` sur sélection | mode Esquisse | déplace la sélection |
| Dupliquer puis déplacer | `Ctrl + Shift + glisser gauche` sur sélection | mode Esquisse | copie puis déplace |

### 5.9 Remarques de contexte

- `Ctrl+Z` et `Ctrl+Y` changent de rôle selon le mode actif.
- Dans un fichier validé `.lkv`, les commandes d'édition sont neutralisées.
- En `Insert`, l'action principale est le `clic-droit` sur le croisement survolé.
- Relâcher `Shift` en esquisse termine le déplacement en cours et efface la sélection visuelle.

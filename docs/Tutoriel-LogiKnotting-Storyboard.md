# Tutoriel LogiKnotting - Storyboard Video

Ce document sert de reference unique pour garder le tutoriel texte et les videos parfaitement coherents.

Principe :
- le texte affiche l'objectif de l'etape ;
- la video montre exactement cette action ;
- l'utilisateur sait ensuite quoi reproduire lui-meme.

## Etape 0 - Bienvenue

Titre UI :
`Bienvenue dans LogiKnotting`

Objectif :
Presenter le flux complet de travail avant d'entrer dans les manipulations.

Texte ecran :
- presentation generale de LogiKnotting
- rappel des 5 phases : ruban, tracage, esquisse, croisements, impression

Video conseillee :
- `Etape 0.mp4` ou pas de video

Contenu video attendu :
- vue generale de l'interface
- mise en evidence visuelle du ruban
- mise en evidence du menu `Modes`
- mise en evidence de la barre d'etat

Action utilisateur apres la video :
- cliquer sur `Repartir d'un document vierge`

Point de vigilance :
- ne pas montrer de geste complexe ici
- cette etape doit seulement rassurer et donner une vue d'ensemble

## Etape 1 - Le document de depart

Titre UI :
`Etape 1 - Le document de depart`

Objectif :
Faire comprendre la structure de l'espace de travail.

Texte ecran :
- regle bleue
- barre d'etat
- menu `Modes`
- logique append-only du trace

Video associee :
- `Etape 1.mp4`

Contenu video attendu :
- document vierge
- balayage lent de la regle bleue
- balayage de la barre d'etat
- ouverture visuelle du menu `Modes`
- survol de `Mode Tracage`, `Mode Esquisses`, `Mode Croisements`

Action utilisateur apres la video :
- cliquer sur `Activer le mode Tracage`

Point de vigilance :
- la video ne doit pas encore poser de segment
- elle doit surtout montrer ou regarder

## Etape 2 - Tracer la premiere corde

Titre UI :
`Etape 2 - Tracer la premiere corde`

Objectif :
Montrer comment poser un point et lire le vecteur de direction.

Texte ecran :
- clic gauche pour poser un point
- fleches pour faire tourner le ruban
- `Shift` et `Ctrl` pour changer le pas
- `Ctrl+1` a `Ctrl+5` pour changer de corde

Video associee :
- `Etape 2.mp4`

Contenu video attendu :
- passage clair en `Mode Tracage`
- creation d'un premier point bleu
- deplacement du curseur avec affichage du vecteur
- moment visible ou le vecteur passe du rouge au vert
- clic final pour creer le segment
- eventuellement changement de corde vers `Corde 2`

Action utilisateur apres la video :
- poser un point puis un premier segment

Point de vigilance :
- c'est l'etape la plus importante pour un debutant
- le passage rouge -> vert doit etre tres lisible
- ne pas aller trop vite

## Etape 3 - Construire une esquisse

Titre UI :
`Etape 3 - Construire une esquisse`

Objectif :
Montrer la logique ancre -> segment guide -> nouvelle ancre.

Texte ecran :
- passer en `Mode Esquisses`
- premier clic = ancre
- clics suivants = segments guides
- `Espace` = interrompre l'esquisse
- les esquisses sont sauvegardees dans les `.lkw`

Video associee :
- `Etape 3.mp4`

Contenu video attendu :
- ouverture du menu `Modes`
- choix de `Mode Esquisses`
- pose d'une ancre
- ajout de deux ou trois segments guides
- interruption avec `Espace`
- reprise ailleurs

Action utilisateur apres la video :
- activer `Mode Esquisses` et poser une petite esquisse

Point de vigilance :
- bien montrer que l'esquisse est differente du trace reel
- ne pas melanger avec la correction de croisements

## Etape 4 - Transformer l'esquisse

Titre UI :
`Etape 4 - Transformer l'esquisse`

Objectif :
Montrer la selection et les transformations du menu `Modes`.

Texte ecran :
- `Shift+glisser` pour selectionner
- `Ctrl+Shift+glisser` pour dupliquer
- `Suppr` pour supprimer la selection
- rotations et retournements

Video associee :
- `Etape 4.mp4`

Contenu video attendu :
- creation d'une petite selection
- deplacement de la selection
- duplication
- rotation 90 degres
- retournement vertical ou horizontal

Action utilisateur apres la video :
- selectionner puis transformer une petite esquisse

Point de vigilance :
- chaque geste doit etre net
- ne pas montrer trop d'operations dans une seule video si cela devient confus

## Etape 5 - Corriger les croisements

Titre UI :
`Etape 5 - Corriger les croisements`

Objectif :
Montrer le travail sur les croisements, y compris la copie d'ordre.

Texte ecran :
- `Ctrl+C` pour `Mode Croisements`
- clic droit pour inverser
- clic gauche segment source puis segment cible pour copier l'ordre

Video associee :
- `Etape 5.mp4`

Contenu video attendu :
- passage en `Mode Croisements`
- survol d'un croisement avec popup
- clic droit pour inversion
- selection d'un segment source
- selection d'un segment cible comparable
- resultat visuel sur les croisements

Action utilisateur apres la video :
- inverser un croisement puis recopier un ordre de croisements

Point de vigilance :
- prendre un exemple tres lisible
- la copie d'ordre doit etre visuellement evidente

## Etape 6 - Enregistrer et valider

Titre UI :
`Etape 6 - Enregistrer et valider`

Objectif :
Expliquer la difference entre `.lkw` et `.lkv`.

Texte ecran :
- `.lkw` = fichier de travail editable
- `.lkv` = fichier valide verrouille
- utiliser `Copie` pour retravailler un fichier valide

Video associee :
- `Etape 6.mp4`

Contenu video attendu :
- enregistrement d'un `.lkw`
- lancement de `Validation`
- reouverture du fichier valide
- demonstration de `Copie`

Action utilisateur apres la video :
- enregistrer un fichier de travail puis comprendre quand valider

Point de vigilance :
- ne pas laisser croire qu'un `.lkv` est editable

## Etape 7 - Imprimer le ruban

Titre UI :
`Etape 7 - Imprimer le ruban`

Objectif :
Montrer comment verifier l'impression avant de lancer une sortie papier.

Texte ecran :
- apercu avant impression
- format papier
- recouvrement
- centrage
- grille visible ou non

Video associee :
- `Etape 7.mp4`

Contenu video attendu :
- ouverture de l'apercu avant impression
- changement de profil A4/A3
- essai de centrage
- exemple avec ou sans grille

Action utilisateur apres la video :
- ouvrir l'apercu avant impression

Point de vigilance :
- insister sur le fait qu'il faut toujours controler l'apercu avant impression

## Etape 8 - Reprendre plus tard

Titre UI :
`Etape 8 - Reprendre plus tard`

Objectif :
Montrer les reperes pour continuer seul apres le tutoriel.

Texte ecran :
- `F1` pour le manuel
- rechargement du dernier noeud
- palette flottante du shell
- option `Ne pas rejouer a l'ouverture`

Video associee :
- `Etape 8.mp4` ou pas de video

Contenu video attendu :
- ouverture du manuel
- fermeture du tutoriel
- coche finale `Ne pas rejouer a l'ouverture`

Action utilisateur apres la video :
- terminer le tutoriel

Point de vigilance :
- cette etape doit conclure calmement et laisser l'utilisateur autonome

## Regles De Coherence

- Le texte ne doit jamais annoncer une action absente de la video.
- La video ne doit jamais montrer un menu ou un geste non mentionne dans le texte.
- Une video = une idee principale.
- Si une action est delicate, ralentir le geste et laisser une courte pause avant le clic.
- Les noms affiches doivent etre strictement les memes que dans l'interface :
  `Mode Tracage`
  `Mode Esquisses`
  `Mode Croisements`
  `Ganses`
- Si le texte parle du vecteur rouge / vert, la video doit montrer clairement ce changement.
- Si une video devient trop chargee, il vaut mieux la scinder en deux et simplifier le texte.

## Ordre De Production Conseille

1. Etape 1
2. Etape 2
3. Etape 3
4. Etape 5
5. Etape 4
6. Etape 6
7. Etape 7
8. Etape 8

Ordre retenu pour produire d'abord les demonstrations les plus structurantes :
- lecture de l'interface
- trace
- esquisse
- croisements


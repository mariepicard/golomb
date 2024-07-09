# Golomb
## Description

Fichier C `ogr.c`. Le but est de calculer le plus rapidement possible une [règle de Golomb][rdg] optimale, pour un certain ordre (pour lequel le temps de calcul reste raisonnable, i.e ordre de grandeur de la minute au plus), et ce sans précalcul. 

[rdg]: https://fr.wikipedia.org/wiki/R%C3%A8gle_de_Golomb

## Résultats

Le programme peut effectuer les calculs pour des ordres jusqu'à 12 sur mon ordianteur, et s'effectue en 23s pour l'ordre 11 et un peu moins de 2 minutes pour l'ordre 12 sur mon ordinateur personnel. Une version qui utilise du multithreading (`ogr_thread.c`) est également présente. Son efficacité dépend majoritairement de l'ordinateur utilisé, et du nombre de coeurs que le programme peut exploiter.

## Utilisation
- pas de fichiers supplémentaires nécessaires
- pas de lecture de l'ordre de la règle implémentée (si besoin, modifier dans le main)

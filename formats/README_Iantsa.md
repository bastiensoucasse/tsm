# Exemple de signal de clavier telephonique

## Analyse de fréquences

### Question 1

durée d'un signal de clavier: d = 0.2 s \
fréquence d'échantillonnage: Fech = 44100 Hz

FRAME_SIZE = d * Fech = 0.2 * 44100 = 8820 \
résolution fréquentielle: **freq_prec** = Fech / FRAME_SIZE / 2 = **2.5 Hz**

A priori, les fréquences sont assez espacées (plus de 2.5 Hz) donc il n'y a pas de précautions à prendre.

### Question 2
Pour voir tout le signal, il suffit de mettre le FRAME_SIZE au nombre d'échantillons total du signal, ici **158760**. \
En visualisant tout le signal, on observe **x pics**. On en déduit que certaines **fréquences apparaissent plusieurs fois**.

### Question 3
Au minimum, on voit x pics. \
Au maximum, on voit x pics. \
On en déduit qu'une **fenêtre de 0.1 s** est pertinente.

### Question 4

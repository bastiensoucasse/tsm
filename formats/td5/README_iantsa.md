# Watermarking

## Exemple sur un signal musical

Dans un 1er temps, il s'agit de détecter les pics dont la fréquence est supérieure à 18000 Hz. On se rend compte avec cette étape qu'il n'y en a pas que 3 comme on l'espérait. 

L'idée est d'établir un seuil d'amplitude, au-dessous duquel la fréquence ne sera pas considérée. Pour cela, on affiche d'abord les amplitudes des fréquences trouvées à la 1ère étape. 

On remarque que 3 amplitudes sortent du lot: environ **51**, **53** et **49**. On peut donc par exemple mettre le seuil à **40**.

Ces amplitudes correspondent respectivement aux fréquences **19126**, **19584** et **20032 Hz**. Comme on connaît l'ordre d'apparition pour le signal sonore `flux1.wav`, on peut en déduire que ce sont respectivement les fréquences des évènements A, B et C.

durée d'un signal de clavier: d = 0.05 s \
fréquence d'échantillonnage: Fech = 44100 Hz \
**FRAME_SIZE** = d * Fech = 0.05 * 44100 = **2205** \
**HOP_SIZE** = **2205**

précision fréquentielle: **res_freq** = Fech / (2*FRAME_SIZE) = **10 Hz**
Cela paraît être une précision raisonnable, étant donné qu'on va travailler sur de grandes fréquences.

TODO: Expliquer l'équilibre trouvé entre la précision d'analyse de la fréquence et la durée choisie pour l'ana- lyse. (lol)


## Détection d'évènements

Les résultats obtenus sont les suivants:

`flux1.wav`
- 8.00 s: A
- 16.00 s: B
- 24.00 s: C

(cohérent avec l'énoncé)

`flux2.wav`
- 1.25 s: C
- 3.35 s: B
- 4.45 s: C
- 7.70 s: B
- 11.70 s: A
- 12.80 s: A
- 14.00 s: B

`flux3.wav`
- 2.00 s: B
- 4.00 s: C
- 6.00 s: A
- 8.00 s: B
- 10.00 s: C
- 12.00 s: A

`flux4.wav`
- 0.10 s: B
- 0.35 s: A
- 0.50 s: A
- 0.65 s: A
- 0.80 s: C
- 1.05 s: C
- 1.20 s: B
- 1.35 s: C
- 1.50 s: B
- 1.75 s: A
- 1.90 s: A
- 2.05 s: A
- 2.20 s: C
- 2.45 s: C
- 2.60 s: B
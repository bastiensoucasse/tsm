Bastien Soucasse

# Watermarking – TD 5

Fréquence d'échantillonnage :\
$F_e = 44100 \; \text{Hz}$

Durée :\
$t = 0.05 \; \text{s}$

Résolution temporelle :\
$n = F_e t = 2205 \; \text{échantillons}$

Résolution fréquentielle :\
$\Delta f = \frac{F_e}{n} = 20 \; \text{Hz}$

Précision fréquntielle :\
$f_{\text{prec}} = \frac{\Delta f}{2} = 10 \; \text{Hz}$

Précision temporelle :\
$t_{\text{prec}} = \frac{t}{2} = 0.025\; \text{s}$

Après analyse, il apparaît que les fréquences contenant des évènements ont une amplitude supérieure à 3. Nous pouvons donc les reconnaître ainsi.

Ainsi, on obtient le résultat suivant.

Dans `sounds/flux1.wav` :
- 8.00 s : Évènement A.
- 16.00 s : Évènement B.
- 24.00 s : Évènement C.

Dans `sounds/flux2.wav` :
- 1.25 s : Évènement C.
- 3.35 s : Évènement B.
- 4.45 s : Évènement C.
- 7.70 s : Évènement B.
- 11.70 s : Évènement A.
- 12.80 s : Évènement A.
- 14.00 s : Évènement B.

Dans `sounds/flux3.wav` :
- 2.00 s : Évènement B.
- 4.00 s : Évènement C.
- 6.00 s : Évènement A.
- 8.00 s : Évènement B.
- 10.00 s : Évènement C.
- 12.00 s : Évènement A.

Dans `sounds/flux4.wav` :
- 0.10 s : Évènement B.
- 0.35 s : Évènement A.
- 0.50 s : Évènement A.
- 0.65 s : Évènement A.
- 0.80 s : Évènement C.
- 1.05 s : Évènement C.
- 1.20 s : Évènement B.
- 1.35 s : Évènement C.
- 1.50 s : Évènement B.
- 1.75 s : Évènement A.
- 1.90 s : Évènement A.
- 2.05 s : Évènement A.
- 2.20 s : Évènement C.
- 2.25 s : Évènement C.
- 2.45 s : Évènement C.
- 2.50 s : Évènement C.
- 2.60 s : Évènement B.
- 2.65 s : Évènement B.

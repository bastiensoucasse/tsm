# Watermarking – TD 5

Bastien Soucasse – TSM

Fréquence d'échantillonnage :\
**F_e = 44100 Hz**

Durée :\
**t = 0.05 s**

Résolution temporelle :\
**n = F_e t = 2205 échantillons**

Résolution fréquentielle :\
**Df = F_e / n = 20 Hz**

Précision fréquntielle :\
**f_prec = Df / 2 = 10 Hz**

Précision temporelle :\
**t_prec = t / 2 = 0.025 s**

Après analyse, il apparaît que les fréquences contenant des évènements ont une amplitude supérieure ou égale à 50 (arrondie). Nous pouvons donc les reconnaître ainsi.

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
- 2.45 s : Évènement C.
- 2.60 s : Évènement B.

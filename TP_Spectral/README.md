Si on met la `FRAME_SIZE` à 1024, il fait environ une demi seconde de calcul. Par contre si on la met à 44100 (une seconde d'intervalle), le temps de calcul devient proche de 60 secondes. On en conclut que la DFT est très lente.

Pour `FRAME_SIZE = 44100` :
- DFT unique : 53.98 secondes.
- FFT unique : 0.00 secondes.

Pour `FRAME_SIZE = 1024` :
- DFT moyenne : 0.03 secondes (11.97 secondes au total).
- FFT entière : 0.000 secondes (0.003 secondes au total).

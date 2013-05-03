manque  Source code
 (les patches)


Les fichiers buildroot-*.tar.bz2, busybox-*.tar.bz2, linux-*.tar.bz2 et u-boot-*.tar.bz2 sont les tarballs nécessaires mais qui pourraient être retéléchargés depuis Internet.
Le fichiers configs.tar.gz contient des .config pour buildroot, busybox, linux, de sorte a ne pas refaire tout dans les make menuconfig, et qui plus est, fonctionnent (pour la carte Arm émulée)
Le fichier u-boot-*.patch est un patch pour booter QEmu avec la carte Arm émulée.
Le fichier images.tar.gz contient des images precompilées.
Les deux fichiers .txt contiennent les commandes pour QEmu.


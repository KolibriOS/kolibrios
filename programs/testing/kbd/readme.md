# KolibriOS Bus Disconnect

```text
Автор  : Михайлов Илья Андреевич aka Ghost
Версия : от 24/10/06
ОС     : KolibriOS
Форум  : meos.sysbin.com -> Программы -> GMon
Почта  : ghost.nsk@mail.ru
```

## Поддерживаемые чипсеты:

- Advanced Micro Devices:
  - AMD-751
  - AMD-761
  - AMD-762

- NVIDIA:
  - NVIDIA nForce
  - NVIDIA nForce 2
  - NVIDIA nForce 2 400

- Silicon Integrated Systems:
  - SiS 730
  - SiS 733
  - SiS 735
  - SiS 740
  - SiS 741/741GX/M741
  - SiS 745
  - SiS 746/746DX/746FX
  - SiS 748
  - SiS 755 (Athlon 64)

- VIA Technologies:
  - VIA KT133/KT133A/KM133/KL133/KN133
  - VIA KX133
  - VIA KLE133
  - VIA KT266/KT266A/KT333
  - VIA KM266/KL266/KM333
  - VIA KN266
  - VIA KT400/KT400A/KT600
  - VIA KM400/KM400A
  - VIA KT880

## ПРИМЕЧАНИЕ (Heavyiron, 24.10.2006):

Владельцы данных чипсетов могут (при желании) поместить
эту программу в автозагрузку с параметром boot. Для этого в
autorun.dat необходимо поместить, например, такую строку:
"/SYS/KBD              BOOT       20    # Enable Bus Disconnect for AMD K7 processors",
не забыв при этом увеличить количество запускаемых программ на одну
в самом начале файла autorun.dat.

## Изменения (ревизия от 28.06.2026)

- Исправлен KLE133: правильный device ID 0x3112 (VT8361), а не 0x0691.
- Добавлены KX133 (0x0691) и SiS 755.
- Удалён несуществующий AMD 751S (0x7004).

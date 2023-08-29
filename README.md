# projekt-opaski

Kod był pisany w ramach realizacji praktyk w RED-Elektronics

Szybkie info co do implementacji

~~Z plików podaję tylko maina oraz config do PlatformIO oraz config do TFT_eSPI bo w sumie tylko to jest przydatne.~~

[edit: Wrzucę też cały folder bo raczej powinno się dać otworzyć to jako projekt w PlatformIO]

## Libraries:
- TFT_eSPI ~bodmer
- EspSoftwareSerial ~plerup

(do znalezienia w wyszukiwarce PlatformIO)

## Wgrywanie kodu do stacji
Przy korzystaniu z flash_download_tool trzeba pamiętać o tym, że jak dostajemy 3 pliki .bin z kompliacji kodu w VScode (bootloader, partitions, firmware)
To trzeba wgrać w kolejności i pod adresami:

**0x1000** *botloader*
**0x8000** *partitions*
**0x10000** *firmware*

(pdf z takimi informacjami też wrzucę)

## Misc
A i tworząc projekt na PlatformIO korzystałem z takich ustawień płytki i frameworka:

![image](https://github.com/FuturePhile/projekt-opaski/assets/135601063/2ea47c5a-1e41-442e-b77c-0d806f8bda28)

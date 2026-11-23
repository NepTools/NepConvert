# NepConvert

An easy to use save file converter for Hyperdimension Neptunia U: Action Unleashed!

## Note(s)

Converting a PSVita save to a PC save reqiures the decrypted save data!!

Your pc save is in the "savedata" folder next to the executable, and the PSVita save data is in ux0:/user/00/savedata (Press triangle and then press "Open decrypted" in VitaShell to get your decrypted save data)!

After converting a PC save file to a PSVita save file I recommend using something like Apollo Save Tool to import them (put it in ux0:/data/apollo/00/PCSE00588)

## How to use

### PSVita Save to PC Save

```
nepconvert path/to/nep_story01.dat nepd_00.dat
```

### PC Save to PSVita Save

```
nepconvert path/to/decrypted/nepd_00.dat nep_story01.dat
```

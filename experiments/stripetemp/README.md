# stripetemp
This experiment shows how to use a webhook (with server-side parsing) to bind an online
weather service to a LED stripe using a particle photon.

## Webhook
Use one of the two provided samples in the webhooks folder:

1. edit the file, replacing the placeholders in brackets
2. issue:
```
particle webhooks create <foo>.json
```

## FastLED library
In order to set up the library for particle dev:

* Clone https://github.com/focalintent/FastLED-Sparkcore
* Move the firmware folder to this folder, as _FastLED_ and remove _docs_ and _examples_ folders
```
mv /path/to/FastLED-Sparkcore/firmware ./FastLED
rm -r ./FastLED/{docs,examples}
```

## Compilation
Open the folder in particle dev and compile in the cloud.

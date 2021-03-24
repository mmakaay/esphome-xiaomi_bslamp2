# esphome-yeelight_bs2

## Installation

Create a folder named `custom_components` in the folder where your
device's yaml configuration file is stored. Then clone the the Github
repo into that folder. For example on the command line:

```
# mkdir custom_components
# cd custom_compnents
# git clone https://github.com/mmakaay/esphome-yeelight_bs2
```

Your folder structure should now look like:
```
config
├── yourdevice.yaml
├── custom_components/
│   ├── esphome-yeelight_bs2/
│   .   ├── README.md
.   .   ├── yeelight_bs2_light_output.h
.       .
        .
```

Then add the required configuration to your device's yaml configuration file.
For an example file, taks a look at `doc/example.yaml` in this repository.


# Interactive System for Electronic Creativity

This folder is for the drawing system.

Currently runs on windows and linux (ubuntu, debian, pi os).

You must have CMAKE and mingw-32 installed on your windows machine if using windows.

If using unix system, make sure you have the following packages:
``` bash
sudo apt install build-essential
sudo apt install cmake xorg-dev libglu1-mesa-dev
sudo apt-get install libwayland-dev
sudo apt-get install libxkbcommon-dev
```

Graphics API is GLFW. Set it up by running:
```bash
python setup_environment.py
```

Build executable for drawing software:
```bash
python build.py build
```

Clean:
```bash
python build.py clean
```
